#ifndef _KERNEL_LOADER
#define _KERNEL_LOADER

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

#endif