#include "../lib-header/task.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/gdt.h"
#include "../lib-header/tss.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/resource.h"
#include "../lib-header/paging.h"

#include "../lib-header/fat32.h"

// TODO: Make tasks with dynamic memory, paging, variable authority, priority, blocking, etc
uint8_t resources_allocated = 0;

PCB tasks[MAX_TASKS] = {0};
int num_task = 0;
PCB* current_task;

extern TSSEntry tss;

void initialize_tasking(){
    num_task = 1;
    current_task = &tasks[0];
    current_task->pid = 0;
    *(current_task->name) = '0';
    current_task->state = RUNNING;
}

// TODO: Manage, this is just the function
uint8_t create_task(FAT32DriverRequest request, uint32_t pid, uint8_t stack_type, uint32_t eflags){

    // Allocate resources with ceiling division
    // Resource amount is added by 1 for both kernel and user stacks
    uint8_t resource_amount = (request.buffer_size + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE + 1;

    // TODO: optimize, using a whole page for stack is really excessive
    uint32_t t_stack = allocate_resource(resource_amount, pid);
    if (!t_stack) return 0;

    // Initialize task
    tasks[pid].pid = pid;
    tasks[pid].parent = current_task;
    tasks[pid].state = NEW;

    // Last frame is used for stacks, half for kernel, half for user
    uint32_t u_stack = t_stack - PAGE_FRAME_SIZE/2;
    uint32_t k_stack = t_stack;

    // Load file into memory
    request.buf = (void*)(t_stack - resource_amount * PAGE_FRAME_SIZE);

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

    num_task++;

    return 1;
}

void schedule(){
    int next_id = (current_task->pid + 1) % num_task;

    PCB* new = &tasks[next_id];
    PCB* old = current_task;
    if(new == old) return; // Will break if not switching due to asm code

    current_task = new;

    tss.esp0 = new->k_stack;

    old->state = READY;
    new->state = RUNNING;

    // TODO: Delete, this is for testing
    if(next_id == 2){
        switch_context(&(old->context), new->context);
    }
    else{
        switch_context(&(old->context), new->context);
    }
}
