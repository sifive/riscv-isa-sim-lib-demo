
#include "riscv-isa-sim/riscv/processor.h"
#include "riscv/simif.h"      // needed for base class
#include "riscv/cfg.h"        // needed for cfg_t* in header
#include "riscv/processor.h"  // needed for processor_t* in header
#include "riscv/isa_parser.h" // needed for isa_parser_t member
#include "riscv/devices.h"    // for bus_t
#include "riscv/log_file.h"   // for log_file_t

class spike_bridge_t;

class s2_demo_proc : public simif_t {
    public:

    friend class processor_t;
    friend class mmu_t;

    s2_demo_proc(const cfg_t* cfg);
    ~s2_demo_proc();
    
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
};
