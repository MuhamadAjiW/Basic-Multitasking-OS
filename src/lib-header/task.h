//work in progress (obviously)

#ifndef _TASK_H
#define _TASK_H

#include "stdtype.h"
#include "cpu.h"
#include "fat32.h"

#define EFLAGS_BASE         0x2
#define EFLAGS_PARITY       0x4
#define EFLAGS_INTERRUPT    0x200

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
    uint32_t int_no, err_code;

    uint32_t eip, cs, eflags, useresp, userss;
} __attribute__((packed)) Context;

//TODO: Document
typedef struct ContextReturn {
      uint32_t edi, esi, ebx, ebp, ret_eip;
} __attribute__((packed)) ContextReturn;

typedef struct PCB
{
    // context needs to be first for context switching
    uint32_t eip;                   // program counter
    uint32_t esp;                   // program counter
    Context context;
    uint32_t cr3;                   // virtual address space

    uint8_t pstate;                // process state
    uint8_t pid;                   // id
    uint8_t parent_pid;            // parent id
    uint8_t privilege;             // kernel or user level privilege

    // Extras
    struct PCB* next;               // next process for pipelining
    char name[MAX_TASKS_PNAME];
    

} __attribute__((packed)) PCB;

void switch_context(PCB* old_task, PCB* new_task);
void restore_context();
void isr_exit();

void initialize_tasking();

// returns 0 if failed, 1 if successful
uint8_t create_task(FAT32DriverRequest request, uint32_t pid, uint8_t stack_type, uint32_t eflags);
void schedule();

#endif