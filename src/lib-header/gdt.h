
#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define GDT_MAX_ENTRY_COUNT 32

extern struct GDTR _gdt_gdtr;

/**
 * Segment Descriptor storing system segment information.
 * Struct defined exactly as Intel Manual Segment Descriptor definition (Figure 3-8 Segment Descriptor).
 * Manual can be downloaded at www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html/ 
 *
 * @param segment_low  16-bit lower-bit segment limit
 * @param base_low     16-bit lower-bit base address
 * @param base_mid     8-bit middle-bit base address
 * @param type_bit     4-bit contain type flags
 * @param non_system   1-bit contain system
 */
struct SegmentDescriptor {
    // First 32-bit
    uint16_t segment_low;               //base 0-15?
    uint16_t base_low;                  //base 16-31?

    // Next 16-bit (Bit 32 to 47)
    uint8_t base_mid;                   //base 32-39
    uint8_t type_bit   : 4;             //
    uint8_t non_system : 1;
    uint8_t DPL        : 2;
    uint8_t P          : 1;

    // Next 16 bit (Bit 48 - 63)
    uint8_t segment_mid: 4;
    uint8_t AVL        : 1;
    uint8_t L          : 1;
    uint8_t DB         : 1;
    uint8_t G          : 1;
    uint8_t base_high;
    // END
} __attribute__((packed));

/**
 * Global Descriptor Table containing list of segment descriptor. One GDT already defined in memory.c.
 * More details at https://wiki.osdev.org/GDT_Tutorial
 * @param table Fixed-width array of SegmentDescriptor with size GDT_MAX_ENTRY_COUNT
 */
struct GlobalDescriptorTable {
    struct SegmentDescriptor table[GDT_MAX_ENTRY_COUNT];
} __attribute__((packed));

/**
 * GDTR, carrying information where's the GDT located and GDT size.
 * Global kernel variable defined at memory.c.
 * 
 * @param size    Global Descriptor Table size, use sizeof operator
 * @param address GDT address, GDT should already defined properly
 */
struct GDTR {
    uint16_t                     size;
    struct GlobalDescriptorTable *address;
} __attribute__((packed));


#define GDT_KERNEL_CODE_SEGMENT_SELECTOR    0x08
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR    0x10
#define GDT_USER_CODE_SEGMENT_SELECTOR      0x18
#define GDT_USER_DATA_SEGMENT_SELECTOR      0x20
#define GDT_TSS_SELECTOR                    0x28
#define PRIVILEGE_USER                      0x3
#define PRIVILEGE_KERNEL                    0x0

// Set GDT_TSS_SELECTOR with proper TSS values, accessing _interrupt_tss_entry
void gdt_install_tss(void);

#endif