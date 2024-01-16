#ifndef _IDT_H
#define _IDT_H

#include "stdtype.h"

// IDT hard limit, see Intel x86 manual 3a - 6.10 Interrupt Descriptor Table
#define IDT_MAX_ENTRY_COUNT    256
#define ISR_STUB_TABLE_LIMIT   64
#define INTERRUPT_GATE_R_BIT_1 0b000
#define INTERRUPT_GATE_R_BIT_2 0b110
#define INTERRUPT_GATE_R_BIT_3 0b0

// Some GDT Constant
#define GDT_KERNEL_CODE_SEGMENT_SELECTOR 0x8
#define GDT_KERNEL_DATA_SEGMENT_SELECTOR 0x10


// Interrupt Handler / ISR stub for reducing code duplication, this array can be iterated in initialize_idt()
extern void *isr_stub_table[ISR_STUB_TABLE_LIMIT];
extern struct IDTR _idt_idtr;

/**
 * IDTGate, IDT entry that point into interrupt handler
 * Struct defined exactly in Intel x86 Vol 3a - Figure 6-2. IDT Gate Descriptors
 *
 * @param offset_low  Lower 16-bit offset
 * @param segment     Memory segment
 * @param _reserved   Reserved bit, bit length: 5
 * @param _r_bit_1    Reserved for idtgate type, bit length: 3
 * @param _r_bit_2    Reserved for idtgate type, bit length: 3
 * @param gate_32     Is this gate size 32-bit? If not then its 16-bit gate
 * @param _r_bit_3    Reserved for idtgate type, bit length: 1
 * @param _privilege DPL, Descriptor privilege level
 * @param valid       P, Segment Present Flag
 * @param offset_high Higher 16-bit offset
 */
typedef struct IDTGate {
    uint16_t offset_low     : 16;
    uint16_t segment        : 16;
    uint8_t  _reserved      : 5;
    uint8_t  _r_bit_1       : 3;
    uint8_t  _r_bit_2       : 3;
    uint8_t  gate_32        : 1;
    uint8_t  _r_bit_3       : 1;
    uint8_t  _privilege     : 2;
    uint8_t  valid_bit      : 1;
    uint16_t offset_high      : 16;
} __attribute__((packed)) IDTGate;

/**
 * Interrupt Descriptor Table, containing lists of IDTGate.
 * One IDT already defined in idt.c
 *
 */
typedef struct InterruptDescriptorTable{
    IDTGate table[IDT_MAX_ENTRY_COUNT];
} __attribute__((packed)) InterruptDescriptorTable;

/**
 * IDTR, carrying information where's the IDT located and size.
 * Global kernel variable defined at idt.c.
 *
 *  @param size size of InterruptDescriptorTable - 1
 *  @param address address of loaded Interrupt Descriptor Table
 */
typedef struct IDTR{
   uint16_t size;
   InterruptDescriptorTable* address;
} __attribute__((packed)) IDTR;



/**
 * Set IDTGate with proper interrupt handler values.
 * Will directly edit global IDT variable and set values properly
 * 
 * @param int_vector       Interrupt vector to handle
 * @param handler_address  Interrupt handler address
 * @param gdt_seg_selector GDT segment selector, for kernel use GDT_KERNEL_CODE_SEGMENT_SELECTOR
 * @param privilege        Descriptor privilege level
 */
void set_interrupt_gate(uint8_t int_vector, void *handler_address, uint16_t gdt_seg_selector, uint8_t privilege);


/**
 * Set IDT with proper values and load with lidt
 */
void initialize_idt(void);

#endif