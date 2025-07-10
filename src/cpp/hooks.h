#ifndef HOOKS_H
#define HOOKS_H


#define MMU_OBSERVE_LOAD(addr, data, length) \
printf("addr = %lx, data = %lx, length = %lx\n", addr, data, length);


#define MMU_OBSERVE_FETCH(addr, insn, length) \
printf("addr = %lx, insn = %lx, length = %lx\n", addr, insn, length);

#endif