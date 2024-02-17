
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include "stdtype.h"

#define MAX_WINDOW_NUM 64
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

//TODO: Document
struct window_info{
    uint16_t* mainBuffer;
    uint16_t* rearBuffer;
    uint16_t  xloc;
    uint16_t  xlen;
    uint16_t  yloc;
    uint16_t  ylen;
    uint8_t   id;
    uint8_t   pid;
    uint8_t   active;       // Activeness not implemented yet
};
void window_init(struct window_info* winfo);
void window_clear(struct window_info* winfo);
void window_write(struct window_info* winfo, uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg);
void window_register(struct window_info* winfo);
void window_update(struct window_info* winfo);
void window_close(struct window_info* winfo);

#endif
