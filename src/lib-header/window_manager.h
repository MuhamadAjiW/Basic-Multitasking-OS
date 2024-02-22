
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include "stdtype.h"

#define MAX_WINDOW_NUM 64

//TODO: Document
typedef struct window_info{
    uint32_t* main_buffer;
    uint32_t* rear_buffer;
    uint16_t  xloc;
    uint16_t  xlen;
    uint16_t  yloc;
    uint16_t  ylen;
    uint8_t   id;
    uint8_t   pid;
    uint8_t   active;       // Activeness not implemented yet
} window_info;

typedef struct window_stack{
    uint8_t ids[MAX_WINDOW_NUM];
    uint8_t top;
} window_stack;

typedef struct window_manager{
    window_info windows[MAX_WINDOW_NUM];
    window_info* windows_ref[MAX_WINDOW_NUM];
    bool windows_exist[MAX_WINDOW_NUM];
    window_stack stack;
    uint8_t window_active;
} window_manager;



//Blackbox local functions
void winmgr_initalilze();
void winmgr_set_window(window_info* winfo, uint8_t row, uint8_t col, uint16_t info, bool main_buffer);
uint8_t winmgr_generate_window_id();
void winmgr_show_window(window_info* winfo);
void winmgr_hide_window(window_info winfo);
void winmgr_update_winfo(window_info winfo, uint8_t id);

// Stacking mechanism
void winmgr_stack_remove(uint16_t id);
void winmgr_stack_add(uint16_t id);

// These are for syscalls
void winmgr_register_winfo(window_info* winfo);
void winmgr_update_window(window_info* winfo);
void winmgr_close_window(uint16_t id);
void winmgr_clean_window(uint16_t pid);

#endif
