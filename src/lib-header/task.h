//work in progress (obviously)

#ifndef _TASK_H
#define _TASK_H

#include "stdtype.h"
#include "cpu.h"

#define PSTATE_ALIVE 0
#define PSTATE_DONE 1
#define PSTATE_DEAD 2

// TODO: Make dynamic
#define MAX_TASKS 64


#define STACKTYPE_KERNEL 0
#define STACKTYPE_USER 3


//TODO: Virtual memory clearance to own page table, tss and gdt reset not implemented yet
typedef struct Context{
    CPUSegments segments;
    CPURegister registers;
    uint32_t eip, cs, eflags, useresp, userss;
} __attribute__((packed)) Context;

//TODO: Document
typedef struct ContextReturn {
      uint32_t edi, esi, ebx, ebp, ret_eip;
} __attribute__((packed)) ContextReturn;

typedef struct PCB
{
    uint32_t pid; //id
    
    // context
    Context context;

    // other registers
    uint32_t cr3; //virtual address space

    // Pipelining purposes
    struct PCB* next; //next process
    
    // Extras
    uint32_t state;

} __attribute__((packed)) PCB;

void switch_context(PCB* old_task, PCB* new_task);
void restore_context();
void isr_exit();

void initialize_tasking();
void create_task(uint32_t pid, uint32_t eip, uint32_t u_stack, uint8_t STACKTYPE);
void schedule();

#endif