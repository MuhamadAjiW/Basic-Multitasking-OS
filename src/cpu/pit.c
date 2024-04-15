#include "../lib-header/idt.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/portio.h"
#include "../lib-header/process.h"

uint32_t tick = 0;

void tick_counter(){
    tick++;
}

void pit_isr(struct InterruptFrame* iframe){
    tick_counter();
    process_schedule(iframe);
}

void set_pit_freq(uint32_t freq){
    uint32_t x = 1193182 / freq;
    uint8_t low = (uint8_t) (x & 0xff);
    uint8_t high = (uint8_t) ( (x >> 8) & 0xff);

    out(0x43, 0x36);
    out(0x40, low);
    out(0x40, high);

    return;
}

void sleep(uint32_t duration){
    uint32_t cachedTime = tick;
    while (cachedTime + duration > tick){}

    return;
}

uint32_t get_tick(){
    return tick;
}
