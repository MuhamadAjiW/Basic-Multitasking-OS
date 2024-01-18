//work in progress (obviously)

// Note: Such a bloody nightmare creating task and context switching
// Had to take a source: https://course.ccs.neu.edu/cs3650/unix-xv6/HTML/

#ifndef _TASK_H
#define _TASK_H

#include "stdtype.h"
#include "cpu.h"
#include "paging.h"
#include "fat32.h"

#define EFLAGS_BASE         0x2
#define EFLAGS_PARITY       0x4
#define EFLAGS_INTERRUPT    0x200


#define MAX_TASKS 64
#define MAX_TASKS_PNAME 16

#define STACKTYPE_KERNEL 0
#define STACKTYPE_USER 3

//TODO: Virtual memory clearance to own page table, tss and gdt reset not implemented yet
enum ProcState { NEW, READY, RUNNING, WAITING, TERMINATED };

//TODO: Document
typedef struct Context {
    uint32_t edi, esi, ebx, ebp, eip;
} __attribute__((packed)) Context;

typedef struct PCB
{
    uint32_t size;                  // process size
    PageDirectory* cr3;             // virtual address space
    uint32_t k_stack;               // kernel stack address
    enum ProcState state;           // state
    uint32_t pid;                   // id
    struct PCB* parent;             // parent id
    TrapFrame* tf;                  // TrapFrame for current syscall
    Context* context;               // Context to switch to
    uint32_t resource_amount;       // Amount of resources used
    char name[MAX_TASKS_PNAME];

} PCB;
// Not packed because of alignment

void switch_context(Context** old_task, Context* new_task);
void restore_context();
void isr_exit();

void task_initialize();

// returns 0 if failed, 1 if successful
uint8_t task_create(FAT32DriverRequest request, uint32_t pid, uint8_t stack_type, uint32_t eflags);
void task_schedule();

#endif