#include "lib-header/task.h"
#include "lib-header/interrupt.h"
#include "lib-header/gdt.h"
#include "lib-header/tss.h"
#include "lib-header/stdmem.h"
#include "lib-header/memory_manager.h"


// TODO: Make tasks with dynamic memory, paging, variable authority, priority, blocking, etc
PCB tasks[MAX_TASKS] = {0};
int num_task = 0;
PCB* current_task;

//TODO: Fix this, it should NOT mess with existing stack without orderly fashion
// ALLOCATE PAGES INSTEAD!
uint32_t k_stack = 0xC0400000;
extern TSSEntry tss;

void initialize_tasking(){
    num_task = 1;
    current_task = &tasks[0];
    current_task->pid = 0;
}

void create_task(uint32_t pid, uint32_t eip, uint32_t u_stack, uint8_t STACKTYPE){

    uint8_t* k_esp = (uint8_t*) k_stack - 4;
    k_esp -= sizeof(Context);
    Context* c_ptr = (Context*) k_esp;
    memset(c_ptr, 0, sizeof(Context));

    uint32_t cs = STACKTYPE == STACKTYPE_KERNEL ? GDT_KERNEL_CODE_SEGMENT_SELECTOR : (GDT_USER_CODE_SEGMENT_SELECTOR | USER_PRIVILEGE);
    uint32_t ds = STACKTYPE == STACKTYPE_KERNEL ? GDT_KERNEL_DATA_SEGMENT_SELECTOR : (GDT_USER_DATA_SEGMENT_SELECTOR | USER_PRIVILEGE);

    c_ptr->cs = cs;
    c_ptr->segments.ds = ds;

    //TODO: Improve, might be utilized by other tasks
    c_ptr->segments.gs = ds;
    c_ptr->segments.fs = ds;
    c_ptr->segments.es = ds;
    c_ptr->registers.edi = 0;
    c_ptr->registers.esi = 0;
    c_ptr->registers.ebp = 0;
    c_ptr->registers.esp = 0;
    c_ptr->registers.ebx = 0;
    c_ptr->registers.ecx = 0;
    c_ptr->registers.eax = 0;

    c_ptr->userss = ds;
    c_ptr->useresp = u_stack;

    //TODO: Improve, flags might not always be this value
    c_ptr->eflags = 0x206;
    c_ptr->eip = eip;

    k_esp -= sizeof(ContextReturn);
    ContextReturn* exit = (ContextReturn*) k_esp;
    exit->edi = 0;
    exit->esi = 0;
    exit->ebx = 0;
    exit->ebp = 0;
    exit->ret_eip = (uint32_t) restore_context;

    tasks[pid].context.registers.ebp = k_stack - 4;
    tasks[pid].context.registers.esp = (uint32_t) k_esp;
    tasks[pid].pid = pid;

    //TODO: kernel stack size?
    k_stack = (uint32_t) k_esp - 0x40000;

    num_task++;
}

void schedule(){
    int next_id = (current_task->pid + 1) % num_task;

    PCB* new = &tasks[next_id];
    PCB* old = current_task;
    current_task = new;

    tss.esp0 = new->context.registers.ebp;

    switch_context(old, new);
}
