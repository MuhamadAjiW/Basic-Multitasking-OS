// Had to take a source: https://course.ccs.neu.edu/cs3650/unix-xv6/HTML/

#ifndef _PROCESS_H
#define _PROCESS_H

#include "stdtype.h"
#include "cpu.h"
#include "fat32.h"
#include "paging.h"

#define EFLAGS_BASE         0x2
#define EFLAGS_PARITY       0x4
#define EFLAGS_INTERRUPT    0x200

#define MAX_PROCESS 64
#define MAX_PROCESS_NAME 8
#define MAX_PROCESS_FRAMES 4

#define STACKTYPE_KERNEL 0
#define STACKTYPE_USER 3

enum ProcState { NULL_PROCESS, NEW, READY, RUNNING, WAITING, TERMINATED };

//TODO: Document
struct Context {
    uint32_t edi, esi, ebx, ebp, eip;
} __attribute__((packed));

struct PCB
{
    uint32_t pid;                           // id
    struct PageDirectory* cr3;              // virtual address space
    uint32_t k_stack;                       // kernel stack address
    struct Context* context;                // Context to switch to
    
    // Extras for process management purposes
    char name[MAX_PROCESS_NAME];
    uint32_t frame_amount;                  // Amount of frames used
    void* virt_addr_used[MAX_PROCESS_FRAMES];
    
    enum ProcState state;                   // state
    struct PCB* parent;                     // parent
    
    // Linked list purposes
    uint32_t previous_pid;
    uint32_t next_pid;
}; // Not packed because of alignment

// To pass to shell
struct process_info
{
    uint32_t pid;                   // id
    uint32_t ppid;                  // parent id    
    uint32_t frame_amount;       // Amount of resources used
    enum ProcState state;           // state
    char name[MAX_PROCESS_NAME];
} __attribute__((packed));

struct process_list
{
    struct process_info info[MAX_PROCESS];
    uint32_t num_process;
} __attribute__((packed));

void switch_context(struct Context** old_process, struct Context* new_process);
void restore_context();

void process_initialize();
void process_get_info(struct process_info* tinfo, struct PCB process);
void process_generate_list(struct process_list* list);
uint32_t process_generate_pid();
uint8_t process_create(struct FAT32DriverRequest request, uint8_t stack_type, uint32_t eflags);
void process_terminate_current();
void process_terminate(uint32_t pid);
void process_clean_scan();
void process_clean(uint32_t pid);
void process_schedule();

#endif