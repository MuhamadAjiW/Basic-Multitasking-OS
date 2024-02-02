#include "lib-header/stdtype.h"
#include "lib-header/syscall.h"
#include "lib-header/shell.h"
#include "lib-header/window_manager.h"

// const uint8_t window_size = 2;

// shell_app app = {
//     .default_font_color = 0,
//     .cursor_x = 0,
//     .cursor_y = 0,
//     .cursor_x_limit = 0,
//     .cursor_y_limit = 0,
//     .cursor_on = 1,

//     .winfo = {
//         .mainBuffer = (uint16_t*) 1,
//         .xloc = 0,
//         .yloc = 0,
//         .xlen = SCREEN_WIDTH,
//         .ylen = SCREEN_HEIGHT
//     },

//     .reader = {
//         .buffer_addr = (char*) 1,
//         .buffer_size = INPUT_BUFFER_SIZE,
//         .max_idx = 0,
//         .current_idx = 0
//     }
// };

// // App
// void app_initalize(){

// }


// // Cursor
// void cursor_on(){

// }

// void cursor_off(){

// }

// void cursor_limit(uint8_t x, uint8_t y){
//     app.cursor_x_limit = x;
//     app.cursor_y_limit = y;
// }

// uint8_t cursor_set(uint8_t x, uint8_t y){
//     return 0;
// }

// int32_t cursor_move(int8_t direction){
//     return 0;
// }

// // Reader
// void reader_initialize(){

// }

// void reader_clear(){

// }

// void reader_append(){

// }

// void reader_move(){

// }

// void reader_backspace(){

// }

// // Shell functionalities
// void shell_clear(){
    
// }

// void shell_evaluate(){

// }

int main(void) {
    // TODO: Fix
    // I may have exited the init process improperly 
    // therefore, a process has to start with a syscall
    // else there wouldn't be any interrupt happening
    syscall(SYSCALL_NULL, 0, 0, 0);
    
    // window_init(&(app.winfo));
    // window_register(&(app.winfo));
    
    while (TRUE){
    
    }

    return 0;
}


