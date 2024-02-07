

#ifndef _USER_SHELL
#define _USER_SHELL

#define INPUT_BUFFER_SIZE 1024

#define ROOT_CLUSTER_NUMBER 65

#include "window_manager.h"
#include "font.h"
#include "image.h"

//TODO: Document

//----Structs
typedef struct text_grid{
    char* char_map;
    uint16_t xlen;
    uint16_t ylen;
} text_grid;

typedef struct shell_app
{    
    uint8_t default_font_color;
    uint8_t default_background_color;
    uint8_t* background;
    window_info winfo;
    font_info finfo;
    text_grid grid;
} shell_app;

//----App
void app_initialize();
void app_load_background();
void app_draw_background();

//----Grid
void grid_initialize();
void grid_write();

#endif