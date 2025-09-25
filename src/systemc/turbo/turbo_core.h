#ifndef TURBO_CORE_H
#define TURBO_CORE_H

#include <systemc>
using namespace sc_core;
using namespace std;

#include <tlm.h>
using namespace tlm;
#include <tlm_utils/simple_initiator_socket.h>

#include "riscv/processor.h"
#include "riscv/simif.h"      // needed for base class
#include "riscv/cfg.h"        // needed for cfg_t* in header
#include "riscv/processor.h"  // needed for processor_t* in header
#include "riscv/isa_parser.h" // needed for isa_parser_t member
#include "riscv/devices.h"    // for bus_t
#include "riscv/log_file.h"   // for log_file_t
#include "riscv/debug_module.h"
#include "util/dbg_component.h" // needed for debug_component class
class remote_bitbang_t;


#ifdef MEASURE_PERF
#include <chrono>
#endif

class turbo_tlm_extension : public tlm_extension<turbo_tlm_extension> {
public:
    turbo_tlm_extension() {}
    ~turbo_tlm_extension() {}
    void copy_from(tlm_extension_base const& ext) {}
    tlm_extension_base* clone() const override { return new turbo_tlm_extension(); }
    std::type_index get_type() const { return typeid(turbo_tlm_extension); }
    
    uint64_t coreId{0};
};
class turbo_core : public simif_t, public sc_module, public debug_component {
    SC_HAS_PROCESS(turbo_core);

public:
    turbo_core(sc_module_name nm, const cfg_t* cfg, const debug_module_config_t& dm_config);
    ~turbo_core() = default;

    // TLM sockets
    tlm_utils::simple_initiator_socket<turbo_core, 256 /*width*/, tlm::tlm_base_protocol_types>
        SC_NAMED(init_socket); // TODO: fix socket width

    // functionalities for running and stopping simulation
    sc_event SC_NAMED(start_ev);
    void run_m(); // running method. let it stay method for now
    
    void trigger_start() { 
        start_ev.notify(SC_ZERO_TIME); 
    }

    inline void simulate_n_cycles(size_t cycles);

    void end_simulation() {
        cout << "Now is the end of simulation @ " << sc_time_stamp() << endl;
        sc_core::sc_stop();
    }

    // bool send_tlm(reg_t paddr, size_t len, const uint8_t* bytes, bool is_write);

    // pure virtual functions from simif_t
    char* addr_to_mem(reg_t paddr) override;
    bool reservable(reg_t paddr) override;
    bool mmio_load(reg_t paddr, size_t len, uint8_t* bytes) override;
    bool mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) override;
    const std::map<size_t, processor_t*>& get_harts() const override;
    void proc_reset(unsigned id) override;
    const cfg_t& get_cfg() const override;
    const char* get_symbol(uint64_t paddr) override;
    // end pure virtual functions

    processor_t* get_core(size_t i) { return procs.at(i); }

    #ifdef MEASURE_PERF
    static const uint64_t PERF_REPORT_INTERVAL = 10000;
    std::chrono::high_resolution_clock::time_point sim_start_time;
    std::chrono::high_resolution_clock::time_point last_report_time;
    uint64_t total_instructions_executed;
    uint64_t instructions_at_last_report;
    void report_performance();
    #endif


    void reset() {
        for(auto& proc : procs) {
            proc->reset();
        }
    }

    void configure_log(bool enable_log = false);
    bool log{false};
    log_file_t log_file{"out.txt"}; // Default log file
    inline bool is_spike_device_addr(reg_t paddr);

private:
    // components
    std::vector<processor_t*> procs; // TODO check
    std::unique_ptr<bus_t> bus;

    std::map<size_t, processor_t*> harts;

    // counting instructions
    static constexpr uint64_t STEP_CYCLES = 5000;
    // isa-related
    isa_parser_t isa;
    const cfg_t* const cfg;
    std::map<std::string, uint64_t> symbols;
    // TLM functionalities
    tlm::tlm_generic_payload* mem_trans{nullptr};
    tlm::tlm_sync_enum nb_transport(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay);
    sc_event SC_NAMED(done_event);

    // for GDB
    remote_bitbang_t* remote_bitbang{nullptr};
    unsigned current_proc{0};

    public:
    debug_module_t debug_module;
    void set_remote_bitbang(remote_bitbang_t* remote_bitbang) {
        this->remote_bitbang = remote_bitbang;
    }
}; // class turbo_core


#endif
