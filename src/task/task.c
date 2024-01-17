#include "lib-header/task.h"
#include "lib-header/interrupt.h"
#include "lib-header/gdt.h"
#include "lib-header/tss.h"
#include "lib-header/stdmem.h"
#include "lib-header/memory_manager.h"
#include "lib-header/resource.h"
#include "lib-header/paging.h"

#include "lib-header/fat32.h"

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
}

// TODO: Manage, this is just the function
uint8_t create_task(FAT32DriverRequest request, uint32_t pid, uint8_t stack_type, uint32_t eflags){

    // Allocate resources with ceiling division
    uint8_t resource_amount = (request.buffer_size + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE;
    uint32_t t_stack = allocate_resource(resource_amount, pid) - 4;
    if (!t_stack) return 0;

    request.buf = (void*)(t_stack + 4 - resource_amount * PAGE_FRAME_SIZE);

    load(request);

    // Set Context to be at the bottom of the given stack
    uint8_t* t_esp = (uint8_t*) t_stack;
    t_esp -= sizeof(Context);
    Context* c_ptr = (Context*) t_esp;
    memset(c_ptr, 0, sizeof(Context));

    uint32_t cs = stack_type == STACKTYPE_KERNEL ? GDT_KERNEL_CODE_SEGMENT_SELECTOR : (GDT_USER_CODE_SEGMENT_SELECTOR | PRIVILEGE_USER);
    uint32_t ds = stack_type == STACKTYPE_KERNEL ? GDT_KERNEL_DATA_SEGMENT_SELECTOR : (GDT_USER_DATA_SEGMENT_SELECTOR | PRIVILEGE_USER);

    c_ptr->cs = cs;
    c_ptr->segments.ds = ds;

    c_ptr->segments.gs = ds;
    c_ptr->segments.fs = ds;
    c_ptr->segments.es = ds;

    c_ptr->userss = ds;
    c_ptr->useresp = t_stack;
    c_ptr->eflags = eflags;

    //NOTE: entry is assumed to be always set at 0 when linking a program
    c_ptr->eip = (uint32_t)request.buf;

    t_esp -= sizeof(void*);
    void** exit = (void**) t_esp;
    *exit = restore_context;

    tasks[pid].context.registers.ebp = t_stack - 4;
    tasks[pid].esp = (uint32_t) t_esp;
    tasks[pid].pid = pid;
    tasks[pid].parent_pid = current_task->pid;
    tasks[pid].privilege = stack_type;

    num_task++;

    return 1;
}

void schedule(){
    int next_id = (current_task->pid + 1) % num_task;

    PCB* new = &tasks[next_id];
    PCB* old = current_task;
    current_task = new;

    tss.esp0 = new->context.registers.ebp;

    switch_context(old, new);
}
