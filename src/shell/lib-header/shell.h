

#ifndef _USER_SHELL
#define _USER_SHELL

#define INPUT_BUFFER_SIZE 1024
#define SHELL_INPUT_COLOR 84

#define NULL_CHAR 0
#define TAB_CHAR 1
#define LARROW_CHAR 2
#define RARROW_CHAR 3
#define UARROW_CHAR 4
#define DARROW_CHAR 5
#define ESC_CHAR 6
#define BACKSPACE_CHAR 7


#include "window_manager.h"

//----Structs

//TODO: Document
typedef struct shell_reader {
    char* buffer_addr;
    uint32_t buffer_size;
    uint32_t max_idx;
    uint32_t current_idx;
} shell_reader;

typedef struct directory_info {
    char* path;
    uint32_t cluster_number;
}__attribute__((packed)) directory_info;

typedef struct shell_app
{    
    uint8_t default_font_color;
    
    uint8_t cursor_x;
    uint8_t cursor_x_limit;
    uint8_t cursor_y;
    uint8_t cursor_y_limit;
    uint8_t cursor_on;
    
    window_info winfo;
    shell_reader reader;
    directory_info dir;

} shell_app;



//----App
void app_initialize();


//----Cursor
void cursor_on();
void cursor_off();
void cursor_limit(uint8_t x, uint8_t y);
uint8_t cursor_set(uint8_t x, uint8_t y);
int32_t cursor_move(int direction);


//----Reader
void reader_initialize();
void reader_clear();
void reader_append();
void reader_move();
void reader_backspace();


//----Shell functionalities
void shell_clear();
void shell_evaluate();

#endif