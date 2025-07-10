    .text
    .balign 4
    .global _start
_start:
    /* Setup the global pointer, which the ABI assumes points to the
     * __global_pointer$ symbol. */
    .option push
    .option norelax
    .option pop

    li   sp, 0x80100000 // __stack_end

    // #define METAL_MSTATUS_FS_INIT 0x00002000UL
    // #define METAL_MSTATUS_FS_CLEAN 0x00004000UL
    // #define METAL_MSTATUS_FS_DIRTY 0x00006000UL
    // #define METAL_MSTATUS_VS 0x00000600UL

    li   t0, 0x2600
    csrs mstatus, t0

    call main

1:
    j 1b
