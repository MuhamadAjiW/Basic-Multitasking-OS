#include "lib-header/stdtype.h"
#include "lib-header/syscall.h"
#include "lib-header/stdio.h"
#include "lib-header/stdlib.h"
#include "lib-header/stdmem.h"
#include "lib-header/window_manager.h"

#include "lib-header/shell.h"

// const uint8_t window_size = 2;

shell_app sh = {
    .default_font_color = 0x0,
    .default_background_color = 0xe,
    .cursor_x = 0,
    .cursor_y = 0,
    .cursor_x_limit = 0,
    .cursor_y_limit = 0,
    .cursor_on = 1,

    .winfo = {
        .mainBuffer = (uint16_t*) 1,
        .xloc = 0,
        .yloc = 0,
        .xlen = SCREEN_WIDTH,
        .ylen = SCREEN_HEIGHT - 1
    },

    .reader = {
        .buffer_addr = (char*) 1,
        .buffer_size = INPUT_BUFFER_SIZE,
        .max_idx = 0,
        .current_idx = 0
    },

    .grid = {0},

    .background = 0
};

// App
void app_initialize(){
    sh.dir.cluster_number = ROOT_CLUSTER_NUMBER;
    sh.dir.path = (char*) malloc (sizeof(char) * INPUT_BUFFER_SIZE);
    memcpy(sh.dir.path, "/root", 6);

    window_init(&(sh.winfo));
    grid_initialize();
    app_load_background();
    app_draw_background();
    window_register(&(sh.winfo));
    reader_initialize();

    shell_newline();
}
void app_load_background(){
    // TODO: Switch to a better background structure
    // Generate simple background
    uint32_t size = sh.grid.xlen * sh.grid.ylen;
    sh.background = (uint8_t*) malloc (sizeof(uint8_t) * size);
    // uint8_t default_value = (sh.default_background_color << 4) | (sh.default_font_color & 0xf);
    uint8_t default_value = (0x8 << 4) | (sh.default_font_color & 0xf);
    for(uint32_t i = 0; i < size; i++){
        sh.background[i] = default_value;
    }
}
void app_draw_background(){
    // TODO: Switch to a better background structure
    // Set background to window
    for (uint16_t r = 0; r < sh.winfo.ylen; r++){
        for (uint16_t c = 0; c < sh.winfo.xlen; c++){
            uint32_t bg_offset = r * sh.winfo.ylen + c;
            window_write(&(sh.winfo), r, c, 0, sh.background[bg_offset], sh.background[bg_offset] >> 4);
        }
    }
}

// Grid
void grid_initialize(){
    sh.grid.xlen = sh.winfo.xlen;
    sh.grid.ylen = sh.winfo.ylen;
    uint32_t size = sh.grid.xlen * sh.grid.ylen;
    sh.grid.char_map = (char*) malloc (sizeof(char) * size);
    sh.grid.char_color_map = (char*) malloc (sizeof(char) * size);
}
void grid_write(){
    app_draw_background();
    for (uint16_t r = 0; r < sh.grid.ylen; r++){
        for (uint16_t c = 0; c < sh.grid.xlen; c++){
            uint32_t grid_offset = r * sh.winfo.xlen + c;
            if(sh.grid.char_map[grid_offset] != 0){
                window_write(&(sh.winfo), r, c, sh.grid.char_map[grid_offset], sh.grid.char_color_map[grid_offset] & 0xf, sh.grid.char_color_map[grid_offset] >> 4);
            }
        }
    }
    window_update(&(sh.winfo));
}

// Cursor
uint16_t cursor_get_y(){
    return sh.cursor_y;
}
uint16_t cursor_get_x(){
    return sh.cursor_x;
}

void cursor_on(){
    sh.cursor_on = 1;
    sys_cursor_set_active(sh.cursor_on);
}

void cursor_off(){
    sh.cursor_on = 0;
    sys_cursor_set_active(sh.cursor_on);
}

void cursor_limit(uint8_t x, uint8_t y){
    sh.cursor_x_limit = x;
    sh.cursor_y_limit = y;
}

void cursor_set(uint8_t x, uint8_t y){
    if(sh.cursor_on){
        sh.cursor_x = x;
        sh.cursor_y = y;
        sys_cursor_set_location(y, x);
    }
}

uint16_t cursor_find_edge(uint16_t y){
    uint16_t counter = 0;
    uint32_t baseline = y * sh.grid.xlen;
    while (sh.grid.char_map[baseline + counter] != 0 && sh.grid.char_map[baseline + counter] != '\n' && counter < sh.grid.xlen){
        counter++;
    }
    if(counter == sh.grid.xlen) counter--;
    
    return counter;
}

