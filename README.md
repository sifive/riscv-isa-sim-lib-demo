# Spike S2 Demo

This project demonstrates integration of external simulator with Spike (RISC-V ISA Simulator). In this implementation, ALL memory operations (loads, stores, and instruction fetches) are handled by the external simulator, without any of Spike's internal memories.

## Note on Spike Integration
Currently, required Spike files are directly copied into the `riscv-isa-sim/` directory. In future versions, this will be replaced with a proper Git submodule pointing to the Spike repository.

## Components

### s2_demo_proc
A custom processor implementation that inherits from Spike's `simif_t`. It provides minimal functionalities
- it adds external simulator device to bus, as a fallback parameter. Check constructor of `s2_demo_proc` for details.

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

### Build System
The Makefile first configures and builds Spike:
- Configures Spike with proper flags (`-fPIC`, `-O2`, `-std=c++17`)
- Builds and installs Spike libraries in `spike_install` directory
- Provides required libraries:
  - `libriscv.a`
  - `libsoftfloat.a`
  - `libdisasm.a`

Then configures the demo build environment with:
- Integration with the built Spike libraries
- C++17 standard compilation
- Proper include paths for Spike integration

## Modifications w.r.t. Spike Github

### Changes to Spike master
Modified files (some of the files are expected to be updated in PR#1953):
- `riscv/cfg.h`
- `riscv/csr_init.cc`
- `riscv/devices.cc`
- `riscv/devices.h`
- `riscv/encoding.h`
- `riscv/riscv.mk.in`
- `softfloat/softfloat.mk.in`

### Changes relative to branch mslijepc:mslijepc_20250410_ext-sim
Modified files:
- `riscv/csr_init.cc` - SiFive-specific register implementations
- `riscv/encoding.h` - SiFive-specific encodings
- `riscv/riscv.mk.in` - Compilation configuration
- `softfloat/softfloat.mk.in` - Compilation configuration

## Building

```bash
make
```
This will:
1. Configure and build Spike with required flags
2. Install Spike libraries in local `spike_install` directory
3. Build the demo application

## Usage

```bash
./spike_demo
```

The demo loads and executes a test ELF file ("return-pass.elf") with debug and logging enabled. By default, execution logs are written to (hardcoded)`out.txt`.

## Project Structure
```
.
├── riscv-isa-sim/      # Spike simulator source
├── s2_demo_proc.cc     # Custom processor implementation
├── s2_demo_proc.h      # Processor header
├── memory_simulator.h  # Memory simulator base class and implementations
├── memory_simulator.cc # Memory simulator implementation
└── main.cc            # Demo entry point
```



