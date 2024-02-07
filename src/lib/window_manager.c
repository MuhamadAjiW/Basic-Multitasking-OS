
#include "../lib-header/stdtype.h"
#include "../lib-header/graphics.h"
// #include "../lib-header/framebuffer.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/task.h"

// TODO: Document
// Note: Would be interesting to make this a separate program if we have inter-process communication 
window_manager winmgr = {0};
extern uint8_t screen_buffer[];
extern PCB* current_task;

void winmgr_initalilze(){
    // The stack have to be initialized with -1 or 255 since 0 is used as an id
    for (uint8_t i = 0; i < MAX_WINDOW_NUM; i++){
        winmgr.stack.ids[i] = -1;
    }
}
void winmgr_set_window(window_info* winfo, uint8_t row, uint8_t col, uint16_t info, bool main_buffer){    
    volatile uint8_t * where = main_buffer? winfo->main_buffer : winfo->rear_buffer;
    where +=  (row * winfo->xlen + col);
    *where = info;
}
uint8_t winmgr_generate_window_id(){
    uint8_t given_id = 0;
    while (given_id < MAX_WINDOW_NUM && winmgr.windows_exist[given_id] == TRUE){
        given_id++;
    }

    if(given_id == MAX_WINDOW_NUM) return -1; // Window manager is full

    winmgr.windows_exist[given_id] = 1;
    return given_id;
}
void winmgr_show_window(window_info* winfo){
    uint16_t j = 0;
    uint16_t xloc = winfo->xloc;
    uint16_t xlen = winfo->xlen;
    uint16_t yloc = winfo->yloc;
    uint16_t ylen = winfo->ylen;

    while (yloc + j < SCREEN_HEIGHT && j < ylen){
        uint16_t i = 0;
        while (xloc + i < SCREEN_WIDTH && i < xlen){
            winmgr_set_window(winfo, j, i, screen_buffer[(SCREEN_WIDTH * (yloc + j)) + (xloc + i)], FALSE);
            graphics_set(yloc + j, xloc + i, winfo->main_buffer[(xlen * j) + i]);
            i++;
        }
        j++;
    }
}
void winmgr_show_window_flat(window_info winfo){
    uint16_t j = 0;
    while (winfo.yloc + j < SCREEN_HEIGHT && j < winfo.ylen){
        uint16_t i = 0;
        while (winfo.xloc + i < SCREEN_WIDTH && i < winfo.xlen){
            graphics_set(winfo.yloc + j, winfo.xloc + i, winfo.main_buffer[(winfo.xlen * j) + i]);
            i++;
        }
        j++;
    }
}
void winmgr_hide_window(window_info winfo){
    uint16_t j = 0;
    while (winfo.yloc + j < SCREEN_HEIGHT && j < winfo.ylen){
        uint16_t i = 0;
        while (winfo.xloc + i < SCREEN_WIDTH && i < winfo.xlen){
            graphics_set(winfo.yloc + j, winfo.xloc + i, winfo.rear_buffer[(winfo.xlen * j) + i]);
            i++;
        }
        j++;
    }
}
void winmgr_update_winfo(window_info winfo, uint8_t id){
    winmgr.windows[id].xloc = winfo.xloc;
    winmgr.windows[id].xlen = winfo.xlen;
    winmgr.windows[id].yloc = winfo.yloc;
    winmgr.windows[id].ylen = winfo.ylen;
    winmgr.windows[id].active = winfo.active;
}
void winmgr_stack_remove(uint16_t id){
    // TODO: Improve, rendering everything each time would be quite heavy
    // There might be a way to do this without re-rendering the whole stack

    int8_t i = 0;
    while (i != MAX_WINDOW_NUM && winmgr.stack.ids[i]!= id){
        i++;
    }
    if (i == MAX_WINDOW_NUM) return; // id not found, window might be an unregistered 

    for (int8_t j = winmgr.stack.top - 1; j >= i; j--){
        winmgr_hide_window(winmgr.windows[winmgr.stack.ids[j]]);
    }

    winmgr.stack.top--;
    while (i < winmgr.stack.top){
        winmgr.stack.ids[i] = winmgr.stack.ids[i+1];
        winmgr_show_window(&winmgr.windows[winmgr.stack.ids[i]]);
        i++;
    }
}
void winmgr_stack_add(uint16_t id){
    if (winmgr.stack.top == MAX_WINDOW_NUM) return; // Stack is full
    
    winmgr.stack.ids[winmgr.stack.top] = id;
    winmgr.stack.top++;
    winmgr_show_window(&winmgr.windows[id]);
}

// Syscall functions
void winmgr_register_winfo(window_info* winfo){
    uint8_t id = winmgr_generate_window_id();
    winfo->id = id;
    winmgr.windows_ref[id] = winfo;
    winmgr.windows[id].main_buffer = winfo->main_buffer;
    winmgr.windows[id].rear_buffer = winfo->rear_buffer;
    winmgr.windows[id].pid = current_task->pid;
    winmgr.windows[id].id = id;
    winmgr_update_winfo(*winfo, id);
    winmgr_stack_add(id);

    graphics_display();
}
void winmgr_update_window(window_info* winfo){
    uint8_t id = winfo->id;

    if(winmgr.windows[id].main_buffer == 0) return; //Unregistered window requested

    // TODO: Implement active window mechanism
    if(winfo->active){
        winmgr_stack_remove(id);
        winmgr_update_winfo(*winfo, id);
        winmgr_stack_add(id);

        for (int8_t j = 0; j < MAX_WINDOW_NUM; j++){
            if(winmgr.windows_exist[j] && j != id){
                winmgr.windows_ref[j]->active = FALSE;
            }
        }
    }
    else{
        int8_t i = 0;
        while (i != MAX_WINDOW_NUM && winmgr.stack.ids[i]!= id){
            i++;
        }
        if (i == MAX_WINDOW_NUM) return; // id not found, window might be an unregistered 

        for (int8_t j = winmgr.stack.top - 1; j >= i; j--){
            winmgr_hide_window(winmgr.windows[winmgr.stack.ids[j]]);
        }

        winmgr_update_winfo(*winfo, id);
        while (i < winmgr.stack.top){
            winmgr_show_window(&winmgr.windows[winmgr.stack.ids[i]]);
            i++;
        }
    }

    graphics_display();
}
void winmgr_close_window(uint16_t id){
    if(winmgr.windows[id].main_buffer == 0) return; //Unregistered window requested

    winmgr_stack_remove(id);
    winmgr.windows_exist[id] = 0;

    graphics_display();
    // TODO: Implement active window mechanism
}

void winmgr_clean_window(uint16_t pid){
    for (uint32_t i = 0; i < winmgr.stack.top; i++){
        uint32_t id = winmgr.stack.ids[i];
        if(winmgr.windows[id].pid == pid) {
            winmgr_close_window(id);
        }
    }
}