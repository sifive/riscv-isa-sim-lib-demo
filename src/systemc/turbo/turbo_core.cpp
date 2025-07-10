
#include "turbo_core.h"
#include "riscv/mmu.h"          
#include "riscv/debug_module.h" 
#include "riscv/devices.h"      
#include "riscv/log_file.h"     
#include "riscv/remote_bitbang.h"
// #include <tlm/scc/tlm_mm.h>


turbo_core::turbo_core(sc_module_name nm, const cfg_t* cfg, const debug_module_config_t& dm_config)
: sc_module(nm), 
  debug_component("turbo_core"), 
  cfg(cfg), 
  isa(cfg ? cfg->isa : "", cfg ? cfg->priv : ""),
  debug_module(this, dm_config) {
    
    assert(cfg != nullptr && "cfg cannot be nullptr");
    // SystemC processes
    SC_THREAD(run_m);
    sensitive << start_ev;
    dont_initialize();

    mem_trans = new tlm::tlm_generic_payload;// TODO: fix tlm::scc::tlm_mm<>::get().allocate<axi::ace_extension>(1024);
    mem_trans->set_data_length(4096);
    unsigned char* data_ptr = new unsigned char[4096];
    mem_trans->set_data_ptr(data_ptr);

    // CPUs
    procs.reserve(cfg->nprocs());

    for(size_t i = 0; i < cfg->nprocs(); i++) {
        // clang-format off
        procs.push_back(new processor_t(cfg->isa /*isa_str*/
                                        , cfg->priv /*priv_str*/
                                        , cfg /*cfg*/
                                        , this /*sim*/
                                        , i /*id*/
                                        , false /*halt_on_reset*/
                                        , log_file.get()  // Pass log file handle to processor
                                        , cout /*sout_*/
                                        ));
        harts[cfg->hartids[i]] = procs[i];
        // clang-format on
    }

    for(const auto& proc : procs) {
        proc->set_debug(false);
    }

    bus = std::make_unique<bus_t>();
    bus->add_device(0x0, &debug_module);

    configure_log(true, true);
    init_socket.register_nb_transport_bw(this, &turbo_core::nb_transport);

}

char* turbo_core::addr_to_mem(reg_t paddr) { 
    return nullptr;
}

bool turbo_core::reservable(reg_t paddr) {
    // Implementation depends on your memory model
    // This is a placeholder implementation
    return false;
}

// if we want to include more devices which will be on the bus
// this function will be more complex
bool turbo_core::is_spike_device_addr(reg_t paddr) {
    return (paddr < PGSIZE);
}

bool turbo_core::mmio_load(reg_t paddr, size_t len, uint8_t* bytes) {
    LOG_DBG("mmio_load called for address 0x" << hex << paddr);
    if (is_spike_device_addr(paddr)) {
        return bus->load(paddr, len, bytes);
    }

    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    sc_time delay{SC_ZERO_TIME};
    tlm::tlm_generic_payload trans;
    trans.set_command(tlm::TLM_READ_COMMAND);
    trans.set_address(paddr);
    // TODO: check if memcpy is better
    trans.set_data_ptr(bytes);
    trans.set_data_length(len);
    turbo_tlm_extension* ext = new turbo_tlm_extension();
    ext->coreId = current_proc;
    trans.set_extension(ext);

    auto status = nb_transport(trans, phase, delay);
    wait(done_event);
    return status;
}

bool turbo_core::mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) {
    LOG_DBG("mmio_store called for address 0x" << hex << paddr);
    if (is_spike_device_addr(paddr)) { // TODO: DO propper address mapping
        return bus->store(paddr, len, bytes);
    }
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    sc_time delay{SC_ZERO_TIME};
    tlm::tlm_generic_payload trans;
    trans.set_command(tlm::TLM_WRITE_COMMAND);
    trans.set_address(paddr);
    trans.set_data_ptr(const_cast<unsigned char*>(bytes));
    trans.set_data_length(len);
    turbo_tlm_extension* ext = new turbo_tlm_extension();
    ext->coreId = current_proc;
    trans.set_extension(ext);
    auto status = nb_transport(trans, phase, delay);
    wait(done_event);
    return status;
}

tlm::tlm_sync_enum turbo_core::nb_transport(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& delay) {
    LOG_DBG("nb_transport called with phase " << phase);
    LOG_DBG("addr = 0x" << hex << trans.get_address());

    if (phase == tlm::BEGIN_REQ) {
        // this is from MMIO funcs
        sc_time t{SC_ZERO_TIME};
        auto end_req_reply = init_socket->nb_transport_fw(trans, phase, t);
        
    } else if (phase == tlm::END_REQ){
        return tlm::TLM_ACCEPTED;
    }
    else if (phase == tlm::BEGIN_RESP) {
        done_event.notify(SC_ZERO_TIME);        
    }
    return tlm::TLM_COMPLETED;
}


const std::map<size_t, processor_t*>& turbo_core::get_harts() const {
    return harts;
}

void turbo_core::proc_reset(unsigned id) {
    // This is a placeholder implementation
}

const cfg_t& turbo_core::get_cfg() const {
    // Return the cfg member
    return *cfg;
}

const char* turbo_core::get_symbol(uint64_t paddr) {
    // This is a placeholder implementation
    return nullptr;
}

void turbo_core::run_m() {
    while(1)
        simulate(5000);
}

void turbo_core::configure_log(bool enable_log, bool enable_commitlog) {
    log = enable_log;
    if (enable_commitlog) {
        for (auto& proc : procs) {
            cout << "Enabling commit log " << endl;
            proc->enable_log_commits();
        }
    }
}

inline void turbo_core::simulate(size_t cycles)  {
        current_proc = 0;

        for(auto& proc : procs) {
            current_proc++;
            proc->step(cycles);
        }
        if (remote_bitbang)
            this->remote_bitbang->tick();
    }
