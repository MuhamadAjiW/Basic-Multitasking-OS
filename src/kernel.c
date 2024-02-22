#include "lib-header/portio.h"
#include "lib-header/stdtype.h"
#include "lib-header/stdmem.h"
#include "lib-header/gdt.h"
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

#include "lib-header/graphics.h"

void kernel_setup(void) {
    enter_protected_mode(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_interrupts();

    task_initialize();
    memory_initialize();

    graphics_initialize(1920, 1080, 32, 1, 0);
    graphics_clear();
    graphics_display();

    gdt_install_tss();
    set_tss_register_kernel();
    set_tss_kernel_current_stack();

    enable_system_calls();
    activate_irq(IRQ_KEYBOARD);
    activate_irq(IRQ_PRIMARY_ATA);
    activate_irq(IRQ_SECOND_ATA);

    register_irq_handler(IRQ_KEYBOARD, keyboard_isr);
    keyboard_state_activate();

    winmgr_initalilze();

    pit_set_freq(DEFAULT_FREQUENCY);
    register_irq_handler(IRQ_TIMER, pit_isr);
    activate_irq(IRQ_TIMER);

    FAT32DriverRequest shell = {
        .buf                   = (void*) 0,
        .name                  = "sh",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER + 1,
        .buffer_size           = 0x100000,
    };
    task_create(shell, STACKTYPE_USER, EFLAGS_BASE | EFLAGS_INTERRUPT | EFLAGS_PARITY, 0);

    // FAT32DriverRequest clock = {
    //     .buf                   = (void*) 0,
    //     .name                  = "sysclock",
    //     .ext                   = "prg",
    //     .parent_cluster_number = ROOT_CLUSTER_NUMBER + 1,
    //     .buffer_size           = 0x100000,
    // };
    // task_create(clock, STACKTYPE_USER, EFLAGS_BASE | EFLAGS_INTERRUPT | EFLAGS_PARITY, 0);

    // the kernel acts as a garbage collector afterwards
    while (TRUE){
        // task_clean_scan();
    }
}