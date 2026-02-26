#include "layout.h"

.section .text, "ax"
.global  _start
.type    _start, function
.align 4

/*
    cpu0 => [sp_stack, sp_stack+4KB)
    cpu1 => [sp_stack+4KB, sp_stack+8KB)
    cpu2 => [sp_stack+8KB, sp_stack+12KB)
    cpu3 => [sp_stack+12KB, sp_stack+16KB)
*/

_start:
    /* Set stack for c code */
    adrp    x0, sp_stack
    /* Get Current code id */
    mrs     x1, mpidr_el1
    and     x1, x1, #0x0f
    add     x2, x1, 1
    mov     x3, #SZ_4K
    mul     x3, x3, x2
    add     x0, x0, x3
    mov     sp, x0
    cbz     x1, hyper_init_primary
    bl      hyper_init_secondary
    /* spin here */
    b       .

.global  smc_call
.type    smc_call, function
smc_call:
    smc #0
    ret
