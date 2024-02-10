#include "lib-header/stdtype.h"
#include "lib-header/stdlib.h"
#include "lib-header/syscall.h"
#include "lib-header/window_manager.h"
#include "lib-header/cmos.h"

#define WINDOW_SIZE 10

window_info winfo = {
    .main_buffer = (uint8_t*) 1,
    .xloc = 1,
    .yloc = 1,
    .xlen = WINDOW_SIZE,
    .ylen = WINDOW_SIZE
};

uint32_t randomizer(uint32_t seed) {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

// TODO: Threading
// keknya asik kalo make threading buat bikin yang lompat lompatnya
int main(__attribute__((unused)) uint32_t argc, __attribute__((unused)) char** argv) {
    uint32_t seed;
    syscall(SYSCALL_GET_TICK, (uint32_t) &seed, 0, 0);

    cmos_reader cmos = get_time();
    seed = seed + cmos.second + cmos.minute + cmos.hour + cmos.day + cmos.month + cmos.century;
    
    seed = randomizer(seed);
    winfo.xloc = (seed % (SCREEN_WIDTH - WINDOW_SIZE - 2)) + 1;

    seed = randomizer(seed);
    winfo.yloc = (seed % (SCREEN_HEIGHT - WINDOW_SIZE - 2)) + 1;
    
    seed = randomizer(seed);
    uint8_t bgcolor = seed;
    
    seed = randomizer(seed);
    uint8_t x_direction = seed & 1;

    seed = randomizer(seed);
    uint8_t y_direction = seed & 1;

    seed = randomizer(seed);
    uint8_t speed = (seed % 10);
    
    window_init(&winfo);

    for (uint8_t i = 0; i < WINDOW_SIZE; i++){
        for (uint8_t j = 0; j < WINDOW_SIZE; j++){
            window_draw_pixel(&winfo, i, j, bgcolor);
        }
    }

    window_register(&winfo);
    window_update(&winfo);
    
    int8_t movx = x_direction? 1 : -1;
    int8_t movy = y_direction? 1 : -1;

    uint32_t running_time = 0;
    uint32_t time_limit = (seed % 512) + 200;

    while (++running_time < time_limit){
        winfo.xloc += movx;
        if(winfo.xloc == SCREEN_WIDTH - WINDOW_SIZE || winfo.xloc == 0){
            movx = (-1) * movx;
        }

        winfo.yloc += movy;
        if(winfo.yloc == SCREEN_HEIGHT - WINDOW_SIZE || winfo.yloc == 0){
            movy = (-1) * movy;
        }

        window_update(&winfo);

        delay(speed);
    }

    exit();

    return 0;
}
