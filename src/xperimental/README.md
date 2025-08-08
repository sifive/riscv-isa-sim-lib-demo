# XPERIMENTAL package

This an example of how to add custom extensions to Spike.

We define 2 custom extensions, which can be dynamically linked with Spike (XPERIA and XPERIV)

* XPERIA is a scalar extension, which include 1 instruction
* XPERIV is a vector extension, which include 1 instruction

## Folder structure

* xperimental_ext
  - 2 .cc files for extensions `xperia.cc` and `xperiv.cc`
  - Makefile to build `libxperi.so`
  - additional files: ???
* xperimental_sw
  - Example code, which triggers these insns `peria.add` and `periv.add`
  - 2 files for software `main.c`
  - linker script `script.ld` and startup code `start.S`
  - Makefile to build `main.elf` and `main.lst`

## Running

Please use spike build from `git@github.com:riscv-software-src/riscv-isa-sim.git`

Assumin that you have built Spike and installed it into `/tools/riscv-isa-sim/20250801`

running with spike for 42 instructions:

```
LD_LIBRARY_PATH=/tools/riscv-isa-sim/20250801/lib /tools/riscv-isa-sim/20250801/bin/spike -m0x8000:0x2000,0x28000:0x1000,0x30000:0x1000,0x38000:0x1000,0x6b000:0x1000,0x80000:0x1000,0x88000:0x1000,0x140000:0x10000,0x1700000:0x10000,0x20d0000:0x2000000,0x20000000:0x20000000,0x40000000:0x20000000,0x7f000000:0x1000000000 --isa rv64imafdcv_zifencei_zicsr_zvl1024b_xperia_xperiv --extlib=./xperimental_ext/libxperi.so --instructions=42 -l --log=log.txt ./xperimental_sw/main.elf
```

The execution log is in `log.txt` file.

You will see the custom instructions with correct disassembly in the log file.

```
core   0: 0x0000000080000018 (0x00c5850b) peri.a.add a1
...
core   0: 0x0000000080000048 (0x0020022b) peri.v.add v4, v0, v2
```

PRs in Spike:
* ???


