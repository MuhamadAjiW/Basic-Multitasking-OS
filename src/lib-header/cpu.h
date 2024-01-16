#ifndef _CPU_H
#define _CPU_H

#include "stdtype.h"

/**
 * CPURegister, store CPU registers that can be used for interrupt handler / ISRs
 * 
 * @param gp_register    CPU general purpose register (a, b, c, d + esi & edi)
 * @param stack_register CPU stack register (bp, sp)
 */
typedef struct CPURegister {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} __attribute__((packed)) CPURegister;

/**
 * CPURegister, store CPU registers that can be used for interrupt handler / ISRs
 * 
 * @param segment        CPU Data Segment registers (g, f, e, d)
 */
typedef struct CPUSegments {
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
} __attribute__((packed)) CPUSegments;

#endif
