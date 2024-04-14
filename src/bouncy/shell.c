#include "lib-header/stdlib.h"
#include "lib-header/syscall.h"
#include "lib-header/window_manager.h"
#include "lib-header/cmos.h"

const uint8_t window_size = 2;

struct window_info winfo = {
    .main_buffer = (uint16_t*) 1,
    .xloc = 1,
    .yloc = 1,
    .xlen = window_size,
    .ylen = window_size
};

uint32_t randomizer(uint32_t seed) {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

int main(void) {
    uint32_t seed;
    syscall(SYSCALL_GET_TICK, (uint32_t) &seed, 0, 0);

    struct cmos_reader cmos = get_time();
    seed = seed + cmos.second + cmos.minute + cmos.hour + cmos.day + cmos.month + cmos.century;
    
    seed = randomizer(seed);
    winfo.xloc = (seed % 77) + 1;

    seed = randomizer(seed);
    winfo.yloc = (seed % 22) + 1;
    
    seed = randomizer(seed);
    uint8_t bgcolor = seed & 0xf;
    
    seed = randomizer(seed);
    uint8_t x_direction = seed & 1;

    seed = randomizer(seed);
    uint8_t y_direction = seed & 1;

    seed = randomizer(seed);
    uint8_t speed = (seed % 90) + 10;
    
    window_init(&winfo);

    window_write(&winfo, 0, 0, 'X', 0xf, bgcolor);
    window_write(&winfo, 0, 1, 'Y', 0xf, bgcolor);
    window_write(&winfo, 1, 0, 'Y', 0xf, bgcolor);
    window_write(&winfo, 1, 1, 'X', 0xf, bgcolor);

    window_register(&winfo);
    
    int8_t movx = x_direction? 1 : -1;
    int8_t movy = y_direction? 1 : -1;

    uint32_t running_time = 0;
    uint32_t time_limit = (seed % 512) + 200;

    while (++running_time < time_limit){
        winfo.xloc += movx;
        if(winfo.xloc == SCREEN_WIDTH - window_size || winfo.xloc == 0){
            movx = (-1) * movx;
        }

        winfo.yloc += movy;
        if(winfo.yloc == SCREEN_HEIGHT - window_size || winfo.yloc == 0){
            movy = (-1) * movy;
        }

        window_update(&winfo);

        delay(speed);
    }

    // exit();

    return 0;
}
