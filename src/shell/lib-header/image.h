#ifndef _IMAGE_H
#define _IMAGE_H

#include "../lib-header/stdtype.h"

//TODO: Document
typedef struct image_info{
    uint32_t* map;
    uint16_t width;
    uint16_t height;
} image_info;


void image_load(image_info* imginfo, char* path, uint32_t current_cluster);
void image_delete(image_info* imginfo);

#endif