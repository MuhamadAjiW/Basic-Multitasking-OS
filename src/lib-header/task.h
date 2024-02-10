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
#define MAX_TASKS_PNAME 8

#define STACKTYPE_KERNEL 0
#define STACKTYPE_USER 3

enum ProcState { NULL_TASK, NEW, READY, RUNNING, WAITING, TERMINATED };

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

    TrapFrame* tf;                  // TrapFrame for syscalls
    Context* context;               // Context to switch to
    
    // Extras for process management purposes
    uint32_t resource_amount;       // Amount of resources used
    char name[MAX_TASKS_PNAME];
    
    // Linked list purposes
    uint32_t previous_pid;
    uint32_t next_pid;

} PCB;

// To pass to shell
typedef struct task_info
{
    uint32_t pid;                   // id
    uint32_t ppid;                  // parent id    
    uint32_t resource_amount;       // Amount of resources used
    enum ProcState state;           // state
    char name[MAX_TASKS_PNAME];

} task_info;

typedef struct task_list
{
    task_info info[MAX_TASKS];
    uint32_t num_task;

} task_list;

// Not packed because of alignment

void switch_context(Context** old_task, Context* new_task);
void restore_context();
void isr_exit();

void task_initialize();
void task_get_info(task_info* tinfo, PCB task);
void task_generate_list(task_list* list);
uint32_t task_generate_pid();
uint8_t task_create(FAT32DriverRequest request, uint8_t stack_type, uint32_t eflags, char** args);
void task_terminate_current();
void task_terminate(uint32_t pid);
void task_clean_scan();
void task_clean(uint32_t pid);
void task_schedule();

#endif