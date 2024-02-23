

#ifndef _USER_SHELL
#define _USER_SHELL

#define INPUT_BUFFER_SIZE 1024

#define ROOT_CLUSTER_NUMBER 65

#include "window_manager.h"

//TODO: Document

//----Structs
struct shell_reader {
    char* buffer_addr;
    uint32_t buffer_size;
    uint32_t max_idx;
    uint32_t current_idx;
};

struct directory_info {
    char* path;
    uint32_t cluster_number;
}__attribute__((packed));

struct text_grid{
    char* char_map;
    char* char_color_map;
    uint16_t xlen;
    uint16_t ylen;
};

struct shell_app
{    
    uint8_t default_font_color;
    uint8_t default_background_color;
    
    uint16_t cursor_x;
    uint16_t cursor_x_limit;
    uint16_t cursor_y;
    uint16_t cursor_y_limit;
    uint8_t cursor_on;
    
    struct window_info winfo;
    struct shell_reader reader;
    struct directory_info dir;
    struct text_grid grid;

    // TODO: Switch to a better background structure
    uint8_t* background;

};



//----App
void app_initialize();
void app_load_background();
void app_draw_background();

//----Grid
void grid_initialize();
void grid_write();

//----Cursor
uint16_t cursor_get_y();
uint16_t cursor_get_x();
void cursor_on();
void cursor_off();
void cursor_limit(uint8_t x, uint8_t y);
void cursor_set(uint8_t x, uint8_t y);
uint16_t cursor_find_edge(uint16_t y);
int32_t cursor_move(int8_t direction);


//----Reader
void reader_initialize();
void reader_clear();
void reader_insert(char c);
void reader_move(int8_t direction);


//----Shell functionalities
void shell_clear();
void shell_backspace();
void shell_newline();
void shell_evaluate();
void shell_recent(int8_t direction);
void shell_reset();
void shell_recent_save();

#endif