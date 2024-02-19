
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib-header/framebuffer.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/portio.h"

// Double buffering to avoid screen tearing
uint16_t screen_buffer[2000];

void framebuffer_enable_cursor(){
	out(CURSOR_PORT_CMD, 0x0A);
	out(CURSOR_PORT_DATA, (in(CURSOR_PORT_DATA) & 0xC0) | 0xE);
 
	out(CURSOR_PORT_CMD, 0x0B);
	out(CURSOR_PORT_DATA, (in(CURSOR_PORT_DATA) & 0xE0) | 0xF);
}

void framebuffer_disable_cursor(){
	out(CURSOR_PORT_CMD, 0x0A);
	out(CURSOR_PORT_DATA, 0x20);
}

void framebuffer_set_cursor(uint8_t r, uint8_t c) {
	uint16_t pos = r * 80 + c;
 
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg){
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *) screen_buffer + (row * 80 + col);
    *where = c | (attrib << 8);
}

void framebuffer_set(uint8_t row, uint8_t col, uint16_t info){
    volatile uint16_t * where;
    where = (volatile uint16_t *) screen_buffer + (row * SCREEN_WIDTH + col);
    *where = info;
}

void framebuffer_clear(void) {
    for (uint8_t i = 0; i < 25; i++){
        for (uint8_t j = 0; j < 80; j++){
            framebuffer_write(i, j, 0, 0x0, 0xf);
        }
    }
    framebuffer_display();
}

void framebuffer_display(){
    memcpy((void*) MEMORY_FRAMEBUFFER, screen_buffer, 4000);
}
