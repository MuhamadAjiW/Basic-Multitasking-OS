#include "../lib-header/process.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/gdt.h"
#include "../lib-header/tss.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/paging.h"
#include "../lib-header/fat32.h"

extern struct TSSEntry tss;

// Note: Would be interesting to make process with dynamic
// Process is managed like a linked list for performance reasons with indices as pointers because static memory (check PCB structure)
// Algorithm for processing is round robin
// Most of this gets slow because of static memory, very recommended to create a dedicated kernel heap for processing
// Task 0 is always the kernel
struct PCB process_array[MAX_PROCESS] = {0};
struct PageDirectory process_page_dir[MAX_PROCESS] = {0};
uint8_t process_active[MAX_PROCESS] = {0};

uint32_t num_process = 0;
struct PCB* current_process;
uint32_t last_process_pid;

void process_initialize(){
    current_process = &process_array[0];
    current_process->pid = 0;
    current_process->k_stack = KERNEL_VMEMORY_OFFSET + PAGE_FRAME_SIZE - 4;
    
    current_process->name[0] = 'k';
    current_process->name[1] = 'e';
    current_process->name[2] = 'r';
    current_process->name[3] = 'n';
    current_process->name[4] = 'e';
    current_process->name[5] = 'l';
    current_process->name[6] = 0;

    current_process->cr3 = (struct PageDirectory*)((uint32_t) &process_page_dir[0] - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);
    current_process->state = RUNNING;

    current_process->frame_amount = KERNEL_PAGE_COUNT;
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
    tinfo->frame_amount = process.frame_amount;
    tinfo->pid = process.pid;
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
    // TODO: implement generation algorithm

        // Template, ga kunjaw sih ini bebas aja mau implementasinya
        uint32_t pid = 1;
        while (process_active[pid]){
            pid++;
        }

    return pid;
}

