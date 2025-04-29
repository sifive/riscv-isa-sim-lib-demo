#include "memory_simulator.h"
#include <cstring>
#include <cassert>

memory_simulator::memory_simulator(uint64_t size, uint64_t start_pc): mem_size(size), start_pc(start_pc) {
    printf("creating memory_simulator\n");
    set_rom_contents();
}

memory_simulator::~memory_simulator() {}

void memory_simulator::write(uint64_t addr, const uint8_t* data, size_t len) {

    if (unlikely(addr == 0x3fffb008)) {
      const unsigned int testStatus = ((unsigned int*)data)[0];

      if (testStatus != 0) {
        if (testStatus == 0x5555) {
          printf("PASS\n");
        } else {
          printf("FAIL with status %x\n", testStatus);
        }
        assert(0);
      } 
    } 
    sparse_arr.write(addr, data, len);
}

void memory_simulator::read(uint64_t addr, uint8_t* data, size_t len) {
    sparse_arr.read(addr, data, len);
}

std::map<std::string, uint64_t> memory_simulator::load_elf_file(const std::string& filename, uint64_t* entry_point) {
    if (entry_point) {
        return load_elf(filename.c_str(), &sparse_arr, entry_point);
    } else {
        uint64_t dummy_entry;
        return load_elf(filename.c_str(), &sparse_arr, &dummy_entry);
    }
}

void memory_simulator::load_hex_file(const std::string& filename) {
    // TODO: Implement hex file loading
}

uint64_t memory_simulator::size() const {
    return mem_size;
}

// Sets up initial ROM contents with reset vector and boot code
void memory_simulator::set_rom_contents() {
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

    uint8_t* tmp_mem = new uint8_t[reset_vec_data.size() * sizeof(uint32_t)];
    for(int i = 0; i < reset_vec_data.size(); i++) {
        memcpy(tmp_mem + i * sizeof(uint32_t), &reset_vec_data[i], sizeof(uint32_t));
    }
    sparse_arr.write(start_addr, tmp_mem, reset_vec_data.size() * sizeof(uint32_t));
    delete[] tmp_mem;
}


////////////////// wrapper
memory_simulator_wrapper::memory_simulator_wrapper(uint64_t size, uint64_t start_pc): memory_simulator(size, start_pc) {
    printf("creating memory_simulator_wrapper\n");
}

memory_simulator_wrapper::~memory_simulator_wrapper() {
}

bool memory_simulator_wrapper::load(reg_t addr, size_t len, uint8_t* bytes) {
    // if (addr + len > mem_size) {
    //     return false;
    // }
    memory_simulator::read(addr, bytes, len);
    // sparse_arr.read(addr, bytes, len);
    return true;
}

bool memory_simulator_wrapper::store(reg_t addr, size_t len, const uint8_t* bytes) {
    // if (addr + len > mem_size) {
    //     printf("store addr = %lx\n", addr);
    //     printf("store len = %lx\n", len);
    //     return false;
    // }
    memory_simulator::write(addr, bytes, len);
    return true;
}

////////////// bridge

spike_bridge_t::spike_bridge_t(memory_simulator* sim, uint64_t start_pc) : sim(sim) {
    sim->set_start_pc(start_pc);
    printf("creating spike_bridge_t\n");
}

bool spike_bridge_t::load(reg_t addr, size_t len, uint8_t* bytes) {
    printf("calling spike_bridge_t load\n");
    assert(sim != nullptr);
    
    if (addr + len > sim->size()) {
        return false;
    }

    sim->read(addr, bytes, len);
    return true;
}

bool spike_bridge_t::store(reg_t addr, size_t len, const uint8_t* bytes) {
    printf("calling spike_bridge_t store\n");
    assert(sim != nullptr);
    if (addr + len > sim->size()) {
        return false;
    }

    sim->write(addr, bytes, len);
    return true;
}

void spike_bridge_t::load_elf_file(const std::string& filename) {
    sim->load_elf_file(filename);
}
