# RISC-V ISA Simulator as a Library Demo

This project demonstrates how to use [RISC-V ISA Simulator](https://github.com/riscv-software-src/riscv-isa-sim) as a library, and integrate it with an external simulator environment. 
We currrently have two use-cases: 
1. connecting with C++ memory simulator
2. having a SystemC wrapper and embedding it within a SystemC environment

Each use-case has its own folder in which you can find separate `README.md` file with more details.
1. `src/cpp` - C++ memory simulator
2. `src/systemc` - SystemC wrapper
