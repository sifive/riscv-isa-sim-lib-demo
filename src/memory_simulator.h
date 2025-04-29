#ifndef MEMORY_H
#define MEMORY_H

#include <util/elf.h>
#include <util/elfloader.h>
#include <util/sparse_array.h>
#include <string>
#include <map>
#include <cstdint>
#include "riscv/devices.h"

// base class, has read/write functionalities we need for a simple memory simulator
class memory_simulator {
public:
    memory_simulator(uint64_t size, uint64_t start_pc = 0x20000000);
    virtual ~memory_simulator();

    void write(uint64_t addr, const uint8_t* data, size_t len);
    void read(uint64_t addr, uint8_t* data, size_t len);
    std::map<std::string, uint64_t> load_elf_file(const std::string& filename, uint64_t* entry_point = nullptr);
    void load_hex_file(const std::string& filename); // not implemented yet
    uint64_t size() const;
    void set_rom_contents();
    void set_start_pc(uint64_t start_pc) { this->start_pc = start_pc; }

protected:
    uint64_t mem_size;
    uint64_t start_pc;
private:
    util::sparse_array<uint8_t, 18, 30> sparse_arr;
};

/// one way is to create a wrapper class
class memory_simulator_wrapper : public memory_simulator, public abstract_sim_if_t {
    public:
        memory_simulator_wrapper(uint64_t size, uint64_t start_pc = 0x20000000);
        ~memory_simulator_wrapper() override;
        // from abstract_sim_if_t
        bool load(reg_t addr, size_t len, uint8_t* bytes) override;
        bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
};

// another way to create a bridge
class spike_bridge_t : public abstract_sim_if_t {
public:
    // class name of external simulator is known by the end used
    spike_bridge_t(memory_simulator* sim, uint64_t start_pc = 0x20000000);
    virtual ~spike_bridge_t() = default;
    spike_bridge_t() = delete;
    spike_bridge_t(const spike_bridge_t&) = delete;

    // from abstract_sim_if_t
    bool load(reg_t addr, size_t len, uint8_t* bytes) override;
    bool store(reg_t addr, size_t len, const uint8_t* bytes) override;

    void load_elf_file(const std::string& filename, uint64_t* entry_point = nullptr);

private:
    memory_simulator* sim;
};

#endif
