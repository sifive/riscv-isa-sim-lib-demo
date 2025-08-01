This is the software part of the XPERIMENTAL package.
It contains a simple RISC-V program that uses XPERIA and XPERIV instructions.

## Prerequisites

Set RISCV toolchain path:

```bash
export RISCV_PATH=<path to RISCV tools>
```

## Building

```bash
make        # Build program
make clean  # Clean artifacts
```

Generates:
- `main.elf`: Executable binary
- `main.lst`: Disassembly listing file

In the main.lst file, you can see the disassembly of the program, including the custom instructions.

Custom instructions will look like:

```
...
    80000018:	00c5850b          	.word	0x00c5850b
...
    80000048:	0020022b          	.word	0x0020022b
...
```
