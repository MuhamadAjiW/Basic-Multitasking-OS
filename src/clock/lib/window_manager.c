#include "../lib-header/stdlib.h"
#include "../lib-header/syscall.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/window_manager.h"

// Note: Would be interesting to make this a dynamically linked library 
void window_init(window_info* winfo){
    winfo->main_buffer = (uint16_t*) malloc (sizeof(uint16_t) * ((uint32_t)winfo->xlen) * ((uint32_t)winfo->ylen));
    winfo->rear_buffer = (uint16_t*) malloc (sizeof(uint16_t) * ((uint32_t)winfo->xlen) * ((uint32_t)winfo->ylen));
}

void window_clear(window_info* winfo){
    free(winfo->main_buffer);
    free(winfo->rear_buffer);
}

void window_draw_pixel(window_info* winfo, uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg){
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *) winfo->main_buffer + (row * winfo->xlen + col);
    *where = c | (attrib << 8);
}

void window_register(window_info* winfo){
    syscall(SYSCALL_WINMGR_REG, (uint32_t) winfo, 0, 0);
}

void window_update(window_info* winfo){
    syscall(SYSCALL_WINMGR_UPDATE, (uint32_t) winfo, 0, 0);
}

void window_close(window_info* winfo){
    syscall(SYSCALL_WINMGR_CLOSE, (uint32_t) winfo->id, 0, 0);
}