#ifndef _IMAGE_H
#define _IMAGE_H

#include "../lib-header/stdtype.h"

//TODO: Document
typedef struct image_info{
    uint8_t* map;
    uint16_t width;
    uint16_t height;
    uint8_t* palette;
    uint8_t palette_len;
} image_info;


void image_load(image_info* imginfo, char* path, uint32_t current_cluster);
void image_delete(image_info* imginfo);
void image_change_palette(image_info imginfo);

#endif