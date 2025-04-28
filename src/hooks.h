#ifndef HOOKS_H
#define HOOKS_H


#define MMU_OBSERVE_LOAD(addr, data, length) \
printf("addr = %lx, data = %lx, length = %lx\n", addr, data, length);



#endif