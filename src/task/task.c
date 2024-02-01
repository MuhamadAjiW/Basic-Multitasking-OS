#include "../lib-header/task.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/gdt.h"
#include "../lib-header/tss.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/resource.h"
#include "../lib-header/paging.h"

#include "../lib-header/fat32.h"

// Note: Would be interesting to make tasks with dynamic
PCB tasks[MAX_TASKS] = {0};
PageDirectory tasks_page_dir[MAX_TASKS] = {0};
uint32_t num_task = 0;
PCB* current_task;

extern TSSEntry tss;

void task_initialize(){
    num_task = 1;
    current_task = &tasks[0];
    current_task->pid = 0;
    current_task->k_stack = KERNEL_VMEMORY_OFFSET + PAGE_FRAME_SIZE;
    *(current_task->name) = '0';
    current_task->cr3 = (PageDirectory*)((uint32_t) &tasks_page_dir[0] - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);
    paging_dir_copy(_paging_kernel_page_directory, &tasks_page_dir[0]);
    current_task->state = RUNNING;
}

// TODO: Manage, this is just the function
uint8_t task_create(FAT32DriverRequest request, uint8_t stack_type, uint32_t eflags){
    __asm__ volatile ("cli");   // Stop interrupts when creating a task
    if(num_task == MAX_TASKS) return 0;

    // TODO: optimize, using a whole page for stack is really excessive
    // Allocate resources with ceiling division with at least 1MB of user stack and always 1 extra page for kernel stack
    uint32_t resource_amount = ((0x100000 + request.buffer_size + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE) + 1;
    
    // Check resource availability
    if (!resource_check(resource_amount)) return 0;
    
    // pid is set as the last task
    uint32_t pid = num_task;

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
// Basically looping endlessly looking for terminated tasks and cleaning it up afterwards
void task_clean_scan(){
    for (uint32_t i = 0; i < num_task; i++){
        if(tasks[i].state == TERMINATED){
            task_clean(i);
        }
    }
}

// Cleaning up one task
void task_clean(uint32_t pid){
    resource_deallocate(pid, tasks[pid].resource_amount);

    __asm__ volatile ("cli");   // Stop interrupts on this part
    num_task--;
    for (uint32_t i = pid; i < num_task - 1; i++){
        tasks[i] = tasks[i + 1];
        tasks[i].pid = i;
        tasks_page_dir[i] = tasks_page_dir[i + 1];
    }
    __asm__ volatile ("sti");   // reenable interrupts
}


void task_schedule(){
    int next_id;
    
    do{
        next_id = (current_task->pid + 1) % num_task;
    } while (tasks[next_id].state == TERMINATED);
    

    PCB* new = &tasks[next_id];
    PCB* old = current_task;
    if(new == old) return; // Will break if not switching due to asm code

    current_task = new;

    tss.esp0 = new->k_stack;

    old->state = READY;
    new->state = RUNNING;

    
    // We need to get old process's kernel stack before switching page tables
    // Doesn't need to clear it when a task is done
    // It doesn't matter since we are copying and reloading them each time
    paging_dir_copy_single(tasks_page_dir[old->pid], &tasks_page_dir[new->pid], (void*) old->k_stack - PAGE_FRAME_SIZE);
    paging_flush_tlb_single((void*) old->k_stack);

    // Also flush user stack
    paging_flush_tlb_range((void*) 0, (void*) (new->resource_amount * PAGE_FRAME_SIZE));

    // Switching page tables
    paging_use_page_dir(new->cr3);

    switch_context(&(old->context), new->context);
}
