#include "lib-header/stdtype.h"
#include "lib-header/stdlib.h"
#include "lib-header/syscall.h"
#include "lib-header/window_manager.h"
#include "lib-header/shell.h"
#include "lib-header/cmos.h"

shell_app sh = {
    .default_font_color = 0xf,
    .default_background_color = 0x0,
    .winfo = {0},
    .grid = {0},
    .background = (uint8_t*) 0
};

cmos_reader time_current = {0};
uint8_t bgcolor = 0x0;
uint8_t fgcolor = 0xf;

void write_time(){
    uint8_t hour = time_current.hour;
    sh.grid.char_map[0] = '0' + hour / 10;
    sh.grid.char_map[1] = '0' + hour % 10;
    uint8_t minute = time_current.minute;
    sh.grid.char_map[3] = '0' + minute / 10;
    sh.grid.char_map[4] = '0' + minute % 10;
    uint8_t second = time_current.second;
    sh.grid.char_map[6] = '0' + second / 10;
    sh.grid.char_map[7] = '0' + second % 10;

    grid_write();
}

void app_initialize(){
    font_load(&(sh.finfo), "system/stdfont.fnt", ROOT_CLUSTER_NUMBER);

    sh.winfo.xlen = 8 * sh.finfo.width;
    sh.winfo.ylen = 1 * sh.finfo.height;
    sh.winfo.xloc = SCREEN_WIDTH - sh.winfo.xlen;
    sh.winfo.yloc = SCREEN_HEIGHT - sh.winfo.ylen;

    grid_initialize();
    
    window_init(&(sh.winfo));

    app_load_background();
    app_draw_background();
    sh.grid.char_map[2] = ':';
    sh.grid.char_map[5] = ':';


    window_register(&(sh.winfo));
    write_time();
}

void app_load_background(){
    // Generate simple background
    uint32_t size = sh.winfo.xlen * sh.winfo.ylen;
    sh.background = (uint8_t*) malloc (sizeof(uint8_t) * size);
    for(uint32_t i = 0; i < size; i++){
        sh.background[i] = sh.default_background_color;
    }
}

void app_draw_background(){
    for (uint16_t r = 0; r < sh.winfo.ylen; r++){
        for (uint16_t c = 0; c < sh.winfo.xlen; c++){
            uint32_t bg_offset = r * sh.winfo.ylen + c;
            window_draw_pixel(&(sh.winfo), r, c, sh.background[bg_offset]);
        }
    }
}

// Grid
void grid_initialize(){
    sh.grid.xlen = sh.winfo.xlen / sh.finfo.width;
    sh.grid.ylen = sh.winfo.ylen / sh.finfo.height;
    uint32_t size = sh.grid.xlen * sh.grid.ylen;
    sh.grid.char_map = (char*) malloc (sizeof(char) * size);
}

void grid_write(){
    app_draw_background(&(sh.winfo), sh.background);
    for (uint16_t r = 0; r < sh.grid.ylen; r++){
        for (uint16_t c = 0; c < sh.grid.xlen; c++){
            uint32_t grid_offset = r * sh.grid.xlen + c;
            if(sh.grid.char_map[grid_offset] != 0){
                font_write(&(sh.winfo), sh.finfo, r, c, sh.grid.char_map[grid_offset], sh.default_font_color);
            }
        }
    }
    window_update(&(sh.winfo));
}

int main(void) {
    app_initialize();

    cmos_reader time_new = get_time();
    time_current = time_new;


    while (TRUE){
        time_new = get_time();
        if (time_current.second != time_new.second){
            time_current = time_new;
            write_time();
            window_update(&(sh.winfo));
        }
        
    }

    exit();
    
    return 0;
}
