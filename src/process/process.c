#include "../lib-header/process.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/gdt.h"
#include "../lib-header/tss.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/resource.h"
#include "../lib-header/paging.h"
#include "../lib-header/fat32.h"

extern struct TSSEntry tss;

// TODO: Review

// Note: Would be interesting to make process with dynamic
// Process is managed like a linked list for performance reasons with indices as pointers because static memory (check PCB structure)
// Algorithm for processing is round robin
// Most of this gets slow because of static memory, very recommended to create a dedicated kernel heap for processing
// Task 0 is always the kernel
struct PCB process_array[MAX_PROCESS] = {0};
uint8_t process_active[MAX_PROCESS] = {0};
struct PageDirectory process_page_dir[MAX_PROCESS] = {0};
uint32_t num_process = 0;
struct PCB* current_process;
uint32_t last_process_pid;

void process_initialize(){
    current_process = &process_array[0];
    current_process->pid = 0;
    current_process->k_stack = KERNEL_VMEMORY_OFFSET + PAGE_FRAME_SIZE;
    
    current_process->name[0] = 'k';
    current_process->name[1] = 'e';
    current_process->name[2] = 'r';
    current_process->name[3] = 'n';
    current_process->name[4] = 'e';
    current_process->name[5] = 'l';
    current_process->name[6] = 0;

    current_process->cr3 = (struct PageDirectory*)((uint32_t) &process_page_dir[0] - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);
    current_process->state = RUNNING;
    current_process->parent = current_process;

    current_process->resource_amount = KERNEL_PAGE_COUNT;
    current_process->previous_pid = 0;
    current_process->next_pid = 0;

    paging_dirtable_init(&process_page_dir[0]);
    
    process_active[0] = 1;
    num_process = 1;
    last_process_pid = 0;
}

void process_get_info(struct process_info* tinfo, struct PCB process){
    tinfo->name[0] = process.name[0];
    tinfo->name[1] = process.name[1];
    tinfo->name[2] = process.name[2];
    tinfo->name[3] = process.name[3];
    tinfo->name[4] = process.name[4];
    tinfo->name[5] = process.name[5];
    tinfo->name[6] = process.name[6];
    tinfo->name[7] = process.name[7];
    tinfo->resource_amount = process.resource_amount;
    tinfo->pid = process.pid;
    tinfo->ppid = process.parent->pid;
    tinfo->state = process.state;
}

void process_generate_list(struct process_list* list){
    uint32_t idx = 0;
    uint32_t next_pid = 0;

    // kernel is always process 0
    process_get_info(&(list->info[idx]), process_array[0]);
    
    idx++;

    __asm__ volatile ("cli");   // Stop interrupts for these parts below
    list->num_process = num_process;

    next_pid = process_array[0].next_pid;
    while (next_pid != 0){
        process_get_info(&(list->info[idx]), process_array[next_pid]);
        idx++;

        next_pid = process_array[next_pid].next_pid;
    }
    
    __asm__ volatile ("sti");   // reenable interrupts
}

uint32_t process_generate_pid(){
    uint32_t i = 1;
    while (process_active[i]){
        i++;
    }

    return i;
}

