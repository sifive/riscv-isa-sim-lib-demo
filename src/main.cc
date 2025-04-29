#include "s2_demo_proc.h"
#include "memory_simulator.h"
#include "riscv/cfg.h"
#include <filesystem>

#define START_PC 0x20000000

int main() {
    // configuration time
    cfg_t cfg;
    
    cfg.isa = "rv64imafdcvh_zba_zbb_zbs_zcb_zcmop_zicond_zkr_zfa_zfbfmin_zfh_zkt_zicbop_zicbom_zicboz_zicfiss_zicfilp_zimop_zawrs_zifencei_zicsr_zihintpause_zihintntl_zicntr_zihpm_zvl1024b_zvfh_zvfbfmin_zvfbfwma_zvbb_zvkt_smcsrind_sscsrind_smrnmi";  // Standard RV64GC ISA
    cfg.priv = "MSU";        // Machine, Supervisor, and User privilege levels
    cfg.misaligned = false;  // Don't allow misaligned memory accesses
    cfg.endianness = endianness_little;  // Little endian
    cfg.start_pc = START_PC;  // Start PC
    cfg.mem_layout.clear();
    cfg.pmpregions = 16;
    cfg.hartids = {0};

    // creating external simulator
    #ifdef USE_BRIDGE
    memory_simulator mem_sim(1024 * 1024 * 1024, START_PC);
    spike_bridge_t ext_sim(&mem_sim);
    #else
    memory_simulator_wrapper ext_sim(1024 * 1024 * 1024, START_PC); // TODO: update naming
    #endif
    // setting cfg field from external simulator
    cfg.external_simulator = &ext_sim;

    s2_demo_proc spike_proc(&cfg);


    // this is runtime from this point
    // first reset and load elf
    spike_proc.reset();
    if (!std::filesystem::exists("sw/main.elf")) {
        std::cerr << "Error: sw/main.elf not found. Please build the software first.\n";
        exit(1);
    }
    uint64_t entry_point;
    ext_sim.load_elf_file("sw/main.elf", &entry_point);
    assert(entry_point == START_PC);
    // enable debugging features
    spike_proc.enable_debug();
    spike_proc.configure_log(true, false);

    while (1)
        spike_proc.step(50);

    printf("\n\n *** the end ***\n");

    return 0;
}
