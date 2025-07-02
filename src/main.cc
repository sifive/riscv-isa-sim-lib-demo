// Copyright 2025 SiFive
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "demo_core.h"
#include "memory_simulator.h"
#include "riscv/cfg.h"
#include "riscv/remote_bitbang.h"
#include "riscv/jtag_dtm.h"
#include <filesystem>
#include <iostream>

#define START_PC 0x20000000
// #define START_PC 0x10110000


int main(int argc, char* argv[]) {
    // configuration time
    cfg_t cfg;
    
    cfg.isa = "rv64imafdcv";  // Standard RV64GC ISA
    cfg.priv = "MSU";        // Machine, Supervisor, and User privilege levels
    cfg.misaligned = false;  // Don't allow misaligned memory accesses
    cfg.endianness = endianness_little;  // Little endian
    cfg.start_pc = START_PC;  // Start PC
    cfg.mem_layout.clear();
    cfg.pmpregions = 16;
    cfg.hartids = {0};

    debug_module_config_t dm_config; // all default params

    uint16_t rbb_port = 0;
    bool use_rbb = false;
    unsigned dmi_rti = 0; // TODO: check if this should be parsed from command line


    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg.find("--rbb-port") != std::string::npos) {
            size_t pos = arg.find("=");
            if (pos != std::string::npos) {
                rbb_port = std::stoi(arg.substr(pos + 1));
            } 
            std::cout << "RBB port set to " << rbb_port << std::endl;
            use_rbb = true;
        }
    }


    // creating external simulator
    #ifdef USE_BRIDGE
    memory_simulator mem_sim(1024 * 1024 * 1024, START_PC);
    memory_sim_bridge ext_sim(&mem_sim);
    #else
    memory_simulator_wrapper ext_sim(1024 * 1024 * 1024, START_PC); // TODO: update naming
    #endif
    // setting cfg field from external simulator
    cfg.external_simulator = &ext_sim;

    demo_core demo_riscv_core(&cfg, dm_config);
    std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);
    std::unique_ptr<jtag_dtm_t> jtag_dtm(new jtag_dtm_t(&demo_riscv_core.debug_module, dmi_rti));

    if (use_rbb) {
      remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
      demo_riscv_core.set_remote_bitbang(&(*remote_bitbang));
    }


    // this is runtime from this point
    // first reset and load elf
    demo_riscv_core.reset();
    if (!std::filesystem::exists("sw/main.elf")) {
        std::cerr << "Error: sw/main.elf not found. Please build the software first.\n";
        exit(1);
    }
    uint64_t entry_point;
    ext_sim.load_elf_file("sw/main.elf", &entry_point);
    assert(entry_point == START_PC);
    // enable debugging features
    demo_riscv_core.enable_debug();
    demo_riscv_core.configure_log(true, false);

    while (1)
        demo_riscv_core.step(5000);

    return 0;
}
