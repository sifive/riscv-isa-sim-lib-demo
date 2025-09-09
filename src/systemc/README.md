# SystemC RISC-V ISA Simulator Integration

This project demonstrates how to integrate the [RISC-V ISA Simulator (Spike)](https://github.com/riscv-software-src/riscv-isa-sim) with SystemC, creating a SystemC wrapper that embeds the RISC-V simulator within a SystemC environment.

## Overview

The SystemC integration provides:
- A multi-core RISC-V processor wrapped in SystemC modules
- TLM-based memory interface for SystemC simulation
- Debug support with JTAG interface (re-used from Spike)
- Integration with external SystemC testbenches

## Architecture

The system consists of three main components:
- **`turbo_core`**: SystemC wrapper for the RISC-V processor core
- **`turbo_uncore`**: Uncore logic and interconnect
- **`mir_tlm_bare`**: TLM-based memory model

## Prerequisites

1. **RISC-V Toolchain**: Set the toolchain path:
```bash
export RISCV_PATH=<path to RISCV tools>
```

2. **SystemC**: Ensure SystemC is installed and set:
```bash
export SYSTEMC_INCLUDE=<path to SystemC headers>
export SYSTEMC_LIBDIR=<path to SystemC libraries>
```

3. **Spike Headers**: Set the Spike source directory:
```bash
export SPIKE_INCLUDE_DIR=<path to spike headers>
```

## Building

### Option 1: Using System-Installed Spike
If Spike is installed on your system, simply run:
```bash
make demo
```

### Option 2: Using Custom Spike Build
Set the include and library paths:
```bash
make SPIKE_INCLUDE_DIR=/path/to/spike/headers demo
```

### Building Test Software (Optional)
```bash
cd sw
make
```

## Configuration

### ISA Configuration
The processor is configured in `sc_main.cpp`:
```cpp
cfg.isa = "rv64imafdcv_zicsr";  // ISA string with extensions
cfg.priv = "MSU";                        // Privilege levels
cfg.start_pc = 0x20000000;              // Start PC address
```

### Memory Layout
The system uses predefined memory regions. Adjust `SCR_BASE` in test software to match your configuration:
```c
#define SCR_BASE 0x3fffb000
```

## Test Software

The included test program (`sw/main.c`) demonstrates:
- Multi-core operation (Core 0 active, others in WFI)
- Vector operations using RISC-V vector extension
- Memory-mapped I/O for simulation control

### Program Termination
The test uses a finisher register at `SCR_BASE + 0x8`:
- Writing `0x5555`: Successful completion ("PASS")
- Writing other values: Failure indication

## Running

Execute the SystemC simulation:
```bash
./demo
```

## Debug Support

Enable debug mode by modifying the `enable_debug` flag in `sc_main.cpp`. This provides:
- Instruction tracing
- Memory access logging
- JTAG debug interface support