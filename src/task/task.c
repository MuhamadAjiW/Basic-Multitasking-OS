#include "../lib-header/task.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/gdt.h"
#include "../lib-header/tss.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/resource.h"
#include "../lib-header/paging.h"
#include "../lib-header/fat32.h"

extern TSSEntry tss;

// TODO: Review

// Note: Would be interesting to make tasks with dynamic
// Tasks is managed like a linked list for performance reasons with indices as pointers because static memory (check PCB structure)
// Algorithm for tasking is round robin
// Most of this gets slow because of static memory, very recommended to create a dedicated kernel heap for tasking
// Task 0 is always the kernel
PCB tasks[MAX_TASKS] = {0};
uint8_t tasks_active[MAX_TASKS] = {0};
PageDirectory tasks_page_dir[MAX_TASKS] = {0};
uint32_t num_task = 0;
PCB* current_task;
uint32_t last_task_pid;

void task_initialize(){
    current_task = &tasks[0];
    current_task->pid = 0;
    current_task->k_stack = KERNEL_VMEMORY_OFFSET + PAGE_FRAME_SIZE;
    
    current_task->name[0] = 'k';
    current_task->name[1] = 'e';
    current_task->name[2] = 'r';
    current_task->name[3] = 'n';
    current_task->name[4] = 'e';
    current_task->name[5] = 'l';
    current_task->name[6] = 0;

    current_task->cr3 = (PageDirectory*)((uint32_t) &tasks_page_dir[0] - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);
    current_task->state = RUNNING;
    current_task->parent = current_task;

    current_task->resource_amount = KERNEL_PAGE_COUNT;
    current_task->previous_pid = 0;
    current_task->next_pid = 0;

    paging_dir_copy(_paging_kernel_page_directory, &tasks_page_dir[0]);
    
    tasks_active[0] = 1;
    num_task = 1;
    last_task_pid = 0;
}

void task_get_info(task_info* tinfo, PCB task){
    tinfo->name[0] = task.name[0];
    tinfo->name[1] = task.name[1];
    tinfo->name[2] = task.name[2];
    tinfo->name[3] = task.name[3];
    tinfo->name[4] = task.name[4];
    tinfo->name[5] = task.name[5];
    tinfo->name[6] = task.name[6];
    tinfo->name[7] = task.name[7];
    tinfo->resource_amount = task.resource_amount;
    tinfo->pid = task.pid;
    tinfo->ppid = task.parent->pid;
    tinfo->state = task.state;
}

void task_generate_list(task_list* list){
    uint32_t idx = 0;
    uint32_t next_pid = 0;

    // kernel is always task 0
    task_get_info(&(list->info[idx]), tasks[0]);
    
    idx++;

    __asm__ volatile ("cli");   // Stop interrupts for these parts below
    list->num_task = num_task;

    next_pid = tasks[0].next_pid;
    while (next_pid != 0){
        task_get_info(&(list->info[idx]), tasks[next_pid]);
        idx++;

        next_pid = tasks[next_pid].next_pid;
    }
    
    __asm__ volatile ("sti");   // reenable interrupts
}

uint32_t task_generate_pid(){
    uint32_t i = 1;
    while (tasks_active[i]){
        i++;
    }

    return i;
}

