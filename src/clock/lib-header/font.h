
#ifndef _FONT_H
#define _FONT_H

#include "../lib-header/stdtype.h"
#include "../lib-header/window_manager.h"

//TODO: Document
typedef struct font_info{
    uint32_t font[128];
    uint8_t width;
    uint8_t height;
} font_info;

void font_load(font_info* finfo, char* path, uint32_t current_cluster);
void font_clear(font_info* finfo);
void font_write(window_info* winfo, font_info finfo, uint8_t row, uint8_t col, char c, uint8_t color);

#endif