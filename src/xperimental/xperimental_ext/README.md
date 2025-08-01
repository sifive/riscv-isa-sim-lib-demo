first you need to build spike from source and install it into your system.

Please use spike build from `git@github.com:riscv-software-src/riscv-isa-sim.git`

## Folder structure

* 2 .cc files for extensions `xperia.cc` and `xperiv.cc`
* Makefile to build `libxperi.so`
* `extension.h, processor.h, decode.h, insn_macros.h, decode_macros.h` headers from spike source tree.

## Prerequisites

Set SPIKE_DIR is path where Spike is installed:

```bash
export SPIKE_DIR=<path to riscv-isa-sim intallation>
```

## Building

```bash
make        # Build library
make clean  # Clean artifacts
```