uint8_t task_create(FAT32DriverRequest request, uint8_t stack_type, uint32_t eflags){
    __asm__ volatile ("cli");   // Stop interrupts when creating a task

    if(num_task == MAX_TASKS) {
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
    
    uint32_t pid = task_generate_pid();

    // Initialize with kernel's paging directory
    PageDirectory* page_dir = &tasks_page_dir[pid];

    paging_dir_copy(_paging_kernel_page_directory, page_dir);

    // Also copy the current kernel stack in case the caller is not the kernel
    paging_dir_copy_single(tasks_page_dir[current_task->pid], page_dir, (void*) current_task->k_stack - PAGE_FRAME_SIZE);

    uint32_t u_stack = resource_allocate(resource_amount - 1, pid, page_dir);
    uint32_t k_stack = resource_allocate_kernel(pid, page_dir);

    // Initialize task
    tasks[pid].pid = pid;
    tasks[pid].parent = current_task;
    tasks[pid].state = NEW;
    tasks[pid].cr3 = (PageDirectory*) ((uint32_t) page_dir - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);
    tasks[pid].resource_amount = resource_amount;

    for (uint8_t i = 0; i < 8; i++){
        tasks[pid].name[i] = request.name[i];
    }
    
    // Assign task as the last entry on the linked list
    tasks[pid].previous_pid = last_task_pid;
    tasks[pid].next_pid = 0;
    tasks[0].previous_pid = pid;
    tasks[last_task_pid].next_pid = pid;
    last_task_pid = pid;
    tasks_active[pid] = 1;

    // Prepare the entry for new task
    paging_use_page_dir(tasks[pid].cr3);

    // Flush user stack pages
    paging_flush_tlb_range((void*) 0, (void*) ((resource_amount - 1) * PAGE_FRAME_SIZE));
    // Flush kernel stack pages
    paging_flush_tlb_single((void*)k_stack);

    // Load file into memory at 0
    request.buf = (void*) 0;
    load(request);

    // Set TrapFrame to be at the bottom of the given kernel stack
    uint8_t* k_esp = (uint8_t*) k_stack;
    k_esp -= sizeof(TrapFrame);
    TrapFrame* tf = (TrapFrame*) k_esp;
    memset(tf, 0, sizeof(TrapFrame));

    // Prepare new task environment
    uint32_t cs = stack_type == STACKTYPE_KERNEL ? GDT_KERNEL_CODE_SEGMENT_SELECTOR : (GDT_USER_CODE_SEGMENT_SELECTOR | PRIVILEGE_USER);
    uint32_t ds = stack_type == STACKTYPE_KERNEL ? GDT_KERNEL_DATA_SEGMENT_SELECTOR : (GDT_USER_DATA_SEGMENT_SELECTOR | PRIVILEGE_USER);

    tf->cs = cs;
    tf->segments.ds = ds;

    tf->userss = ds;
    tf->useresp = u_stack;
    tf->eflags = eflags;

    // Note: entry is assumed to be always set at 0 when linking a program
    tf->eip = (uint32_t) request.buf;

    k_esp -= sizeof(Context);
    Context* context = (Context*) k_esp;
    context->edi = 0;
    context->esi = 0;
    context->ebx = 0;
    context->ebp = 0;
    context->eip = (uint32_t) restore_context;


    // Save the data on our task list
    tasks[pid].k_stack = k_stack;
    tasks[pid].context = context;
    tasks[pid].tf = tf;
    tasks[pid].state = READY;

    paging_use_page_dir(current_task->cr3);

    // Flush user stack pages
    paging_flush_tlb_range((void*) 0, (void*) ((resource_amount - 1) * PAGE_FRAME_SIZE));
    // Flush kernel stack pages
    paging_flush_tlb_single((void*)k_stack);    

    num_task++;

    __asm__ volatile ("sti");   // reenable interrupts
    return 1;
}

void task_terminate_current(){
    current_task->state = TERMINATED;
}

void task_terminate(uint32_t pid){
    tasks[pid].state = TERMINATED;  // Only marking it as terminated, cleaning up is done later on the background
}

// Naive garbage collector implementation
// Basically looping endlessly looking for terminated tasks and cleaning it up
void task_clean_scan(){
    for (uint32_t i = 0; i < MAX_TASKS; i++){
        if(tasks[i].state == TERMINATED){
            task_clean(i);
        }
    }
}

// Cleaning up one task
void task_clean(uint32_t pid){
    __asm__ volatile ("cli");   // Stop interrupts
    
    resource_deallocate(pid, tasks[pid].resource_amount);
    winmgr_clean_window(pid);

    PCB task = tasks[pid];

    tasks[task.next_pid].previous_pid = tasks[pid].previous_pid;
    tasks[task.previous_pid].next_pid = tasks[pid].next_pid;
    tasks[pid].state = 0;
    tasks_active[pid] = 0;

    if(pid == last_task_pid){
        last_task_pid = task.previous_pid;
    }

    num_task--;

    __asm__ volatile ("sti");   // reenable interrupts
}


void task_schedule(){
    int next_pid = current_task->next_pid;

    while (tasks[next_pid].state == TERMINATED){
        next_pid = tasks[next_pid].next_pid;
    }    

    PCB* new = &tasks[next_pid];
    PCB* old = current_task;
    if(new == old) return; // Will break if not switching due to asm code

    current_task = new;

    tss.esp0 = new->k_stack;

    if(old->state == RUNNING) old->state = READY;
    new->state = RUNNING;

    
    // We need to get old process's kernel stack before switching page tables
    // Doesn't need to clear it when a task is done
    // It doesn't matter since we are copying and reloading them each time
    paging_dir_copy_single(tasks_page_dir[old->pid], &tasks_page_dir[new->pid], (void*) old->k_stack - PAGE_FRAME_SIZE);
    
    // Switching page tables
    paging_use_page_dir(new->cr3);

    // flush pages
    paging_flush_tlb_single((void*) old->k_stack);
    paging_flush_tlb_range((void*) 0, (void*) (new->resource_amount * PAGE_FRAME_SIZE));

    switch_context(&(old->context), new->context);
}
