
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include "stdtype.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

//TODO: Document
typedef struct window_info{
    uint32_t*  main_buffer;
    uint32_t*  rear_buffer;
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
void window_draw_pixel(window_info* winfo, uint16_t row, uint16_t col, uint32_t color);
void window_register(window_info* winfo);
void window_update(window_info* winfo);
void window_close(window_info* winfo);

#endif
