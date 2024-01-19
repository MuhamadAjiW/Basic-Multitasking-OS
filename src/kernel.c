#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
#include "lib-header/framebuffer.h"
#include "lib-header/kernel_loader.h"
#include "lib-header/idt.h"
#include "lib-header/interrupt.h"
#include "lib-header/keyboard.h"
#include "lib-header/pit.h"
#include "lib-header/tss.h"
#include "lib-header/syscall.h"
#include "lib-header/fat32.h"
#include "lib-header/paging.h"
#include "lib-header/memory_manager.h"

#include "lib-header/task.h"
#include "lib-header/window_manager.h"

// TODO: Delete, this is for testing
extern PCB tasks[];

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);

    activate_interrupts();
    
    gdt_install_tss();
    set_tss_register_kernel();
    set_tss_kernel_current_stack();
    task_initialize();

    activate_irq(IRQ_KEYBOARD);
    activate_irq(IRQ_PRIMARY_ATA);
    activate_irq(IRQ_SECOND_ATA);

    register_irq_handler(IRQ_KEYBOARD, keyboard_isr);

    enable_system_calls();

    memory_initialize();

    keyboard_state_activate();

    // FAT32DriverRequest shell = {
    //     .buf                   = (void*) 0,
    //     .name                  = "sh",
    //     .ext                   = "\0\0\0",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER + 1,
    //     .buffer_size           = 0x100000,
    // };
    // task_create(shell, 1, STACKTYPE_USER, EFLAGS_BASE | EFLAGS_INTERRUPT | EFLAGS_PARITY);
    

    winmgr_initalilze();

    set_pit_freq(DEFAULT_FREQUENCY);
    register_irq_handler(IRQ_TIMER, pit_isr);
    activate_irq(IRQ_TIMER);

    FAT32DriverRequest clock = {
        .buf                   = (void*) 0,
        .name                  = "cl",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER + 1,
        .buffer_size           = 0x100000,
    };

    for (int i = 1; i < 40; i++)    {
        task_create(clock, i, STACKTYPE_USER, EFLAGS_BASE | EFLAGS_INTERRUPT | EFLAGS_PARITY);
    }
    

    while (TRUE);
}