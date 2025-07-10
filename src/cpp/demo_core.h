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

#ifndef DEMO_CORE_H
#define DEMO_CORE_H

#include "riscv/simif.h"      // needed for base class
#include "riscv/log_file.h"   // for log_file_t
#include "riscv/debug_module.h"

#include <map>
#include <memory>
#include <vector>

// Forward declarations
class processor_t;
class cfg_t;
class bus_t;
class memory_sim_bridge;
class remote_bitbang_t;
class debug_module_config_t;


class demo_core : public simif_t {
    public:

    friend class processor_t;
    friend class mmu_t;

    demo_core(const cfg_t* cfg, const debug_module_config_t& dm_config);
    ~demo_core();
    
    // pure virtual functions from simif_t
    char* addr_to_mem(reg_t paddr) override;
    bool reservable(reg_t paddr) override;
    bool mmio_fetch(reg_t paddr, size_t len, uint8_t* bytes) override;
    bool mmio_load(reg_t paddr, size_t len, uint8_t* bytes) override;
    bool mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) override;
    const std::map<size_t, processor_t*>& get_harts() const override;
    void proc_reset(unsigned id) override;
    const cfg_t& get_cfg() const override;
    const char* get_symbol(uint64_t paddr) override;
    // end pure virtual functions

    void set_rom();
    processor_t* get_core(size_t i);
    void reset();
    void step(size_t n);
    void enable_debug(bool enable = true);
    void configure_log(bool enable_log, bool enable_commitlog = false);
    FILE* get_log_file();

    private:
        std::map<size_t, processor_t*> harts;
        std::vector<processor_t*> procs;
        std::map<std::string, uint64_t> symbols;
        const cfg_t* const cfg;
        std::unique_ptr<bus_t> bus;
        bool debug{false};
        bool log{false};
        log_file_t log_file{"out.txt"}; // Default log file
        // for GDB
        remote_bitbang_t* remote_bitbang{nullptr};
    public:
    debug_module_t debug_module;
    void set_remote_bitbang(remote_bitbang_t* remote_bitbang) {
        this->remote_bitbang = remote_bitbang;
    }
};

#endif
