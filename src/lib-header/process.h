// Had to take a source: https://course.ccs.neu.edu/cs3650/unix-xv6/HTML/

#ifndef _PROCESS_H
#define _PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "cpu.h"
#include "fat32.h"
#include "paging.h"

#define EFLAGS_BASE         0x2
#define EFLAGS_PARITY       0x4
#define EFLAGS_INTERRUPT    0x200
#define EFLAGS_USER_PROC    EFLAGS_BASE | EFLAGS_PARITY | EFLAGS_INTERRUPT

#define MAX_PROCESS 64
#define MAX_PROCESS_NAME 8
#define MAX_PROCESS_FRAMES 4

enum ProcState { NULL_PROCESS, NEW, READY, RUNNING, WAITING, TERMINATED };

/**
 * Context, Saved registers on the stack.
 * Other registers are handled by the CPU or implicitly in the asm code
 */
struct Context {
    uint32_t edi, esi, ebx, ebp, eip;
} __attribute__((packed));

/**
 * PCB, Process Control Block. Used to save information about a process
 * 
 * @param pid               process id
 * @param cr3               pointer to physical address of page table
 * @param k_stack           kernel stack address, integer type to match esp0 in tss
 * @param context           address of context to switch to
 * 
 * Process management purposes
 * @param name              process name
 * @param frame_amount      amount of frames used by the process
 * @param virt_addr_used    array of virtual addresses mapped in the page table
 * 
 * @param state             state of the current process
 * @param parent            pointer to the parent process
 * 
 * Process scheduling purposes, linked list structure
 * @param previous_pid      pid of the previously executed task
 * @param next_pid          pid of the to be executed task next
 */
struct PCB
{
    uint32_t pid;                           // id
    struct PageDirectory* cr3;              // virtual address space
    uint32_t k_stack;                       // kernel stack address

    // Static context to switch to    
    struct Context context_static;              // Context to switch to
    struct InterruptFrame cpu_state;            // Saved CPU state
    uint32_t useresp;                           // Add useresp since InterruptStack doesn't include it
    uint32_t userss;                            // Add userss since InterruptStack doesn't include it

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

// To pass to shell during ps
/**
 * process_info, basically PCB with sensitive information removed
 * 
 * @param pid               process id
 * @param ppid              pointer to physical address of page table
 * @param frame_amount      amount of frames used by the process
 * @param state             state of the current process
 * @param name              process name
 */
struct process_info
{
    uint32_t pid;                   // id
    uint32_t ppid;                  // parent id    
    uint32_t frame_amount;       // Amount of resources used
    enum ProcState state;           // state
    char name[MAX_PROCESS_NAME];
} __attribute__((packed));

/**
 * process_list, array of process_info to be passed during ps
 * 
 * @param info              process_info array
 * @param num_process       number of process shown
 */
struct process_list
{
    struct process_info info[MAX_PROCESS];
    uint32_t num_process;
} __attribute__((packed));

/**
 *  External assembly code for context switching,
 *  
 *  @param new_process             pointer to context of process to switch to
 */
void switch_context(struct InterruptFrame* new_process);

/**
 *  External assembly code for context switching, basically retrieves context and system state from the stack
 *  Only used in process creation, not used in general context switching because it is already done by default during interrupt exit
 */
void restore_context();

/**
 *  Initializes multi-processing, assigns kernel as task 0
 */
void process_initialize();

/**
 *  process_generate_pid,
 *  Generates pid for the process, algorithm may vary
 *  
 *  @return             the generated pid
 */
uint32_t process_generate_pid();

/**
 *  process_create,
 *  Create a user process to jump to
 *  
 *  @param request      executable file information
 *  @return             success state of process creation with true being process successfully created
 */
uint8_t process_create_user_proc(struct FAT32DriverRequest request);

/**
 *  Terminates current process
 */
void process_terminate_current();

/**
 *  process_terminate,
 *  Terminate a currently running process. May or may not include the cleaning
 *  
 *  @param pid process identifier
 */
void process_terminate(uint32_t pid);

/**
 *  Garbage collecting scan, detects a terminated task and executes process_clean
 */
void process_clean_scan();

/**
 *  Garbage collecting function, deallocates resources used by a terminated process
 * 
 *  @param pid          pid of a terminated process
 */
void process_clean(uint32_t pid);

/**
 *  Contains the main Scheduling algorithm
 *  Also hooked as the callback to PIT IRQ
 */
void process_schedule(struct InterruptFrame* iframe);

// PS purposes
/**
 *  Generates info from a PCB to a process_info
 * 
 *  @param tinfo        address to process_info to be generated
 *  @param process      PCB of process
 */
void process_get_info(struct process_info* tinfo, struct PCB process);

/**
 *  Generates list of info from currently active tasks
 * 
 *  @param list         address to process_list to be generated
 */
void process_generate_list(struct process_list* list);

#endif