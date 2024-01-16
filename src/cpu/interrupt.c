#include "lib-header/interrupt.h"
#include "lib-header/portio.h"
#include "lib-header/idt.h"
#include "lib-header/syscall.h"

InterruptHandler irq_handlers[IRQ_COUNT];
InterruptHandler syscall_handlers[SYSCALL_COUNT];

char *exception_msg[] = {
    "Division By Zero",                 //0
    "Debug",                            //1
    "Non Maskable Interrupt",           //2
    "Breakpoint",                       //3
    "Into Detected Overflow",           //4
    "Out of Bounds",                    //5
    "Invalid Opcode",                   //6
    "No Coprocessor",                   //7
    "Double Fault",                     //8
    "Coprocessor Segment Overrun",      //9
    "Bad TSS",                          //10
    "Segment Not Present",              //11
    "Stack Fault",                      //12
    "General Protection Fault",         //13
    "Page Fault",                       //14
    "Unknown Interrupt",                //15
    "Coprocessor Fault",                //16
    "Alignment Check",                  //17
    "Machine Check",                    //18
    "SIMD Floating-Point Exception",    //19
    "Virtualization Exception",         //20
    "Control Protection Exception",     //21
    "Intel Reserved",                   //22
    "Intel Reserved",                   //23
    "Intel Reserved",                   //24
    "Intel Reserved",                   //25
    "Intel Reserved",                   //26
    "Intel Reserved",                   //27
    "Hypervisor Injection Exception",   //28
    "VMM Communication Exception",      //29
    "Security Exception",               //30
    "Intel Reserved"                    //31
};

void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8)
        out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    uint8_t a1, a2;

    // Save masks
    a1 = in(PIC1_DATA); 
    a2 = in(PIC2_DATA);

    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100);      // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010);      // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore masks
    out(PIC1_DATA, a1);
    out(PIC2_DATA, a2);
}

void main_interrupt_handler(
    __attribute__((unused)) struct CPURegister cpu,
    uint32_t int_number,
    __attribute__((unused)) struct InterruptStack info
) {
    if(int_number < 0x20){
        // Error, message is exception_msg[int_number]
        __asm__ volatile("hlt");
    }
    else{
        // IRQs or Syscalls
        InterruptHandler handler = {0};
        if (int_number < 0x30)
            handler = irq_handlers[int_number - IRQ_OFFSET];
        else if (int_number == 0x30)
            handler = syscall_handlers[cpu.eax];
        
        if(handler) handler(cpu, int_number, info);

        // Refresh PIC
        if (int_number >=40){
            out(PIC2, 0x20);
        }

        out(PIC1, 0x20);
    }
    

}

void activate_interrupts(){
    out(PIC1_DATA, 0xff);
    out(PIC2_DATA, 0xff);
}

void activate_irq(uint8_t irq){
    uint16_t port;
    uint8_t value;
 
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = in(port) & ~(1 << irq);
    out(port, value);        
}

void disable_irq(uint8_t irq){
    uint16_t port;
    uint8_t value;
 
    if(irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = in(port) | (1 << irq);
    out(port, value);  
}

void register_irq_handler(uint16_t int_no, InterruptHandler handler){
    irq_handlers[int_no] = handler;
}
