#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "stdtype.h"
#include "lib-header/paging.h"

// Vesa driver
// https://wiki.osdev.org/Bochs_VBE_Extensions
// https://cvs.savannah.nongnu.org/viewvc/*checkout*/vgabios/vgabios/vbe_display_api.txt
#define VBE_DISPI_BANK_ADDRESS          0xA0000
#define VBE_DISPI_BANK_SIZE_KB          64

#define VBE_DISPI_MAX_XRES              1024
#define VBE_DISPI_MAX_YRES              768

#define VBE_DISPI_IOPORT_INDEX          0x01CE
#define VBE_DISPI_IOPORT_DATA           0x01CF

#define VBE_DISPI_INDEX_ID              0x0
#define VBE_DISPI_INDEX_XRES            0x1
#define VBE_DISPI_INDEX_YRES            0x2
#define VBE_DISPI_INDEX_BPP             0x3
#define VBE_DISPI_INDEX_ENABLE          0x4
#define VBE_DISPI_INDEX_BANK            0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH      0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT     0x7
#define VBE_DISPI_INDEX_X_OFFSET        0x8
#define VBE_DISPI_INDEX_Y_OFFSET        0x9

#define VBE_DISPI_ID0                   0xB0C0
#define VBE_DISPI_ID1                   0xB0C1
#define VBE_DISPI_ID2                   0xB0C2
#define VBE_DISPI_ID3                   0xB0C3
#define VBE_DISPI_ID4                   0xB0C4

#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_LFB_ENABLED           0x40
#define VBE_DISPI_NOCLEARMEM            0x80

#define VBE_DISPI_LFB_PHYSICAL_ADDRESS  (uint32_t *) 0xFD000000


// Personal defines
#define DEFAULT_COLOR_BG 0xFFFFFFFF
#define DEFAULT_COLOR_FG 0x00000000

/**
 * Configures VESA
 */
void graphics_initialize(uint32_t width, uint32_t height, uint32_t colorbits, uint8_t linear_framebuffer, uint8_t clear);

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
void graphics_set(uint16_t row, uint16_t col, uint32_t color);

//TODO: Document
typedef struct graphics_info{
    uint16_t width;
    uint16_t height;
    uint16_t color_depth;
    uint32_t* buffer;
    uint32_t size;
} graphics_info;
void vesa_write_register(uint16_t index, uint16_t data);
uint16_t vesa_read_register(uint16_t index);

#endif