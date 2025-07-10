#include "turbo/turbo_core.h"
#include "uncore/turbo_uncore.h"
#include "memory/memory.h"
#include "riscv/remote_bitbang.h"
#include "riscv/jtag_dtm.h"


#define START_PC 0x20000000

class testbench : public sc_module {
public:
    SC_HAS_PROCESS(testbench);

    sc_in<bool> SC_NAMED(clk_i);

    testbench(sc_module_name nm, const cfg_t &cfg, const debug_module_config_t &dm_config, bool enable_debug)
    : sc_module(nm) {
        SC_THREAD(run);

        // creating core, uncore, and memory
        core = std::make_unique<turbo_core>("core", &cfg, dm_config);
        uncore = std::make_unique<turbo_uncore>("uncore", &cfg);
        mem = std::make_unique<mir_tlm_bare>("mem");

        // connect sockets and clock
        mem->clock(clk_i);
        core->init_socket(uncore->targ_socket);
        uncore->init_socket(mem->target_socket);
        
        //auto start_pc = mem->mem_loader.loadElf("rot13-64");
        auto start_pc = mem->mem_loader.loadElf("sw/main.elf");
        cout << "Start pc = " << start_pc << endl;

        if (enable_debug) {
            core->enableDebug(true);
            mem->enableDebug();
            uncore->set_debug();
            core->configure_log(true, false);
        }

        // load "ROM"
        uint64_t reset_vect_0 = start_pc;
        uint8_t reset_vec_size = 8;
        std::vector<uint32_t> reset_vec_data;
        reset_vec_data.reserve(reset_vec_size);
        reset_vec_data.push_back(0x297);                                // auipc  t0,0x0
        reset_vec_data.push_back(0x28593 + (reset_vec_size * 4 << 20)); // addi   a1, t0, &dtb
        reset_vec_data.push_back(0xf1402573);                           // csrr   a0, mhartid
        reset_vec_data.push_back(0x0182b283); // lw/ld  t0,24(t0)
        reset_vec_data.push_back(0x28067);
        reset_vec_data.push_back(0);
        reset_vec_data.push_back(reset_vect_0 & 0xffffffff);
        reset_vec_data.push_back(reset_vect_0 >> 32);
        uint64_t start_addr = 0x1000;
        mem->mem_loader.loadVector(reset_vec_data, start_addr);
    }

    ~testbench() = default;

    std::unique_ptr<turbo_core> core;
    std::unique_ptr<turbo_uncore> uncore;
    std::unique_ptr<mir_tlm_bare> mem;

private:
    void run() {
        wait(10, SC_NS);
        core->trigger_start();
    }
};

int sc_main(int argc, char* argv[]) {
    // Parse some command line arguments
    // TODO: use program options parser
    bool enable_debug = false;
    uint16_t rbb_port = 0;
    bool use_rbb = false;
    unsigned dmi_rti = 0; // TODO: check if this should be parsed from command line


    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--debug" || arg == "-d") {
            enable_debug = true;
            std::cout << "Debug logging enabled" << std::endl;
        } else if (arg.find("--rbb-port") != std::string::npos) {
            size_t pos = arg.find("=");
            if (pos != std::string::npos) {
                rbb_port = std::stoi(arg.substr(pos + 1));
            } 
            std::cout << "RBB port set to " << rbb_port << std::endl;
            use_rbb = true;
        }
    }

    cfg_t cfg;
    cfg.isa = "rv64imafdcv_zicsr";  // Standard RV64GC ISA
    cfg.priv = "MSU";        // Machine, Supervisor, and User privilege levels
    cfg.misaligned = false;  // Don't allow misaligned memory accesses
    cfg.endianness = endianness_little;  // Little endian
    cfg.start_pc = START_PC;  // Start PC
    cfg.mem_layout.clear();
    cfg.pmpregions = 16;
    cfg.hartids = {0};

    debug_module_config_t dm_config; // all default params

    // Create testbench
    testbench SC_NAMED(tb, cfg, dm_config, enable_debug);

    std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);

    std::unique_ptr<jtag_dtm_t> jtag_dtm(
        new jtag_dtm_t(&tb.core->debug_module, dmi_rti));
    if (use_rbb) {
      remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
      tb.core->set_remote_bitbang(&(*remote_bitbang));
    }

    sc_clock SC_NAMED(clk, 10, SC_NS);
    tb.clk_i(clk);
    
    // Start simulation
    sc_start();
    
    return 0;
}
