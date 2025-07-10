#ifndef _SC_DEVICES_H_
#define _SC_DEVICES_H_

#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include "util/dbg_component.h"

#include <map>

class sc_device : public sc_module, public debug_component {
public:
    SC_HAS_PROCESS(sc_device);
    sc_device(sc_module_name nm) : sc_module(nm), debug_component(std::string(nm)) {} // TODO: fix name
    virtual ~sc_device() = default;
    virtual void write(uint64_t addr, const uint8_t* data, size_t len) {}
    virtual void read(uint64_t addr, uint8_t* data, size_t len) {} // TODO IF
};

class sc_mem_device : public sc_device {
public:
    SC_HAS_PROCESS(sc_mem_device);
    sc_mem_device(sc_module_name nm, uint64_t bs, uint64_t sz)
    : sc_device(nm) 
    , base(bs)
    , size(sz)
    {
    }

    uint64_t get_base() { return base; }
    uint64_t get_size() { return size; }

protected:
    uint64_t size;
    uint64_t base;
};

class sc_bus_device : public sc_module, public debug_component {
public:
    SC_HAS_PROCESS(sc_bus_device);

    sc_bus_device(sc_module_name nm): sc_module(nm), debug_component(std::string(nm)) {}

    void register_device(uint64_t base, uint64_t size, sc_device* dev) {
        devices_with_size[std::make_pair(base, size)] = dev;
    }

    void register_device(sc_device* dev) {
        sc_mem_device* mem_dev = dynamic_cast<sc_mem_device*>(dev);
        if(mem_dev) {
            register_device(mem_dev->get_base(), mem_dev->get_size(), mem_dev);
            return;
        }
    }

    sc_device* find_device(uint64_t addr) {
        for(auto& dev : devices_with_size) {
            if(addr >= dev.first.first && addr < dev.first.first + dev.first.second) {
                LOG_DBG("Found device " << dev.second->name() << " for address " << hex << addr);
                return dev.second;
            }
        }
        return nullptr;
    }

    void print_devices() {
        for(auto& dev : devices_with_size) {
            cout << "Device " << dev.second->name() << " at " << hex << dev.first.first << " with size " << hex << dev.first.second << endl;
        }
    }

    std::map<std::pair<uint64_t, uint64_t>, sc_device*>& get_devices() {
        return devices_with_size;
    }

private:
  std::map<std::pair<uint64_t, uint64_t>, sc_device*> devices_with_size;
};

class sc_soc_scr : public sc_mem_device {
public:
    SC_HAS_PROCESS(sc_soc_scr);
    sc_soc_scr(sc_module_name nm, uint64_t bs, uint64_t sz)
    : sc_mem_device(nm, bs, sz)
    {
    }
    void read(uint64_t addr, uint8_t* data, size_t len) override {
        LOG_DBG("read from " << hex << addr);
        uint64_t offset = addr - base;
        if(offset == 0x0) {
            LOG_DBG("read from version id");
        } else if (offset == 0x8) {
            LOG_DBG("read from test status");
            assert(0); // TODO: This is just TMP
        }
    }
    void write(uint64_t addr, const uint8_t* data, size_t len) override {
        LOG_DBG("write to " << hex << addr);
        uint64_t offset = addr - base;
        if(offset == 0x8) {
            cout << "write to soc scr test status" << endl;
            cout << "End simulation with status: ";
            if (((uint32_t*)data)[0] == 0x5555) {
                cout << "PASS" << endl;
            } else {
                cout << "FAIL. Value: 0x" << hex << ((uint32_t*)data)[0] << endl;
            }
            assert(0);
        }
    }
};

class sc_cache_device : public sc_device {
public:
    SC_HAS_PROCESS(sc_cache_device);
    sc_cache_device(sc_module_name nm, uint64_t bs, uint64_t sz)
    : sc_device(nm)
    , base(bs)
    , size(sz)
    {
    }

    uint64_t get_base() { return base; }
    uint64_t get_size() { return size; }

    void read(uint64_t addr, uint8_t* data, size_t len) override {
        LOG_DBG("read from " << hex << addr);
        uint64_t offset = addr - base;
        uint64_t line_addr = offset / line_size;
        uint64_t line_offset = offset % line_size;
        if(cache.find(line_addr) == cache.end()) {
            LOG_DBG("Cache miss");
        } else {
            LOG_DBG("Cache hit");
            std::copy(cache[line_addr].data + line_offset, cache[line_addr].data + line_offset + len, data);
        }
    }

    void write(uint64_t addr, const uint8_t* data, size_t len) override {
        LOG_DBG("write to " << hex << addr);
        uint64_t offset = addr - base;
        uint64_t line_addr = offset / line_size;
        uint64_t line_offset = offset % line_size;
        if(cache.find(line_addr) == cache.end()) {
            cache[line_addr] = cache_line();
        }
        std::copy(data, data + len, cache[line_addr].data + line_offset);
        cache[line_addr].valid = true;
    }


private:
    class cache_line {
    public:
        cache_line() : valid(false) {}
        bool valid;
        uint8_t data[64];
    };
    std::map<uint64_t, cache_line> cache;
    uint64_t line_size = 64;
    uint64_t associativity = 1;

    uint64_t size; // in bytes
    uint64_t base;
};

#endif
