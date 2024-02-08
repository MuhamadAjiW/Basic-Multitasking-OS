#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "stdtype.h"

/* further reading: 
    https://wiki.osdev.org/VGA_Hardware#Memory_Layout_in_256-color_graphics_modes
    http://www.osdever.net/FreeVGA/home.htm
*/

/* VGA Port Macros */
#define MISC_OUT_REG 0x3c2
#define FEAT_CONT_REG 0x3ca

#define ATTR_ADDR_REG 0x3c0
#define ATTR_DATA_REG 0x3c1

#define CRT_ADDR_REG 0x3d4
#define CRT_DATA_REG 0x3d5

#define MEM_MODE_REG 0x3c4

#define DAC_MASK_REG 0x3c6
#define DAC_READ_REG 0x3c7
#define DAC_WRITE_REG 0x3c8
#define DAC_DATA_REG 0x3c9
#define DAC_STATE_REG 0x3c7

#define GRAP_ADDR_REG 0x3ce
#define GRAP_DATA_REG 0x3cf

#define INPUT_STATUS_1 0x3da

/* Graphics Attributes */
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200


#define BLOCK_WIDTH 5
#define BLOCK_HEIGHT 8

#define BUFFER_HEIGHT 25
#define BUFFER_WIDTH 64


#define DEFAULT_COLOR_FG 0x1
#define DEFAULT_COLOR_INPUT 0x2
#define DEFAULT_COLOR_BG 0x3
#define DEFAULT_CURSOR_COLOR 0x4

// #define DEFAULT_COLOR_BG 84
// #define DEFAULT_COLOR_FG 15
// #define DEFAULT_CURSOR_COLOR 13

#define MEMORY_GRAPHICS (uint8_t *) 0xC00A0000 //0xA0000 before remapping

/**
 * Configures VGA to use mode 13h
 * Also loads necessary palette for the system
 * 
 */
void graphics_initialize();

/**
 * Overwrites every pixel with current given background image
 * 
 */
void graphics_clear();

/**
 * Overwrites shown screen with writable buffer
 * 
 */
void graphics_display();

/**
 * Sets 1 pixel of screen to a certain color
 * 
 * @param col       horizontal coordinate of pixel
 * @param row       vertical coordinate of pixel
 * @param color     index of color palette
 */
void graphics_set(uint16_t row, uint16_t col, uint8_t color);

//TODO: Document
void graphics_palette_update(void* palette, uint32_t len, uint32_t offset);
void graphics_palette_reset();

#endif