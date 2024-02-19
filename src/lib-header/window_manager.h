
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_WINDOW_NUM 64

//TODO: Document
struct window_info{
    uint16_t* mainBuffer;
    uint16_t* rearBuffer;
    uint16_t  xloc;
    uint16_t  xlen;
    uint16_t  yloc;
    uint16_t  ylen;
    uint8_t   id;
    uint8_t   pid;
    uint8_t   active;       // Activeness not implemented yet
};

struct window_stack{
    uint8_t ids[MAX_WINDOW_NUM];
    uint8_t top;
};

struct window_manager{
    struct window_info windows[MAX_WINDOW_NUM];
    struct window_info* windows_ref[MAX_WINDOW_NUM];
    bool windows_exist[MAX_WINDOW_NUM];
    struct window_stack stack;
    uint8_t window_active;
};



//Blackbox local functions
void winmgr_initalilze();
void winmgr_set_window(struct window_info* winfo, uint8_t row, uint8_t col, uint16_t info, bool mainBuffer);
uint8_t winmgr_generate_window_id();
void winmgr_show_window(struct window_info* winfo);
void winmgr_hide_window(struct window_info winfo);
void winmgr_update_winfo(struct window_info winfo, uint8_t id);

// Stacking mechanism
void winmgr_stack_remove(uint16_t id);
void winmgr_stack_add(uint16_t id);

// These are for syscalls
void winmgr_register_winfo(struct window_info* winfo);
void winmgr_update_window(struct window_info* winfo);
void winmgr_close_window(uint16_t id);
void winmgr_clean_window(uint16_t pid);

#endif