uint8_t process_create(struct FAT32DriverRequest request, uint8_t stack_type, uint32_t eflags){
    __asm__ volatile ("cli");   // Stop interrupts when creating a process

    if(num_process == MAX_PROCESS) {
        __asm__ volatile ("sti");   // reenable interrupts
        return 0;
    }

    // TODO: optimize, using a whole page for stack is really excessive
    // Allocate resources with ceiling division with at least 1MB of user stack and always 1 extra page for kernel stack
    uint32_t resource_amount = ((0x100000 + request.buffer_size + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE) + 1;
    
    // Check resource availability
    if (!resource_check(resource_amount)){
        __asm__ volatile ("sti");   // reenable interrupts
        return 0;
    }
    
    // Initialize process base data
    uint32_t pid = process_generate_pid();
    process_array[pid].pid = pid;
    process_array[pid].parent = current_process;
    process_array[pid].state = NEW;
    process_array[pid].resource_amount = resource_amount;

    // Initialize with kernel's paging directory
    struct PageDirectory* page_dir = &process_page_dir[pid];

    paging_dirtable_init(page_dir);

    // Also copy the current kernel stack in case the caller is not the kernel
    paging_clone_kernel_stack(*current_process, process_array[pid]);

    uint32_t u_stack = resource_allocate(resource_amount - 1, pid, page_dir);
    uint32_t k_stack = resource_allocate_kernel(pid, page_dir);

    // Initialize process paging data
    process_array[pid].cr3 = (struct PageDirectory*) ((uint32_t) page_dir - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);

    for (uint8_t i = 0; i < 8; i++){
        process_array[pid].name[i] = request.name[i];
    }
    
    // Assign process as the last entry on the linked list
    process_array[pid].previous_pid = last_process_pid;
    process_array[pid].next_pid = 0;
    process_array[0].previous_pid = pid;
    process_array[last_process_pid].next_pid = pid;
    last_process_pid = pid;
    process_active[pid] = 1;

    // Prepare the entry for new process
    paging_use_page_dir(process_array[pid].cr3);

    // Flush user stack pages
    paging_flush_tlb_range((void*) 0, (void*) ((resource_amount - 1) * PAGE_FRAME_SIZE));
    // Flush kernel stack pages
    paging_flush_tlb_single((void*)k_stack);

    // Load file into memory at 0
    request.buf = (void*) 0;
    load(request);

    // Set TrapFrame to be at the bottom of the given kernel stack
    uint8_t* k_esp = (uint8_t*) k_stack;
    k_esp -= sizeof(struct TrapFrame);
    struct TrapFrame* tf = (struct TrapFrame*) k_esp;
    memset(tf, 0, sizeof(struct TrapFrame));

    // Prepare new process environment
    uint32_t cs = stack_type == STACKTYPE_KERNEL ? GDT_KERNEL_CODE_SEGMENT_SELECTOR : (GDT_USER_CODE_SEGMENT_SELECTOR | PRIVILEGE_USER);
    uint32_t ds = stack_type == STACKTYPE_KERNEL ? GDT_KERNEL_DATA_SEGMENT_SELECTOR : (GDT_USER_DATA_SEGMENT_SELECTOR | PRIVILEGE_USER);

    tf->cs = cs;
    tf->segments.ds = ds;

    tf->userss = ds;
    tf->useresp = u_stack;
    tf->eflags = eflags;

    // Note: entry is assumed to be always set at 0 when linking a program
    tf->eip = (uint32_t) request.buf;

    k_esp -= sizeof(struct Context);
    struct Context* context = (struct Context*) k_esp;
    context->edi = 0;
    context->esi = 0;
    context->ebx = 0;
    context->ebp = 0;
    context->eip = (uint32_t) restore_context;


    // Save the data on our process list
    process_array[pid].k_stack = k_stack;
    process_array[pid].context = context;
    process_array[pid].tf = tf;
    process_array[pid].state = READY;

    paging_use_page_dir(current_process->cr3);

    // Flush user stack pages
    paging_flush_tlb_range((void*) 0, (void*) ((resource_amount - 1) * PAGE_FRAME_SIZE));
    // Flush kernel stack pages
    paging_flush_tlb_single((void*)k_stack);    

    num_process++;

    __asm__ volatile ("sti");   // reenable interrupts
    return 1;
}

void process_terminate_current(){
    current_process->state = TERMINATED;
}

void process_terminate(uint32_t pid){
    if(process_array[pid].state != NULL_PROCESS){
        process_array[pid].state = TERMINATED;  // Only marking it as terminated, cleaning up is done later on the background
    }
}

// Naive garbage collector implementation
// Basically looping endlessly looking for terminated process and cleaning it up
void process_clean_scan(){
    for (uint32_t i = 0; i < MAX_PROCESS; i++){
        if(process_array[i].state == TERMINATED){
            process_clean(i);
        }
    }
}

// Cleaning up one process
void process_clean(uint32_t pid){
    __asm__ volatile ("cli");   // Stop interrupts
    
    resource_deallocate(pid, process_array[pid].resource_amount);
    winmgr_clean_window(pid);

    struct PCB process = process_array[pid];

    process_array[process.next_pid].previous_pid = process_array[pid].previous_pid;
    process_array[process.previous_pid].next_pid = process_array[pid].next_pid;
    process_array[pid].state = NULL_PROCESS;
    process_active[pid] = 0;

    if(pid == last_process_pid){
        last_process_pid = process.previous_pid;
    }

    num_process--;

    __asm__ volatile ("sti");   // reenable interrupts
}


void process_schedule(){
    int next_pid = current_process->next_pid;

    while (process_array[next_pid].state == TERMINATED){
        next_pid = process_array[next_pid].next_pid;
    }    

    struct PCB* new = &process_array[next_pid];
    struct PCB* old = current_process;
    if(new == old) return; // Will break if not switching due to asm code

    current_process = new;

    tss.esp0 = new->k_stack;

    if(old->state == RUNNING) old->state = READY;
    new->state = RUNNING;

    
    // We need to get old process's kernel stack before switching page tables
    // Doesn't need to clear it when a process is done
    // It doesn't matter since we are copying and reloading them each time
    paging_clone_kernel_stack(*old, *new);
    
    // Switching page tables
    paging_use_page_dir(new->cr3);

    // flush pages
    paging_flush_tlb_single((void*) old->k_stack);
    paging_flush_tlb_range((void*) 0, (void*) (new->resource_amount * PAGE_FRAME_SIZE));

    switch_context(&(old->context), new->context);
}