int32_t cursor_move(int8_t direction){
    int32_t dist = 0;
    uint32_t cursor_overall_position = sh.cursor_y * sh.grid.xlen + sh.cursor_x;
    if (sh.cursor_on){
        switch (direction){
            case 1:
                if((sh.cursor_x == sh.grid.xlen - 1 && sh.grid.char_map[cursor_overall_position] != 0)){
                    if (sh.cursor_y != sh.grid.ylen - 1){
                        dist = 1;
                        cursor_set(0, sh.cursor_y + 1);
                    }
                }
                else{
                    if(sh.grid.char_map[cursor_overall_position] == '\n'){
                        if(sh.cursor_y != sh.grid.ylen - 1){
                            dist = 1;
                            cursor_set(0, sh.cursor_y + 1);
                        }
                    }
                    else if(sh.grid.char_map[cursor_overall_position] != 0){
                        dist = 1;
                        cursor_set(sh.cursor_x + 1, sh.cursor_y);
                    }
                }
                break;
            case -1:
                if(!(sh.cursor_y == sh.cursor_y_limit && sh.cursor_x == sh.cursor_x_limit)){
                    if(sh.cursor_x != 0 ){
                        dist = -1;
                        cursor_set(sh.cursor_x - 1, sh.cursor_y);
                    }
                    else{
                        if(sh.cursor_y != 0){
                            dist = -1;
                            uint8_t edge = cursor_find_edge(sh.cursor_y - 1);
                            if(sh.grid.char_map[(sh.cursor_y - 1) * sh.grid.xlen + edge - 1] == '\n') cursor_set(edge - 1, sh.cursor_y - 1);
                            else cursor_set(edge, sh.cursor_y - 1);
                        }
                    }
                }
                break;
            
            default:
                break;
        }
    }

    return dist;
}

// Reader
void reader_initialize(){
    sh.reader.buffer_size = INPUT_BUFFER_SIZE;
    sh.reader.buffer_addr = (char*) malloc (sizeof(char) * INPUT_BUFFER_SIZE);
    sh.reader.buffer_addr[0] = 0;
    sh.reader.current_idx = 0;
    sh.reader.max_idx = 0;
}

void reader_clear(){
    sh.reader.buffer_size = INPUT_BUFFER_SIZE;
    free(sh.reader.buffer_addr);
    sh.reader.buffer_addr = (char*) malloc (sizeof(char) * INPUT_BUFFER_SIZE);
    sh.reader.buffer_addr[0] = 0;
    sh.reader.current_idx = 0;
    sh.reader.max_idx = 0;
}

void reader_insert(char c){
    if(sh.reader.max_idx < sh.reader.buffer_size - 1){
        for(uint32_t i = sh.reader.max_idx; i > sh.reader.current_idx; i--){
            sh.reader.buffer_addr[i] = sh.reader.buffer_addr[i - 1];
        }

        sh.reader.buffer_addr[sh.reader.current_idx] = c;
        sh.reader.max_idx++;
        sh.reader.current_idx++;
    }
    else{
        sh.reader.buffer_size += INPUT_BUFFER_SIZE;
        sh.reader.buffer_addr = (char*) realloc (sh.reader.buffer_addr, sizeof(char) * sh.reader.buffer_size);
        reader_insert(c);
    }
}

void reader_move(int8_t direction){
    sh.reader.current_idx += direction;
}

// void reader_backspace(){

// }

// // Shell functionalities
void shell_clear(){
    
}

void shell_newline(){
    print("\n");
    print(sh.dir.path);
    print("> ");
    cursor_limit(sh.cursor_x, sh.cursor_y);
}

void shell_evaluate(){

}

int main(void) {
    // TODO: Fix
    // I may have exited the init process improperly 
    // therefore, a process has to start with a syscall
    // else there wouldn't be any interrupt happening
    syscall(SYSCALL_NULL, 0, 0, 0);

    app_initialize();
    
    char buf[2] = {0};
    while (TRUE){
        buf[0] = sys_keyboard_get_char();

        switch (buf[0]){
            case TAB_CHAR:
                reader_insert(' ');
                reader_insert(' ');
                reader_insert(' ');
                reader_insert(' ');
                print("    ");
                break;

            case LARROW_CHAR:
                if(cursor_move(-1)){
                    reader_move(-1);
                }
                break;

            case RARROW_CHAR:
                if(cursor_move(1)){
                    reader_move(1);
                }
                break;
            
            // case BACKSPACE_CHAR:
            //     backspace();
            //     break;

            case '\n':
                shell_evaluate();
                shell_clear();
                shell_newline();
                break;
            
            default:
                if(buf[0] >= 32 && buf[0] <= 126){
                    reader_insert(buf[0]);
                    print(buf);
                }
                break;
        }
    }

    return 0;
}


