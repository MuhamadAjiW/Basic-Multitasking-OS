#include "lib-header/stdlib.h"
#include "lib-header/syscall.h"
#include "lib-header/window_manager.h"
#include "lib-header/cmos.h"

struct window_info winfo = {
    .mainBuffer = (uint16_t*) 1,
    .xloc = SCREEN_WIDTH - 8,
    .yloc = SCREEN_HEIGHT - 1,
    .xlen = 8,
    .ylen = 1
};

struct cmos_reader time_current = {0};
uint8_t bgcolor = 0x0;
uint8_t fgcolor = 0xf;

void write_time(){
    uint8_t hour = time_current.hour;
    window_write(&winfo, 0, 0, '0' + hour / 10, fgcolor, bgcolor);
    window_write(&winfo, 0, 1, '0' + hour % 10, fgcolor, bgcolor);

    uint8_t minute = time_current.minute;
    window_write(&winfo, 0, 3, '0' + minute / 10, fgcolor, bgcolor);
    window_write(&winfo, 0, 4, '0' + minute % 10, fgcolor, bgcolor);
    
    uint8_t second = time_current.second;
    window_write(&winfo, 0, 6, '0' + second / 10, fgcolor, bgcolor);
    window_write(&winfo, 0, 7, '0' + second % 10, fgcolor, bgcolor);
}

int main(void) {
    window_init(&winfo);
    struct cmos_reader time_new = get_time();
    time_current = time_new;

    window_write(&winfo, 0, 2, ':', fgcolor, bgcolor);
    window_write(&winfo, 0, 5, ':', fgcolor, bgcolor);

    write_time();
    window_register(&winfo);

    while (true){
        time_new = get_time();
        if (time_current.second != time_new.second){
            time_current = time_new;
            write_time();
            window_update(&winfo);
        }
        
    }
    
    exit();

    return 0;
}
