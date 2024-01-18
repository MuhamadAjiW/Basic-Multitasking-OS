#include "lib-header/stdtype.h"
#include "lib-header/syscall.h"
#include "lib-header/window_manager.h"

const uint8_t window_size = 2;

window_info winfo = {
    .mainBuffer = (uint16_t*) 1,
    .xloc = 0,
    .yloc = 1,
    .xlen = window_size,
    .ylen = window_size
};

int main(void) {
    // Note: I have no idea why but a process has to start with a syscall
    // Or else there wouldn't be any interrupt happening
    syscall(SYSCALL_NULL, 0, 0, 0);

    window_init(&winfo);

    window_write(&winfo, 0, 0, 'X', 0xf, 0x0);
    window_write(&winfo, 0, 1, 'Y', 0xf, 0x0);
    window_write(&winfo, 1, 0, 'Y', 0xf, 0x0);
    window_write(&winfo, 1, 1, 'X', 0xf, 0x0);

    window_register(&winfo);
    
    int8_t movx = 1;
    int8_t movy = 1;


    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
    while (TRUE){
        __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADB00F));

        winfo.xloc += movx;
        if(winfo.xloc == SCREEN_WIDTH - window_size || winfo.xloc == 0){
            movx = (-1) * movx;
        }

        winfo.yloc += movy;
        if(winfo.yloc == SCREEN_HEIGHT - window_size || winfo.yloc == 0){
            movy = (-1) * movy;
        }

        window_update(&winfo);

        delay(50);
    }

    return 0;
}
