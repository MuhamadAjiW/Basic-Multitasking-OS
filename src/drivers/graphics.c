#include "../lib-header/graphics.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/string.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/portio.h"
#include "../lib-header/pit.h"
#include "../lib-header/resource.h"

// Double buffering to avoid screen tearing
// Might cause the kernel to get too big
graphics_info graphics = {
    .width = 800,
    .height = 600,
    .color_depth = 32,
    .buffer = 0,
    .size = 0
};

extern Resource resource_table[RESOURCE_AMOUNT];

void vesa_write_register(uint16_t index, uint16_t data){
    out2(VBE_DISPI_IOPORT_INDEX, index);
    out2(VBE_DISPI_IOPORT_DATA, data);
}
 
uint16_t vesa_read_register(uint16_t index){
    out2(VBE_DISPI_IOPORT_INDEX, index);
    return in2(VBE_DISPI_IOPORT_DATA);
}

void graphics_initialize(uint32_t width, uint32_t height, uint32_t colorbits, uint8_t linear_framebuffer, uint8_t clear){
    graphics.width = width;
    graphics.height = height;
    graphics.color_depth = colorbits;
    graphics.size = width * height * (colorbits / 8);
    graphics.buffer = (uint32_t*) kmalloc (sizeof(uint8_t) * graphics.size);
    
    vesa_write_register(VBE_DISPI_INDEX_ENABLE, 0);
    vesa_write_register(VBE_DISPI_INDEX_XRES, width);
    vesa_write_register(VBE_DISPI_INDEX_YRES, height);
    vesa_write_register(VBE_DISPI_INDEX_BPP, colorbits);
    vesa_write_register(VBE_DISPI_INDEX_ENABLE, 1 |
        (linear_framebuffer ? VBE_DISPI_LFB_ENABLED : 0) |
        (clear ? 0 : VBE_DISPI_NOCLEARMEM));

    PageDirectoryEntryFlag flags ={
        .present_bit       = 1,
        .user_supervisor   = 1,
        .write_bit         = 1,
        .use_pagesize_4_mb = 1
    };
    for (uint32_t i = 0; i < 4; i++){
        paging_dir_update((void*) VBE_DISPI_LFB_PHYSICAL_ADDRESS + (i * PAGE_FRAME_SIZE), (void*) VBE_DISPI_LFB_PHYSICAL_ADDRESS + (i * PAGE_FRAME_SIZE), flags, &_paging_kernel_page_directory);
    }
}

void graphics_clear(){
    memset((void*) graphics.buffer, DEFAULT_COLOR_BG, graphics.size);
}

void graphics_display(){
    memcpy((void*) VBE_DISPI_LFB_PHYSICAL_ADDRESS, (void*) graphics.buffer, graphics.size);
}

void graphics_set(uint16_t row, uint16_t col, uint32_t color){
    memcpy((void*) &graphics.buffer[(graphics.width * row) + col], &color, 4);
}
