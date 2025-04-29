
# RISC-V Test Program

This directory contains a test program for the RISC-V ISA simulator demonstration.

## Program Description

The test program demonstrates basic RISC-V vector operations and multi-core functionality:
- Core 0 performs vector addition operations using RISC-V V-extension
- Other cores enter Wait-For-Interrupt (WFI) state
- Program signals completion by writing to a predefined memory location

## Prerequisites

1. Set the RISCV_PATH environment variable:
```bash
export RISCV_PATH=<path to RISCV tools>
```

## Configuration

1. SCR_BASE in `main.c`:
   The SCR_BASE macro must be defined to match your SoC's configuration. Adjust this value according to your core.dts configuration file. Current value is set to:
   ```c
   #define SCR_BASE 0x3fffb000
   ```

## Building

Build the program:
```bash
make
```

This will generate:
- `main.elf`: The executable
- `main.lst`: Disassembly listing

## Cleaning

To clean build artifacts:
```bash
make clean
```

