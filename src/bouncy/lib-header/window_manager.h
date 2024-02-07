
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include "stdtype.h"

#define MAX_WINDOW_NUM 64
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

//TODO: Document
typedef struct window_info{
    uint16_t* main_buffer;
    uint16_t* rear_buffer;
    uint16_t  xloc;
    uint16_t  xlen;
    uint16_t  yloc;
    uint16_t  ylen;
    uint8_t   id;
    uint8_t   pid;
    uint8_t   active;       // Activeness not implemented yet
} window_info;
void window_init(window_info* winfo);
void window_clear(window_info* winfo);
void window_draw_pixel(window_info* winfo, uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg);
void window_register(window_info* winfo);
void window_update(window_info* winfo);
void window_close(window_info* winfo);

#endif
