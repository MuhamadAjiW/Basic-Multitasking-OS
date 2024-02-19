
#include <stdint.h>
#include <stdbool.h>

#include "../lib-header/stdlib.h"
#include "../lib-header/syscall.h"
#include "../lib-header/window_manager.h"

// Note: Would be interesting to make this a dynamically linked library 
void window_init(struct window_info* winfo){
    winfo->mainBuffer = (uint16_t*) malloc (sizeof(uint16_t) * ((uint32_t)winfo->xlen) * ((uint32_t)winfo->ylen));
    winfo->rearBuffer = (uint16_t*) malloc (sizeof(uint16_t) * ((uint32_t)winfo->xlen) * ((uint32_t)winfo->ylen));
}

void window_clear(struct window_info* winfo){
    free(winfo->mainBuffer);
    free(winfo->rearBuffer);
}

void window_write(struct window_info* winfo, uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg){
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *) winfo->mainBuffer + (row * winfo->xlen + col);
    *where = c | (attrib << 8);
}

void window_register(struct window_info* winfo){
    syscall(SYSCALL_WINMGR_REG, (uint32_t) winfo, 0, 0);
}

void window_update(struct window_info* winfo){
    syscall(SYSCALL_WINMGR_UPDATE, (uint32_t) winfo, 0, 0);
}

void window_close(struct window_info* winfo){
    syscall(SYSCALL_WINMGR_CLOSE, (uint32_t) winfo->id, 0, 0);
}