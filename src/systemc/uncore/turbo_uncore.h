#include <systemc>
using namespace sc_core;
using namespace std;
#include <tlm.h>
using namespace tlm;
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

#include "util/dbg_component.h"
#include "sc_devices.h"
#include "riscv/cfg.h"

class sc_bus_device;

class turbo_uncore : public sc_module, public debug_component {
    SC_HAS_PROCESS(turbo_uncore);

public:
    turbo_uncore(sc_module_name nm, const cfg_t* cfg);
    ~turbo_uncore() = default;

    // sockets // FIXED for now. make a config
    tlm_utils::simple_target_socket<turbo_uncore, 256 /*width*/, tlm::tlm_base_protocol_types> SC_NAMED(targ_socket); // from sc_spike
    tlm_utils::simple_initiator_socket<turbo_uncore, 256 /*width*/, tlm::tlm_base_protocol_types> SC_NAMED(init_socket); // to mir

    void make_mems(const std::vector<mem_cfg_t>& layout);

    tlm::tlm_sync_enum nb_transport_bw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t);
    tlm::tlm_sync_enum nb_transport_fw(tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_time& t);

    void set_debug();

private:
    sc_bus_device SC_NAMED(bus);
};
