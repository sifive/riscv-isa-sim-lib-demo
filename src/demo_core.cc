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
#include "riscv/mmu.h"
#include "riscv/processor.h"
#include "riscv/devices.h"


char* demo_core::addr_to_mem(reg_t paddr) {
  auto desc = bus->find_device(paddr >> PGSHIFT << PGSHIFT, PGSIZE);
  if (auto mem = dynamic_cast<abstract_mem_t*>(desc.second))
    if (paddr - desc.first < mem->size())
      return mem->contents(paddr - desc.first);

  return NULL;
}


void demo_core::set_rom()
{
  const int reset_vec_size = 8;

  reg_t start_pc = 0x20000000;

  uint32_t reset_vec[reset_vec_size] = {
    0x297,                                      // auipc  t0,0x0
    0x28593 + (reset_vec_size * 4 << 20),       // addi   a1, t0, &dtb
    0xf1402573,                                 // csrr   a0, mhartid
    get_core(0)->get_xlen() == 32 ?
      0x0182a283u :                             // lw     t0,24(t0)
      0x0182b283u,                              // ld     t0,24(t0)
    0x28067,                                    // jr     t0
    0,
    (uint32_t) (start_pc & 0xffffffff),
    (uint32_t) (start_pc >> 32)
  };
    for (int i = 0; i < reset_vec_size; i++)
      reset_vec[i] = to_le(reset_vec[i]);

  std::vector<char> rom((char*)reset_vec, (char*)reset_vec + sizeof(reset_vec));

  const int align = 0x1000;
  rom.resize((rom.size() + align - 1) / align * align);

  std::shared_ptr<rom_device_t> boot_rom(new rom_device_t(rom));
  
  bus->add_device(DEFAULT_RSTVEC, boot_rom.get());
}

demo_core::demo_core(const cfg_t* cfg):
    cfg(cfg)
{
    abstract_device_t* bus_fallback = nullptr;

    if (cfg->external_simulator.has_value()) {
        auto* ext_sim = cfg->external_simulator.value();
        bus_fallback = new external_sim_device_t(ext_sim);
    }

    if (bus_fallback != nullptr) {
        bus = std::make_unique<bus_t>(bus_fallback);
    } else {
        bus = std::make_unique<bus_t>();
    }

    for (auto id : cfg->hartids) {
        harts[id] = new processor_t(
            cfg->isa, 
            cfg->priv, 
            cfg, 
            this, 
            id, 
            false, 
            log_file.get(),  // Pass log file handle to processor
            std::cout);
        procs.push_back(harts[id]);
    }
}

demo_core::~demo_core() {
}

bool demo_core::reservable(reg_t paddr) { 
    return true; 
}

bool demo_core::mmio_fetch(reg_t paddr, size_t len, uint8_t* bytes) {
    // printf("mmio_fetch address: %lx\n"); 
    return bus->load(paddr, len, bytes); 
}

bool demo_core::mmio_load(reg_t paddr, size_t len, uint8_t* bytes) { 
    // printf("mmio_load address: %lx\n");
    return bus->load(paddr, len, bytes); 
}

bool demo_core::mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) { 
    // printf("mmio_store address: %lx\n");
    return bus->store(paddr, len, bytes); 
}

const std::map<size_t, processor_t*>& demo_core::get_harts() const { 
    return harts; 
}

void demo_core::proc_reset(unsigned id) { 
    // TODO 
}

const cfg_t& demo_core::get_cfg() const { 
    return *cfg; 
}

const char* demo_core::get_symbol(uint64_t paddr) {
    for(const auto& [name, addr] : symbols) {
        if(addr == paddr) {
            return name.c_str();
        }
    }
    return nullptr;
}

processor_t* demo_core::get_core(size_t i) { 
    return procs.at(i); 
}

void demo_core::reset() {
    for(auto& proc : procs) {
        proc->reset();
    }
}

void demo_core::step(size_t n) {
    for(auto& proc : procs) {
        proc->step(n);
    }
}

void demo_core::enable_debug(bool enable) {
    debug = enable;
    for (auto& proc : procs) {
        proc->set_debug(enable);
    }
}

void demo_core::configure_log(bool enable_log, bool enable_commitlog) {
    log = enable_log;
    if (enable_commitlog) {
        for (auto& proc : procs) {
            proc->enable_log_commits();
        }
    }
}

FILE* demo_core::get_log_file() { 
    return log_file.get(); 
}
