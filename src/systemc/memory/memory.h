// Simple Memory, Interconect, Routing
#ifndef _SIMPLE_MEM_TLM_H_
#define _SIMPLE_MEM_TLM_H_

#include "util/sparse_array.h"
#include "util/dbg_component.h"
#include "util/mem-loader.h"
#include <tlm_utils/simple_target_socket.h>
#include <memory>
#include <iomanip>

#define SOC_SCR_PRINT 0x3fffb040
#define SOC_SCR_FINISH 0x3fffb008


/**
 * \brief part of base_protocol_simple_memory
 *
 * Module is connected to parent module via simple_target_socket. AT coding style is used.
 * It has 2 methods for responses, r_resp and w_resp.
 *
 * Since we can have 1 read and multiple write requests, reads and writes are differently handled.
 * For answering read requests, in this example we use a tlm_utils::peq_with_get.
 * For answering write requests, we keep write requests in std::queue (since they need to be answered in order) and we
 * use sc_event_queue for notifying.
 * (Note: If you send request to axi_target before the previous was finished (i.e. END_REQ was sent), assertion will
 * happen)
 *
 * \tparam N Width of the socket
 */

struct CircularBuffer {
    tlm::tlm_generic_payload** buffer;
    size_t head;     // next write position
    size_t tail;     // next read position
    static constexpr size_t CIRCULAR_BUFFER_SIZE = 32;
    static constexpr size_t BUFFER_MASK = CIRCULAR_BUFFER_SIZE - 1;

    CircularBuffer() : head(0), tail(0) {
        buffer = new tlm::tlm_generic_payload*[CIRCULAR_BUFFER_SIZE];
    }

    ~CircularBuffer() {
        delete[] buffer;
    }

    inline void push_back(tlm::tlm_generic_payload* value) {
        size_t next_head = (head + 1) & BUFFER_MASK;
        assert(next_head != tail && "Buffer is full");
        buffer[head] = value;
        head = next_head;
    }

    inline tlm::tlm_generic_payload* pop_front() {
        if (tail == head) return nullptr;
        tlm::tlm_generic_payload* value = buffer[tail];
        tail = (tail + 1) & BUFFER_MASK;
        return value;
    }

    inline bool empty() const {
        return tail == head;
    }
};

template <unsigned int N, typename TYPES=tlm::tlm_base_protocol_types>
class mem_module : public sc_module, public debug_component {
public:

    sc_event SC_NAMED(stop_event);
    
    SC_HAS_PROCESS(mem_module);
    // ports and sockets
    sc_in<bool> SC_NAMED(clk_i);
    tlm_utils::simple_target_socket<mem_module, N, TYPES> SC_NAMED(socket);

    bool printVerb = false;

    mem_module(
        sc_module_name nm,
        util::sparse_array<uint8_t, 18, 30>* m_ptr,
        unsigned r_done,
        unsigned r_ack,
        unsigned w_done,
        unsigned w_ack
    )
    : sc_module(nm)
    , mem_ptr(m_ptr)
    , read_done_delay(r_done)
    , read_ack_delay(r_ack)
    , write_done_delay(w_done)
    , write_ack_delay(w_ack)
    , debug_component("mem_module")
    {
        socket.register_nb_transport_fw(this, &mem_module::transport_fw);

        SC_METHOD(do_read);
        sensitive << pend_r_q_ev;
        dont_initialize();

        SC_METHOD(ack_read);
        sensitive << ack_r_q_ev;
        dont_initialize();

        SC_METHOD(do_write);
        sensitive << pend_w_q_ev;
        dont_initialize();

        SC_METHOD(ack_write);
        sensitive << ack_w_q_ev;
        dont_initialize();

    SC_METHOD(stop_sim);
        sensitive << stop_event;
        dont_initialize();
    
        printVerb = false; // (program_options::getInstance().getVerbosity() >= 1) ? true : false;
    }

    // helper to calculate next rising clock edge
    inline sc_time getNextRisingClockEdge(unsigned clk_cycles) {
        return clk_cycles * clock_period + (sc_core::sc_time_stamp() % clock_period);
    }

