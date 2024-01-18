
#ifndef _TSS_H
#define _TSS_H

#include "stdtype.h"

/**
 * TSSEntry, Task State Segment. Used when jumping back to ring 0 / kernel
 */
typedef struct {
    uint32_t prev_tss; // Previous TSS
    uint32_t esp0;     // Stack pointer to load when changing to kernel mode
    uint32_t ss0;      // Stack segment to load when changing to kernel mode
    
    uint32_t esp1;
    uint32_t ss1;
    
    uint32_t esp2;
    uint32_t ss2;

    uint32_t cr3;

    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint32_t ldtr;
    uint32_t debug;
} __attribute__((packed)) TSSEntry;

// Set kernel stack in TSS
void set_tss_kernel_current_stack(void);

#endif