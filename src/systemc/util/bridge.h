#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
using namespace sc_core;
using namespace tlm;
using namespace tlm_utils;
using namespace std;

class bridge : public sc_module {
    using pl_t = tlm_generic_payload;
    SC_HAS_PROCESS(bridge);
public:
    simple_target_socket<bridge, 256> SC_NAMED(b_transport_socket);
    simple_initiator_socket<bridge, 256> SC_NAMED(nb_transport_socket);

    bridge(sc_module_name nm) : sc_module(nm) {
        b_transport_socket.register_b_transport(this, &bridge::b_transport);
        nb_transport_socket.register_nb_transport_bw(this, &bridge::nb_transport_bw);
    }
private:
    sc_event SC_NAMED(done_event);

    tlm_sync_enum nb_transport_bw(pl_t& trans, tlm_phase& phase, sc_time& delay) {

        if (phase == END_REQ) {
            return TLM_ACCEPTED;
        } else if (phase == BEGIN_RESP) {
            done_event.notify(SC_ZERO_TIME);
            return TLM_ACCEPTED;
        }
        return TLM_ACCEPTED;
    }

    void b_transport(pl_t& trans, sc_time& delay) {
        tlm_phase phase = BEGIN_REQ;
        sc_time local_delay = delay;
        // send BEGIN_REQ
        tlm_sync_enum status = nb_transport_socket->nb_transport_fw(trans, phase, local_delay);
        
        if (status == TLM_ACCEPTED || status == TLM_UPDATED) {
            wait(done_event);
            trans.set_response_status(TLM_OK_RESPONSE);
        } else if (status == TLM_COMPLETED) {
            // Handle completed transaction immediately
            return;
        }
        delay += local_delay; // Accumulate the delay
    }
};