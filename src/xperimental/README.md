# XPERIMENTAL package
We define 2 custom extensions, which can be dynamically linked with Spike (XPERIA and XPERIB)
XPERIA include 1 instruction 
XPERIV include 1 instruction (vect)

Folder structure 
- 2 .h files for XPERIA.h and XPERIV.h
- 2 .h files with implementation of insns
- Makefile (make all and make clean) 
- code which 
  triggers these insns xperia.add and xperiv.add
  include DEFINES
- Makefile for code

output
- xperimental.so

running spike:
```
spike -m0x8000:0x2000,0x28000:0x1000,0x30000:0x1000,0x38000:0x1000,0x6b000:0x1000,0x80000:0x1000,0x88000:0x1000,0x140000:0x10000,0x1700000:0x10000,0x20d0000:0x2000000,0x20000000:0x20000000,0x40000000:0x20000000,0x7f000000:0x1000000000 --isa rv64imafdcvh_zba_zbb_zbs_zcb_zcmop_zicond_zkr_zfa_zfbfmin_zfh_zkt_zicbop_zicbom_zicboz_zicfiss_zicfilp_zimop_zawrs_zifencei_zicsr_zihintpause_zihintntl_zicntr_zihpm_zvl1024b_zvfh_zvfbfmin_zvfbfwma_zvbb_zvkt_smcsrind_sscsrind_smrnmi_xperia_xperiv --extlib=xperimental.so -d rvv-vcix.elf
```

PRs in Spike:
- 

