//work in progress (obviously)

#ifndef _TASK_H
#define _TASK_H

#include "stdtype.h"
#include "cpu.h"

#define PSTATE_NEW 0
#define PSTATE_READY 1
#define PSTATE_RUNNING 2
#define PSTATE_WAITING 3
#define PSTATE_TERMINATED 4

#define MAX_TASKS 64
#define MAX_TASKS_PNAME 32

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
    uint8_t pstate;                // process state
    uint8_t pid;                   // id
    uint8_t parent_pid;            // parent id
    uint8_t eip;                   // program counter
    
    // context
    Context context;
    uint32_t cr3;                   // virtual address space

    // Extras
    struct PCB* next;               // next process for pipelining
    char name[MAX_TASKS_PNAME];
    

} __attribute__((packed)) PCB;

void switch_context(PCB* old_task, PCB* new_task);
void restore_context();
void isr_exit();

void initialize_tasking();
void create_task(uint32_t pid, uint32_t eip, uint32_t u_stack, uint8_t STACKTYPE, uint32_t eflags);
void schedule();

#endif