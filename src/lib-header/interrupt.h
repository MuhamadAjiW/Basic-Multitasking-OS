
#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "cpu.h"

/* -- PIC constants -- */

// PIC interrupt offset
#define PIC1_OFFSET          0x20
#define PIC2_OFFSET          0x28

// PIC ports
#define PIC1                 0x20
#define PIC2                 0xA0
#define PIC1_COMMAND         PIC1
#define PIC1_DATA            (PIC1 + 1)
#define PIC2_COMMAND         PIC2
#define PIC2_DATA            (PIC2 + 1)

// PIC ACK & mask constant
#define PIC_ACK              0x20
#define PIC_DISABLE_ALL_MASK 0xFF

// PIC remap constants
#define ICW1_ICW4            0x01   /* ICW4 (not) needed */
#define ICW1_SINGLE          0x02   /* Single (cascade) mode */
#define ICW1_INTERVAL4       0x04   /* Call address interval 4 (8) */
#define ICW1_LEVEL           0x08   /* Level triggered (edge) mode */
#define ICW1_INIT            0x10   /* Initialization - required! */

#define ICW4_8086            0x01   /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO            0x02   /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE       0x08   /* Buffered mode/slave */
#define ICW4_BUF_MASTER      0x0C   /* Buffered mode/master */
#define ICW4_SFNM            0x10   /* Special fully nested (not) */


/* -- PICs IRQ list -- */
#define IRQ_OFFSET       0x20
#define IRQ_COUNT        0x10

// PIC Master
#define IRQ_TIMER        0
#define IRQ_KEYBOARD     1
#define IRQ_CASCADE      2
#define IRQ_COM2         3
#define IRQ_COM1         4
#define IRQ_LPT2         5
#define IRQ_FLOPPY_DISK  6
#define IRQ_LPT1_SPUR    7

// PIC Slave
#define IRQ_CMOS         8
#define IRQ_PERIPHERAL_1 9
#define IRQ_PERIPHERAL_2 10
#define IRQ_PERIPHERAL_3 11
#define IRQ_MOUSE        12
#define IRQ_FPU          13
#define IRQ_PRIMARY_ATA  14
#define IRQ_SECOND_ATA   15

/**
 * InterruptHandler, Pointer to a function for an interrupt handler
 * 
 * @param cpu        CPU information when interrupt is raised
 */
typedef void (*InterruptHandler)(struct InterruptFrame cpu);

// I/O port wait, around 1-4 microsecond, for I/O synchronization purpose
void io_wait(void);

// Send ACK to PIC - @param irq Interrupt request number destination, note: this function already include PIC1_OFFSET
void pic_ack(uint8_t irq);

// Shift PIC interrupt number to PIC1_OFFSET and PIC2_OFFSET (master and slave)
void pic_remap(void);

/**
 * Main interrupt handler when any interrupt / exception is raised.
 * Do not call this function normally.
 * 
 * This function will be called first if any INT 0x00 - 0x40 is raised, 
 * and will call proper ISR for respective interrupt / exception.
 * 
 * Again, this function is not for normal function call, all parameter will be automatically set when interrupt is called.
 * @param cpu        CPU information when interrupt is raised
 */
void main_interrupt_handler(struct InterruptFrame cpu);

/**
 * Registers a function as an interrupt handler.
 * 
 * @param int_no        interrupt number to be handled
 * @param handler       function to be registered as a handler
 */
void register_irq_handler(uint16_t int_no, InterruptHandler handler);

/**
 * Activates interrupts by masking PIC registers, both master and slave
 */
void activate_interrupts();

/**
 * Activates an IRQ by unmasking PIC registers
 * 
 * @param irq           irq number to be activated
 */
void activate_irq(uint8_t irq);

#endif