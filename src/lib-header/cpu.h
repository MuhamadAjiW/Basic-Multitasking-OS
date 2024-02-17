
#ifndef _CPU_H
#define _CPU_H

#include "stdtype.h"

/**
 * CPURegister, store CPU registers that can be used for interrupt handler / ISRs
 * 
 * @param gp_register    CPU general purpose register (a, b, c, d + esi & edi)
 * @param stack_register CPU stack register (bp, sp)
 */
struct CPURegister {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} __attribute__((packed));

/**
 * CPURegister, store CPU registers that can be used for interrupt handler / ISRs
 * 
 * @param segment        CPU Data Segment registers (g, f, e, d)
 */
struct CPUSegments {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
} __attribute__((packed));

// TODO: Document
struct TrapFrame{
    struct CPUSegments segments;
    struct CPURegister registers;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, userss;
} __attribute__((packed));

#endif
