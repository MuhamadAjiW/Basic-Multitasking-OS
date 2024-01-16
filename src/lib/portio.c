#include "../lib-header/stdtype.h"
#include "../lib-header/portio.h"

/** x86 inb/outb:
 * @param dx target port 
 * @param al input/output byte
 */

void out(uint16_t port, uint8_t data) {
    __asm__(
        "outb %%al, %%dx"
        : // <Empty output operand>
        : "a"(data), "d"(port)
    );
    return;
}

void out2(uint16_t port, uint16_t data) {
    __asm__(
        "out %%ax, %%dx"
        : // <Empty output operand>
        : "a"(data), "d"(port)
    );
    return;
}

void out4(uint32_t port, uint32_t data) {
    __asm__(
        "out %%eax, %%edx"
        : // <Empty output operand>
        : "a"(data), "d"(port)
    );
    return;
}

uint8_t in(uint16_t port) {
    uint8_t result;
    __asm__ volatile(
        "inb %%dx, %%al" 
        : "=a"(result) 
        : "d"(port)
    );
    return result;
}

uint16_t in2(uint16_t port) {
    uint16_t result;
    __asm__ volatile(
        "in %%dx, %%ax" 
        : "=a"(result) 
        : "d"(port)
    );
    return result;
}

uint32_t in4(uint32_t port) {
    uint32_t result;
    __asm__ volatile(
        "inl %%dx, %%eax" 
        : "=a"(result) 
        : "d"(port)
    );
    return result;
}