uint8_t process_create_user_proc(struct FAT32DriverRequest request){
    __asm__ volatile ("cli");   // Stop interrupts when creating a process

    //TODO: check process and resource availability
        
        // Kunjaw:
        if(num_process == MAX_PROCESS) {
            __asm__ volatile ("sti");   // reenable interrupts
            return 0;
        }

        // Allocate resources with ceiling division with at least 1MB of user stack and always 1 extra page for kernel stack
        uint32_t frame_amount = ((0x100000 + request.buffer_size + PAGE_FRAME_SIZE - 1) / PAGE_FRAME_SIZE) + 1;
        
        // Check resource availability
        if (!paging_allocate_check(frame_amount) || frame_amount > MAX_PROCESS_FRAMES){
            __asm__ volatile ("sti");   // reenable interrupts
            return 0;
        }
    
    // Initialize process base data
    uint32_t pid = process_generate_pid();
    process_array[pid].pid = pid;
    for (uint8_t i = 0; i < 8; i++){
        process_array[pid].name[i] = request.name[i];
    }
    process_array[pid].state = NEW;

    // Initialize paging directory
    struct PageDirectory* page_dir = &process_page_dir[pid];
    process_array[pid].cr3 = (struct PageDirectory*) ((uint32_t) page_dir - KERNEL_VMEMORY_OFFSET + KERNEL_PMEMORY_OFFSET);
    paging_dirtable_init(page_dir);
    paging_clone_directory_entry((void*)(current_process->k_stack), &process_page_dir[current_process->pid], &process_page_dir[pid]);

    // TODO: Allocate frames
    // uint32_t frame_amount;
    // uint32_t u_stack;
    // uint32_t k_stack;

        // Kunjaw:
        process_array[pid].frame_amount = frame_amount;

        // user stack
        uint32_t u_stack = (frame_amount - 1) * PAGE_FRAME_SIZE;
        uint32_t virt_addr = 0;
        for(uint32_t i = 0; i < frame_amount - 1; i++){
            virt_addr = i * PAGE_FRAME_SIZE;
            process_array[pid].virt_addr_used[i] = (void*) virt_addr;
            paging_allocate_page_frame((void*) virt_addr, page_dir);
        }

        // kernel stack
        uint32_t k_stack = (pid + 1) * PAGE_FRAME_SIZE + KERNEL_VMEMORY_OFFSET - 4;
        process_array[pid].virt_addr_used[frame_amount - 1] = (void*) k_stack;
        paging_allocate_page_frame((void*) k_stack, page_dir);

    // Assign process as the last entry on the linked list
    process_array[pid].previous_pid = last_process_pid;
    process_array[pid].next_pid = 0;
    process_array[0].previous_pid = pid;
    process_array[last_process_pid].next_pid = pid;
    last_process_pid = pid;
    process_active[pid] = 1;

    // Prepare the entry for new process
    paging_use_page_dir(process_array[pid].cr3);

    // Flush pages stack pages
    paging_flush_tlb_range((void*) 0, (void*) ((frame_amount - 1) * PAGE_FRAME_SIZE));
    paging_flush_tlb_single((void*)k_stack);

    // Load file into memory at 0
    request.buf = (void*) 0;
    load(request);

    // Prepare new process environment
    uint32_t cs = GDT_USER_CODE_SEGMENT_SELECTOR | PRIVILEGE_USER;
    uint32_t ds = GDT_USER_DATA_SEGMENT_SELECTOR | PRIVILEGE_USER;

    // Set InterruptFrame to be at the bottom of the given kernel stack
    // volatile uint32_t* userss = (uint32_t*)( k_stack - 4);
    // *userss = ds;
    // volatile uint32_t* useresp = userss - 1;
    // *useresp = u_stack;
    // uint8_t* k_esp = (uint8_t*) useresp;

    // k_esp -= sizeof(struct InterruptFrame);
    // struct InterruptFrame* iframe = (struct InterruptFrame*) k_esp;
    // memset(iframe, 0, sizeof(struct InterruptFrame));

    // iframe->int_stack.cs = cs;
    // iframe->cpu.segment.ds = ds;
    // iframe->int_stack.eflags = EFLAGS_USER_PROC;

    // Note: entry is assumed to be always set at 0 when linking a program
    // iframe->int_stack.eip = (uint32_t) request.buf;

    // k_esp -= sizeof(struct Context);
    // struct Context* context = (struct Context*) k_esp;
    // context->edi = 0;
    // context->esi = 0;
    // context->ebx = 0;
    // context->ebp = 0;
    // context->eip = (uint32_t) restore_context;

    struct InterruptFrame emptyFrame = {0};
    process_array[pid].cpu_state = emptyFrame;
    process_array[pid].cpu_state.cpu.segment.ds = ds;
    process_array[pid].cpu_state.int_stack.cs = cs;
    process_array[pid].cpu_state.int_stack.eflags = EFLAGS_USER_PROC;
    process_array[pid].cpu_state.int_stack.eip = (uint32_t) request.buf;
    process_array[pid].userss = ds;
    process_array[pid].useresp = u_stack;
    
    // Save the data on our process list
    process_array[pid].k_stack = k_stack;
    // process_array[pid].context = context;

    // Return to initial task space
    paging_use_page_dir(current_process->cr3);

    // Flush pages
    paging_flush_tlb_range((void*) 0, (void*) ((frame_amount - 1) * PAGE_FRAME_SIZE));
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
    
    for (uint32_t i = 0; i < process_array[pid].frame_amount; i++){
        paging_free_page_frame(process_array[pid].virt_addr_used[i], &process_page_dir[pid]);
    }
    
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

void process_switch(struct PCB* old, struct PCB* new, __attribute__((unused)) struct InterruptFrame iframe){
    tss.esp0 = new->k_stack;

    // We need to get old process's kernel stack before switching page tables
    // Doesn't need to clear it when a process is done
    // It doesn't matter since we are copying and reloading them each time
    paging_clone_directory_entry((void*)(old->k_stack), &process_page_dir[old->pid], &process_page_dir[new->pid]);

    // Switching page tables
    paging_use_page_dir(new->cr3);

    // flush pages
    paging_flush_tlb_single((void*) old->k_stack);
    paging_flush_tlb_range((void*) 0, (void*) (new->frame_amount * PAGE_FRAME_SIZE));

    old->cpu_state = iframe;

    switch_context(&(new->cpu_state));
}

void process_schedule(struct InterruptFrame iframe){
    // TODO: implement scheduling algorithm

        // Template, ga kunjaw sih ini bebas aja mau implementasinya
        int next_pid = current_process->next_pid;

        while (process_array[next_pid].state == TERMINATED){
            next_pid = process_array[next_pid].next_pid;
        }    

        struct PCB* new = &process_array[next_pid];
        struct PCB* old = current_process;
        if(new == old) return; // Will break if not switching due to asm code

        current_process = new;

        if(old->state == RUNNING) old->state = READY;
        new->state = RUNNING;

    process_switch(old, new, iframe);
}