    void end_of_elaboration() override {
        auto *clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface());
        assert(clk_if != nullptr);
        clock_period = clk_if->period();
    }

    tlm::tlm_sync_enum transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t) {
        tlm::tlm_command cmd = trans.get_command();

        if (isDebugEnabled()) {
          std::stringstream ss;
          ss << "{\"t\":"<< sc_core::sc_time_stamp().to_double() << "," << "\"addr\":\"0x"<<hex<<trans.get_address()<<"\", \"msg\":\"";
          if (cmd == tlm::TLM_READ_COMMAND) {
            ss << "TLM_MEM_READ";
          } else {
            ss << "TLM_MEM_WRITE";
          }
          ss << " phase " << phase << "\"}";
          log_debug(ss.str());
        }
        switch(cmd) {
        case(tlm::TLM_READ_COMMAND): {
            switch(phase) {
            case(tlm::BEGIN_REQ): {
                pend_r_q_ev.notify(getNextRisingClockEdge(read_done_delay));
                pend_r_q.push_back(&trans);

                phase = tlm::END_REQ;
                sc_time t{SC_ZERO_TIME};
                auto end_req_reply = socket->nb_transport_bw(trans, phase, t);
                return tlm::TLM_ACCEPTED;
            } break;
            case(tlm::END_RESP): {
                return tlm::TLM_COMPLETED;
            } break;
            default:
                cerr << "Wrong TLM command: " << cmd << endl;
            }
        } break;
        case(tlm::TLM_WRITE_COMMAND): {
            switch(phase) {
            case tlm::BEGIN_REQ: {
                pend_w_q_ev.notify(getNextRisingClockEdge(write_done_delay));
                pend_w_q.push_back(&trans);

                phase = tlm::END_REQ;
                sc_time t{SC_ZERO_TIME};
                auto end_req_reply = socket->nb_transport_bw(trans, phase, t);
                return tlm::TLM_ACCEPTED;
            } break;
            case tlm::END_RESP:
                return tlm::TLM_COMPLETED;
                break;
            default:
                cerr << "Wrong TLM phase" << endl;
            }
        } break;
        default:
            cerr << "Wrong TLM command" << endl;
        }
        return tlm::TLM_COMPLETED;
    }

    void do_read(){
      if (!pend_r_q.empty()) {
        auto* trans = pend_r_q.pop_front();
        
        assert(trans != nullptr);
        const auto addr = trans->get_address();
        uint8_t* const data_ptr = trans->get_data_ptr();
        const unsigned data_len = trans->get_data_length();
        // do read
        mem_ptr->read(addr, data_ptr, data_len);
        // schedule notification
        ack_r_q.push_back(trans);
        ack_r_q_ev.notify(getNextRisingClockEdge(read_ack_delay));

        if(isDebugEnabled()) {
          std::stringstream ss;
          ss << "{\"t\":"<< sc_core::sc_time_stamp().to_double() << "," << "\"addr\":\"0x"<<hex<<addr<<"\","<<"\"data\":\"0x";
          ss << std::hex;
          for(int i = 0; i < data_len; i++ ) {
            ss << std::setw(2) << std::setfill('0') << (int)data_ptr[i];
          }
          ss << "\",\"msg\":\"TLM_MEM_READ done\"}";
          log_debug(ss.str());
        }
      }
      else {
        cerr << "pend_r_q is empty @" << sc_core::sc_time_stamp() << endl;
      }
    }
    void stop_sim() {
      sc_core::sc_stop();
    }

    static bool testStatusWritten;

    void do_write(){
      if (!pend_w_q.empty()) {
        auto* trans = pend_w_q.pop_front();
        assert(trans != nullptr);

        const auto addr = trans->get_address();
        uint8_t* const data_ptr = trans->get_data_ptr();
        const unsigned data_len = trans->get_data_length();
        uint8_t* const be_ptr = trans->get_byte_enable_ptr();
        const unsigned be_len = trans->get_byte_enable_length();



        if (addr == SOC_SCR_FINISH) {
          const unsigned int testStatus = ((unsigned int*)data_ptr)[0];
          if (testStatus != 0 && !testStatusWritten) {
            if (testStatus == 0x5555) {
                cout << "PASS" << endl;
            } else {
                cout << "FAIL" << endl;
            }
            testStatusWritten = true;
            // don't finish immediately, allow 100 cycles to be dumped to VCD file
            stop_event.notify(SC_ZERO_TIME);
          }
        } else
          if (printVerb && addr == SOC_SCR_PRINT) {
            printf("%c", (int)data_ptr[0]);
        }

        // do write
        mem_ptr->write(addr, data_ptr, data_len, be_ptr, be_len);

        if (isDebugEnabled()){
          // TODO: replace to_double()
          std::stringstream ss;
          ss << "{\"t\":"<< sc_core::sc_time_stamp().to_double() << "," << "\"addr\":\"0x"<<hex<<addr<<"\","<<"\"data\":\"0x";
          ss << std::hex;
          for(int i = 0; i < data_len; i++ ) {
            ss << std::setw(2) << std::setfill('0') << (int)data_ptr[i];
          }
          ss << "\",\"msg\":\"TLM_MEM_WRITE done\"}";
          log_debug(ss.str());
        }

        // schedule notification
        ack_w_q.push_back(trans);
        ack_w_q_ev.notify(getNextRisingClockEdge(write_ack_delay));
      }
      else {
        cerr << "pend_w_q is empty @ " << sc_core::sc_time_stamp() << endl;
      }
    }

    void ack_read() {
      if (!ack_r_q.empty()) {
        auto* trans = ack_r_q.pop_front();
        assert(trans != nullptr);

        sc_time t{SC_ZERO_TIME}; // time value is not important
        tlm::tlm_phase phase = tlm::BEGIN_RESP;
        trans->set_response_status(tlm::TLM_OK_RESPONSE);

        auto begin_resp_reply = socket->nb_transport_bw(*trans, phase, t);
      }
      else {
        cerr << "ack_r_q is empty @ " << sc_core::sc_time_stamp() << endl;
      }
    }

    void ack_write() {
      if (!ack_w_q.empty()) {
        auto trans = ack_w_q.pop_front();
        assert(trans != nullptr); 
        sc_time t{SC_ZERO_TIME}; // time value is not important
        tlm::tlm_phase phase = tlm::BEGIN_RESP;
        trans->set_response_status(tlm::TLM_OK_RESPONSE);

         auto begin_resp_reply = socket->nb_transport_bw(*trans, phase, t);
      } else {
        cerr << "ack_w_q is empty @ " << sc_core::sc_time_stamp() << endl;
      }
    }

    inline void set_read_done_delay(unsigned d) noexcept {read_done_delay = d;}
    inline void set_read_ack_delay(unsigned d) noexcept {read_ack_delay = d;}
    inline void set_write_done_delay(unsigned d) noexcept {write_done_delay = d;}
    inline void set_write_ack_delay(unsigned d) noexcept {write_ack_delay = d;}

    private:

    // members
    unsigned read_done_delay;
    unsigned read_ack_delay;
    unsigned write_done_delay;
    unsigned write_ack_delay;

    CircularBuffer pend_r_q;
    CircularBuffer ack_r_q;
    CircularBuffer pend_w_q;
    CircularBuffer ack_w_q;

    sc_event_queue SC_NAMED(pend_r_q_ev);
    sc_event_queue SC_NAMED(ack_r_q_ev);
    sc_event_queue SC_NAMED(pend_w_q_ev);
    sc_event_queue SC_NAMED(ack_w_q_ev);

    util::sparse_array<uint8_t, 18, 30>* mem_ptr; // pointer to memory array
    sc_time clock_period{0, SC_NS};               // to be filled in end_of_elaboration

}; // end class mem_module

template <unsigned int N, typename TYPES>
bool mem_module<N,TYPES>::testStatusWritten = false;


class mir_tlm_bare : public sc_module {
public:
  SC_HAS_PROCESS(mir_tlm_bare);
  sc_in<bool> SC_NAMED(clock);


  tlm::tlm_target_socket<256, tlm::tlm_base_protocol_types> SC_NAMED(target_socket);
  void enableDebug() {
    mem_module_0.enableDebug();
  }


private:

  mem_module<256> SC_NAMED(mem_module_0, &mem0, 1, 6, 0, 5);

  util::sparse_array<uint8_t, 18, 30> mem0;

public:
  mir_tlm_bare(sc_module_name nm)
  : sc_module(nm) {

    target_socket.bind(mem_module_0.socket);
    mem_module_0.clk_i(clock);

}
MemoryLoader mem_loader{&mem0};

};

#endif // _SIMPLE_MEM_TLM_H_
