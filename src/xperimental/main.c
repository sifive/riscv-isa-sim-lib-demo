#include <riscv_vector.h>
// #include <stdio.h>
#include <stdint.h>


#define OPCODE_S1_SHIFT (7+4+4+5+4+4)
#define OPCODE_K_SHIFT (7+4+4+5+4)
#define OPCODE_S2_SHIFT (7+4+4+5)
#define OPCODE_OP_SHIFT (7+4)


#define FINISHER_BASE 0x3fffb000

// scalar add
#define PERIAADD(s1, s2)   __asm   (".word(((" _XSTR(s1) ") << " _XSTR(OPCODE_S1_SHIFT) ") | (15<<" _XSTR(OPCODE_K_SHIFT) ") | ((" _XSTR(s2) ") <<" _XSTR(OPCODE_S2_SHIFT) ") | (15  <<" _XSTR(OPCODE_OP_SHIFT) ") | | (0x0b))")
#define ADD_KNOP_p(ds, s1, s2, rs1)   asm   (".word(((" _XSTR(s1) ") << " _XSTR(OPCODE_S1_SHIFT) ") | ((" _XSTR(s2) ") <<" _XSTR(OPCODE_S2_SHIFT) ") | (((((\"%0\"==\"t0\") &5) |((\"%0\"==\"t1\") &6) |((\"%0\"==\"t2\") &7) |((\"%0\"==\"s0\") &8) |((\"%0\"==\"s1\") &9) |((\"%0\"==\"a0\") &10)|((\"%0\"==\"a1\") &11)|((\"%0\"==\"a2\") &12)|((\"%0\"==\"a3\") &13)|((\"%0\"==\"a4\") &14)|((\"%0\"==\"a5\") &15)|((\"%0\"==\"a6\") &16)|((\"%0\"==\"a7\") &17)|((\"%0\"==\"s2\") &18)|((\"%0\"==\"s3\") &19)|((\"%0\"==\"s4\") &20)|((\"%0\"==\"s5\") &21)|((\"%0\"==\"s6\") &22)|((\"%0\"==\"s7\") &23)|((\"%0\"==\"s8\") &24)|((\"%0\"==\"s9\") &25)|((\"%0\"==\"s10\")&26)|((\"%0\"==\"s11\")&27)|((\"%0\"==\"t3\") &28)|((\"%0\"==\"t4\") &29)|((\"%0\"==\"t5\") &30)|((\"%0\"==\"t6\") &31))) << " _XSTR(OPCODE_RS1_SHIFT) ") | (1  <<" _XSTR(OPCODE_OP_SHIFT) ") | ((" _XSTR(ds) ") <<" _XSTR(OPCODE_DS_SHIFT) ") | (0x0b))" : : "r" (rs1))
unsigned volatile * const p_finisher = (unsigned *) (FINISHER_BASE + 8);

// custom vector add
#define PERIVADD(s1, s2)   


// Triggering XPERIA and XPERIV
int main () { 
    
PERIAADD()
    
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
\
  li      a1, 4;\
\
  li      t6, 0x4;\
  .word   0x000107b3; /* dummy instruction for alignment if needed */\
  sf.vc.x 0x0, 0x1, 0x3, t6;\
  sf.vc.i 0x0, 0x1, 0x3, 0x4;\
\
  sf.vc.xvv 0x3, v0, v2, a1;\
  sf.vc.v.x 0x0, 0x1, v2, t6;\
\
  sf.vc.ivv 0x3, v0, v2, 15;\
  sf.vc.v.i 0x3, 0xf, v0, 15;\
\
  sf.vc.v.vv 0x0, v0, v2, v0;\
\
  li      t4, 0x80002000;\
  li      t5, 4;\
  vse32.v v16, (t4);\
");

  *p_finisher = 0x5555;

  while(1) {
      __asm__("nop; nop;");
  };
  return 0;
}
