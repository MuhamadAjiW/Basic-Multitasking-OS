
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib-header/syscall.h"


void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void delay(uint32_t tick){
    uint32_t currentTick = 0;
    uint32_t cachedTick = 0;
    syscall(SYSCALL_GET_TICK, (uint32_t) &currentTick, 0, 0);
    cachedTick = currentTick + tick;

    while (currentTick < cachedTick){
        syscall(SYSCALL_GET_TICK, (uint32_t) &currentTick, 0, 0);
    }
}

void close_window(uint32_t id){
    syscall(SYSCALL_WINMGR_CLOSE, id, 0, 0);
}

void exit(){
    syscall(SYSCALL_PROCESS_EXIT, 0, 0, 0);
    while (true); // wait for pit interrupt
}