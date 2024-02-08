#ifndef _ANIM_H
#define _ANIM_H

#include "../lib-header/stdtype.h"

//TODO: Document
typedef struct anim_info{
    uint8_t* map;
    uint16_t width;
    uint16_t height;
    uint16_t frame_count;
    uint8_t* palette;
    uint16_t palette_len;
} anim_info;

void anim_load(anim_info* anim, char* path, uint32_t current_cluster);
void anim_delete(anim_info* anim);

#endif