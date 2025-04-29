
# RISC-V Test Program

A simple multi-core RISC-V test program where:
- Core 0: Reads data from memory (0x40000000), performs vector addition, and stores the sum back
- Other cores: Enter sleep state (WFI)
- Program completion is signaled by writing 0x5555 to finisher address

## Prerequisites

1. Set RISCV toolchain path:
```bash
export RISCV_PATH=<path to RISCV tools>
```

## Configuration

Adjust SCR_BASE in `main.c` to match your SoC configuration:
```c
#define SCR_BASE 0x3fffb000
```

## Building

```bash
make        # Build program
make clean  # Clean artifacts
```

Generates: `main.elf` and `main.lst`


