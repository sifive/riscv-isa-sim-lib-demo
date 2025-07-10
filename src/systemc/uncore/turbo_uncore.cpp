#include "turbo_uncore.h"

turbo_uncore::turbo_uncore(sc_module_name nm, const cfg_t* cfg) : 
    sc_module(nm) 
    , debug_component("turbo_uncore") {
    // sockets
    init_socket.register_nb_transport_bw(this, &turbo_uncore::nb_transport_bw);
    targ_socket.register_nb_transport_fw(this, &turbo_uncore::nb_transport_fw);
    // adding default devices
    cout << "Creating soc scr" << endl;
    sc_soc_scr* soc_scr = new sc_soc_scr("soc_scr", 0x3fffb000 /*base*/, 0x1000 /*size*/);
    bus.register_device(soc_scr);
    make_mems(cfg->mem_layout);
};

// TODO:
tlm::tlm_sync_enum turbo_uncore::nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t) {
    LOG_DBG("nb_transport_fw with phase: " << phase);

    tlm::tlm_sync_enum status = TLM_ACCEPTED;
    auto* device = bus.find_device(trans.get_address());
    // if device is nullptr, just forward the transaction to the memory
    if (device == nullptr) {
        status = init_socket->nb_transport_fw(trans, phase, t);
    } else {
        // This is for now modelled as immediate R/W
        if (trans.is_read()) {
            device->read(trans.get_address(), trans.get_data_ptr(), trans.get_data_length());
        } else {
            LOG_DBG("write to device at " << hex << trans.get_address());
            device->write(trans.get_address(), trans.get_data_ptr(), trans.get_data_length());
        }
        return TLM_COMPLETED;
    }
    return status;
}

tlm::tlm_sync_enum turbo_uncore::nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t) {
    LOG_DBG("nb_transport_bw with phase: " << phase);

    if (phase == END_REQ) {
        return TLM_ACCEPTED;
    } else if (phase == BEGIN_RESP) {
        targ_socket->nb_transport_bw(trans, phase, t);
        return TLM_ACCEPTED;
    }
    return TLM_ACCEPTED;
}

void turbo_uncore::make_mems(const std::vector<mem_cfg_t>& layout) {
    int i = 0;

    // This is currently rendered in TLM wrapper
    for(const auto& cfg : layout) {
        std::string name = "mem_" + std::to_string(i++);
        uint64_t base = cfg.get_base();
        uint64_t size = cfg.get_size();
        sc_mem_device* mem = new sc_mem_device(name.c_str(), base, size);
        LOG_DBG("adding memory to bus. Name: " << name << " at " << hex << base << " with size 0x" << hex << size);
        bus.register_device(base, size, mem);
    }
    bus.print_devices();
}

void turbo_uncore::set_debug() {
    this->enableDebug(true);
    bus.enableDebug(true);
    for(auto& dev : bus.get_devices()) {
        dev.second->enableDebug(true);
    }

}
