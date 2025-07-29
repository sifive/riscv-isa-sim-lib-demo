#include <riscv_vector.h>
// #include <stdio.h>
#include <stdint.h>

#define OPCODE_DS_SHIFT_VAL 7
#define OPCODE_RS1_SHIFT_VAL 15
#define OPCODE_RS2_SHIFT_VAL 20

#define CUSTOM0 0x0b
#define CUSTOM1 0x2b

#define FINISHER_BASE 0x3fffb000

// scalar add
#define _XSTR(x) #x
#define PERIAADD(rd, rs1, rs2) __asm__ volatile (".word ((" _XSTR(rs1) " << " _XSTR(15) ") | (" _XSTR(rs2) " << " _XSTR(20) ") | (" _XSTR(rd) " << " _XSTR(7) ") | 0x0b)" : : : "memory")
unsigned volatile * const p_finisher = (unsigned *) (FINISHER_BASE + 8);

#define PERIVADD(vd, vs1, vs2) __asm__ volatile (".word ((" _XSTR(vs1) " << " _XSTR(15) ") | (" _XSTR(vs2) " << " _XSTR(20) ") | (" _XSTR(vd) " << " _XSTR(7) ") | 0x2b)" : : : "memory")



// #define PERIAADD(rd, rs1, rs2) __asm__ volatile (".word ((" _XSTR(rs1) " << " _XSTR(OPCODE_RS1_SHIFT_VAL) ") | (" _XSTR(rs2) " << " _XSTR(OPCODE_RS2_SHIFT_VAL) ") | (" _XSTR(rd) " << " _XSTR(OPCODE_DS_SHIFT_VAL) ") | CUSTOM0)" : : : "memory")
// unsigned volatile * const p_finisher = (unsigned *) (FINISHER_BASE + 8);

// #define PERIVADD(vd, vs1, vs2) __asm__ volatile (".word ((" _XSTR(vs1) " << " _XSTR(OPCODE_RS1_SHIFT_VAL) ") | (" _XSTR(vs2) " << " _XSTR(OPCODE_RS2_SHIFT_VAL) ") | (" _XSTR(vd) " << " _XSTR(OPCODE_DS_SHIFT_VAL) ") | CUSTOM1)" : : : "memory")



// Triggering XPERIA and XPERIV
int main () { 
    
    PERIAADD(10, 11, 12);  // x10 = x11 + x12
    
    __asm__("\
      li      t0, 0x600;\
      csrs    mstatus, t0;\
    \
      li      t0, 1024;\
      vsetvli t0, t0, e32, m2, tu, mu;\
    \
      li      t0, 0x80001000;\
      vle32.v v0, (t0);\
    \
      li      t2, 0x80001100;\
      vle32.v v2, (t2);\
    ");
    
    PERIVADD(4, 0, 2);     // v4 = v0 + v2
    
    *p_finisher = 0x5555;
    while(1) {
        __asm__("nop; nop;");
    };
    return 0;
}
