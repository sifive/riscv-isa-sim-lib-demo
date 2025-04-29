#include <riscv_vector.h>

#define SCR_BASE 0x3fffb000

unsigned volatile * const p_finisher = (unsigned *) (SCR_BASE + 8);

// static inline unsigned readCycle() {
//   unsigned res;
//   __asm__ __volatile__("rd" "cycle" " %0" : "=r"(res));
//   return res;
// }

static inline unsigned cpu_get_current_hartid() {
  unsigned mhartid;
  __asm__ volatile("csrr %0, mhartid" : "=r"(mhartid));
  return mhartid;
}

__attribute__((noinline)) void task_load_add () {
  const int * start = (const int *) 0x40000000; // core local data
  const int length = 30;
  int len = length;
  int * src = start;

  vint32m2_t sum;
  while (len > 0) {
    size_t vl = __riscv_vsetvl_e32m2(len);
    vint32m2_t a = __riscv_vle32_v_i32m2(src, vl);
    sum = __riscv_vadd(a, sum, vl);
    len -= vl;
    src += vl;
  }

  size_t vl1 = __riscv_vsetvl_e32m2(length);
  vint32m1_t final_sum;
  final_sum = __riscv_vredsum_vs_i32m2_i32m1(sum, final_sum, vl1);

  size_t vl2 = __riscv_vsetvl_e32m1(1);
  __riscv_vse32_v_i32m1(start, final_sum, vl2);
}

__attribute__((noinline)) void task_wfi () {
  __asm__ __volatile__("wfi;");
}

int main () {
  switch(cpu_get_current_hartid()) {
  case 0:
    task_load_add();
    break;
  default:
    task_wfi();
  }

  __asm__ __volatile__("fence;");
  *p_finisher = 0x5555;
  while (1) {
    __asm__("nop; nop;");
  }
  return 0;
}
