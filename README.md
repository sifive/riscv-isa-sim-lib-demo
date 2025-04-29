# RISC-V ISA Simulator as a Library Demo

This project demonstrates how to use [RISC-V ISA Simulator](https://github.com/riscv-software-src/riscv-isa-sim) as a library, and integrate it with an external simulator. 
In this demonstration, external simulator is a simple memory simulator. In our use case, all memory operations (loads, stores, and instruction fetches) are handled by the external simulator, without any of riscv-isa-sim's internal memories.

## Important Notes

### Hardcoded Parameters
Several parameters are hardcoded in `src/main.cc`:
- Start PC: `0x20000000`
- Memory size: 1GB (1024 * 1024 * 1024 bytes)
- ISA string: "rv64imafdcvh_zba_zbb_zbs_zcb_zcmop_zicond_zkr_zfa_zfbfmin_zfh_zkt_zicbop_zicbom_zicboz_zicfiss_zicfilp_zimop_zawrs_zifencei_zicsr_zihintpause_zihintntl_zicntr_zihpm_zvl1024b_zvfh_zvfbfmin_zvfbfwma_zvbb_zvkt_smcsrind_sscsrind_smrnmi"
- Privilege levels: "MSU" (Machine, Supervisor, and User)
- CPU runs sw from `src/sw/` folder. Check `src/sw/README.md` for more details

### Program Termination
The program will terminate with either:
- "PASS" message: When the test program writes `0x5555` to the finisher address
- "FAIL with status {x}" message: When any other non-zero value is written to the finisher address

## Steps to Build the Project

### Step 1: Clone RISC-V ISA Simulator Repository
Clone the RISC-V ISA Simulator repository and checkout the required version:

```bash
git clone https://github.com/riscv-software-src/riscv-isa-sim.git
cd riscv-isa-sim
git checkout 77ea9deec2fbbe93fc181c5079081c66d7e1504a
cd ..
```

### Step 2: Set Environment Variable
Set the environment variable to point to your RISC-V ISA Simulator repository:

```bash
export SPIKE_SOURCE_DIR=/path/to/riscv-isa-sim
```

### Step 3: Run Setup Script
Run the setup script to patch simulator files:

```bash
./src/setup.sh
```

This script performs several critical tasks:
- Verifies that `SPIKE_SOURCE_DIR` environment variable is set and valid
- Validates repository SHA against expected version
- Copies required modified files to simulator repository:
  1. `softfloat/softfloat.mk.in`
  2. `riscv/riscv.mk.in`
  3. `riscv/mmu.h`
Those 3 files should be upsteamed to riscv-isa-sim Github soon.
PRs are already made [PR 1968](https://github.com/riscv-software-src/riscv-isa-sim/pull/1968) and [PR 1969](https://github.com/riscv-software-src/riscv-isa-sim/pull/1969)

The script creates a `.copy_files_stamp` file to track successful patching. If you need to repatch, remove this file and rerun the script.

### Step 4: Build the Demo
After the setup script completes successfully, build the project:

```bash
make
```
Note: We are forcing C++17 standard in the Makefile

### Optional: Building with Memory Operation Hooks
To enable memory operation observation/debugging, you can build with observability hooks enabled:

```bash
make USE_HOOKS=1
```

This will:
- Include the `hooks.h` header automatically in the build
- Enable printing of memory operations:
  - Load operations: address, data, and length
  - Fetch operations: address, instruction, and length

## Adding External Simulator API

### riscv-isa-sim cfg_t class
class cfg_t has a new field: `external_simulator`. That field is a pointer to an object that implements `abstract_sim_if_t` interface. 

```cpp
class cfg_t {
public:
    // ...
    std::optional<abstract_sim_if_t*> external_simulator;
    // ...
};
```
Usage is in `main.cc`

```cpp
cfg.external_simulator = &ext_sim;
```

### s2_demo_proc
A custom processor implementation that inherits from riscv-isa-sim's `simif_t`.
Since recently, `bus_t` has a fallback parameter, in case no device is found on the bus, request is forwarded to the fallback device. In our case it is the external simulator. 
Example is in constructor of `s2_demo_proc`
```cpp
if (cfg->external_simulator.has_value()) {
    auto* ext_sim = cfg->external_simulator.value();
    bus_fallback = new external_sim_device_t(ext_sim);
}

if (bus_fallback != nullptr) {
    bus = std::make_unique<bus_t>(bus_fallback);
} else {
    bus = std::make_unique<bus_t>();
}
```
### riscv-isa-sim abstract_sim_if_t
This is the interface that external simulator must implement. It has two methods: `load` and `store`.
```cpp
class abstract_sim_if_t {
public:
    virtual bool load(reg_t addr, size_t len, uint8_t* bytes) = 0;
    virtual bool store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
};
```
We use this class to integrate with riscv-isa-sim.

### Components

### Memory Simulator
We have a base class and two implementation approaches:
- `memory_simulator`: Base class implementing core memory operations:
  - Read/write operations
  - ELF file loading
  - ROM content initialization
  - Sparse array-based memory management

Two different ways to integrate with riscv-isa-sim:
1. `memory_simulator_wrapper`: Inherits from both `memory_simulator` and `abstract_sim_if_t`
2. `spike_bridge_t`: Bridge pattern implementation that wraps a `memory_simulator` instance

#### 1. Memory Simulator Wrapper (`memory_simulator_wrapper`)
Direct inheritance approach that combines `memory_simulator` and `abstract_sim_if_t`:

```cpp
class memory_simulator_wrapper : public memory_simulator, public abstract_sim_if_t {
    public:
        memory_simulator_wrapper(uint64_t size);
        ~memory_simulator_wrapper() override;
        bool load(reg_t addr, size_t len, uint8_t* bytes) override;
        bool store(reg_t addr, size_t len, const uint8_t* bytes) override;
};
```

This approach is straightforward and leverages C++ inheritance to create a single class that implements both the memory simulation logic and the Spike interface.

#### 2. Spike Bridge (`spike_bridge_t`)
Wrapper class that uses composition to integrate the memory simulator with riscv-isa-sim:

```cpp
class spike_bridge_t {
    public:
        spike_bridge_t(memory_simulator* sim);
        bool load(reg_t addr, size_t len, uint8_t* bytes);
        bool store(reg_t addr, size_t len, const uint8_t* bytes);
    private:
        memory_simulator* sim;
};
```

This approach uses composition, where the `spike_bridge_t` class holds a pointer to a `memory_simulator` instance and forwards memory operations to it. This can be more flexible, as it allows for easier swapping of memory simulation implementations.

In both approaches we need to define `load` and `store` methods from `abstract_sim_if_t` interface.

### Main Program
`main.cc` demonstrates the usage:
```cpp
// Choose implementation method at compile time
#ifdef USE_BRIDGE
    memory_simulator mem_sim(1024 * 1024 * 1024, START_PC);
    spike_bridge_t ext_sim(&mem_sim);
#else
    memory_simulator_wrapper ext_sim(1024 * 1024 * 1024, START_PC);
#endif

// Initialize and configure processor
s2_demo_proc spike_proc(&cfg);
spike_proc.reset();
ext_sim.load_elf_file("return-pass.elf");

// Enable debugging features
spike_proc.enable_debug();
spike_proc.configure_log(true, true);

// Run simulation, execute 50 instructions
spike_proc.step(50);
```




