# RISC-V ISA Simulator (Spike) as a Library Demo

This project demonstrates how to use [RISC-V ISA Simulator](https://github.com/riscv-software-src/riscv-isa-sim) as a library, and integrate it with an external simulator. 
In this implementation, ALL memory operations (loads, stores, and instruction fetches) are handled by the external simulator, without any of Spike's internal memories.

## Important Notes

### Hardcoded Parameters
Several parameters are hardcoded in `src/main.cc`:
- Start PC: `0x20000000`
- Memory size: 1GB (1024 * 1024 * 1024 bytes)
- ISA string: "rv64imafdcvh_zba_zbb_zbs_zcb_zcmop_zicond_zkr_zfa_zfbfmin_zfh_zkt_zicbop_zicbom_zicboz_zicfiss_zicfilp_zimop_zawrs_zifencei_zicsr_zihintpause_zihintntl_zicntr_zihpm_zvl1024b_zvfh_zvfbfmin_zvfbfwma_zvbb_zvkt_smcsrind_sscsrind_smrnmi"
- Privilege levels: "MSU" (Machine, Supervisor, and User)

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

## Components

### s2_demo_proc
A custom processor implementation that inherits from Spike's `simif_t`. It provides minimal functionalities:
- Adds external simulator device to bus as a fallback parameter
- Check constructor of `s2_demo_proc` for details

### Memory Simulator
The memory simulation is structured with a base class and two implementation approaches:

- `memory_simulator`: Base class implementing core memory operations:
  - Read/write operations
  - ELF file loading
  - ROM content initialization
  - Sparse array-based memory management

Two different ways to integrate with Spike:
1. `memory_simulator_wrapper`: Inherits from both `memory_simulator` and `abstract_sim_if_t`
2. `spike_bridge_t`: Bridge pattern implementation that wraps a `memory_simulator` instance

Both approaches ensure that all memory accesses (including instruction fetches) are handled by the external simulator rather than Spike's internal memory.

### Memory Simulator Integration Approaches

The project provides two different ways to integrate the external memory simulator with Spike:

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
Wrapper class that uses composition to integrate the memory simulator with Spike:

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

### Main Program
`main.cc` demonstrates the usage:
```cpp
// Choose implementation method at compile time
#ifdef USE_BRIDGE
    memory_simulator mem_sim(1024 * 1024 * 1024);
    spike_bridge_t ext_sim(&mem_sim);
#else
    memory_simulator_wrapper ext_sim(1024 * 1024 * 1024);
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


## External Simulator Integration Guide

This section details the complete process of integrating an external simulator with Spike.

### 1. Core Components Required

1. Memory Simulator Implementation:
   - Must inherit from `memory_simulator` base class
   - Implements core memory operations
   - Handles memory allocation and management

2. Spike Interface:
   - Must implement `abstract_sim_if_t` interface
   - Handles communication between Spike and memory simulator
   - Choose either wrapper or bridge approach

3. Processor Configuration:
   - Configure `s2_demo_proc` with external simulator
   - Set up debugging and logging options

### 2. Step-by-Step Integration

#### Step 2.1: Implement Memory Simulator

First, create your memory simulator by inheriting from the base class:

```cpp
class your_memory_simulator : public memory_simulator {
public:
    your_memory_simulator(uint64_t size) : memory_simulator(size) {
        // Your initialization code
    }

    // Optional: Override base class methods if needed
    void write(uint64_t addr, const uint8_t* data, size_t len) override {
        // Your implementation
    }

    void read(uint64_t addr, uint8_t* data, size_t len) override {
        // Your implementation
    }
};
```

#### Step 2.2: Choose Integration Method

Either use Wrapper approach:

```cpp
class your_simulator_wrapper : public your_memory_simulator, 
                             public abstract_sim_if_t {
public:
    your_simulator_wrapper(uint64_t size) 
        : your_memory_simulator(size) {}

    bool load(reg_t addr, size_t len, uint8_t* bytes) override {
        read(addr, bytes, len);
        return true;
    }

    bool store(reg_t addr, size_t len, const uint8_t* bytes) override {
        write(addr, bytes, len);
        return true;
    }
};
```

Or Bridge approach:

```cpp
class your_simulator_bridge : public abstract_sim_if_t {
private:
    your_memory_simulator* sim;

public:
    your_simulator_bridge(your_memory_simulator* simulator) : sim(simulator) {}

    bool load(reg_t addr, size_t len, uint8_t* bytes) override {
        sim->read(addr, bytes, len);
        return true;
    }

    bool store(reg_t addr, size_t len, const uint8_t* bytes) override {
        sim->write(addr, bytes, len);
        return true;
    }
};
```

#### Step 2.3: Configure Processor

Set up the processor with your external simulator:

```cpp
// Create configuration
cfg_t cfg;

// Initialize your simulator
#ifdef USE_BRIDGE
    your_memory_simulator mem_sim(1024 * 1024 * 1024);
    your_simulator_bridge ext_sim(&mem_sim);
#else
    your_simulator_wrapper ext_sim(1024 * 1024 * 1024);
#endif

// Set external simulator in configuration
cfg.external_simulator = &ext_sim;

// Create processor with configuration
s2_demo_proc spike_proc(&cfg);
```

### 3. Available APIs

#### Spike Interface APIs

```cpp
class abstract_sim_if_t {
public:
    // Required memory operations
    virtual bool load(reg_t addr, size_t len, uint8_t* bytes) = 0;
    virtual bool store(reg_t addr, size_t len, const uint8_t* bytes) = 0;
};
```

### 4. Usage Example

Complete example of setting up and using the external simulator:

```cpp
int main() {
    // 1. Create configuration
    cfg_t cfg;

    // 2. Initialize external simulator
    #ifdef USE_BRIDGE
        your_memory_simulator mem_sim(1024 * 1024 * 1024);
        your_simulator_bridge ext_sim(&mem_sim);
    #else
        your_simulator_wrapper ext_sim(1024 * 1024 * 1024);
    #endif

    // 3. Configure external simulator
    cfg.external_simulator = &ext_sim;

    // 4. Create and configure processor
    s2_demo_proc spike_proc(&cfg);
    spike_proc.reset();

    // 5. Load program
    ext_sim.load_elf_file("your_program.elf");

    // 6. Configure debugging (optional)
    spike_proc.enable_debug();
    spike_proc.configure_log(true, false);

    // 7. Run simulation
    while (1) {
        spike_proc.step(50);
        // Add your simulation control logic here
    }

    return 0;
}
```


