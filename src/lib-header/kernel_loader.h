
#ifndef _KERNEL_LOADER
#define _KERNEL_LOADER

// Optional linker variable : Pointing to kernel start & end address
// Note : Use & operator, example : a = (uint32_t) &_linker_kernel_stack_top;
extern uint32_t _linker_kernel_virtual_addr_start;
extern uint32_t _linker_kernel_virtual_addr_end;
extern uint32_t _linker_kernel_physical_addr_start;
extern uint32_t _linker_kernel_physical_addr_end;
extern uint32_t _linker_kernel_stack_top;

/**
 * Load GDT from gdtr and launch protected mode. This function defined in asm source code.
 * 
 * @param gdtr Pointer to already defined & initialized GDTR
 * @warning Invalid address / definition of GDT will cause bootloop after calling this function.
 */
extern void enter_protected_mode(struct GDTR *gdtr);

/**
 * Set the tss register pointing to GDT_TSS_SELECTOR with ring 0
 */
extern void set_tss_register_kernel(void);  // Note : Already implemented in kernel_loader.asm

/**
 * Set the tss register pointing to GDT_TSS_SELECTOR with ring 3
 */
extern void set_tss_register_user(void);  // Note : Already implemented in kernel_loader.asm

//TODO: Delete
/**
 * Execute user program from kernel, one way jump. This function is defined in asm source code.
 * 
 * @param virtual_addr Pointer into user program that already in memory
 * @warning            Assuming pointed memory is properly loaded
 */
extern void kernel_execute_user_program(void *virtual_addr);

#endif