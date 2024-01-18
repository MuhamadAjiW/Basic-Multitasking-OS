
#include "../lib-header/stdtype.h"
#include "../lib-header/framebuffer.h"
#include "../lib-header/window_manager.h"

//TODO: Document
window_manager winmgr = {0};
extern uint16_t screen_buffer[];


void winmgr_initalilze(){
    // The stack have to be initialized with -1 or 255 since 0 is used as an id
    for (uint8_t i = 0; i < MAX_WINDOW_NUM; i++){
        winmgr.stack.ids[i] = -1;
    }
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
    while (winfo->yloc + j < SCREEN_HEIGHT && j < winfo->ylen){
        uint16_t i = 0;
        while (winfo->xloc + i < SCREEN_WIDTH && i < winfo->xlen){
            framebuffer_set(winfo->xloc + i, winfo->yloc + j, winfo->mainBuffer[(winfo->xlen * j) + i], screen_buffer);
            framebuffer_set(winfo->xloc + i, winfo->yloc + j, screen_buffer[(winfo->xlen * j) + i], winfo->rearBuffer);
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
            framebuffer_set(winfo.xloc + i, winfo.yloc + j, winfo.mainBuffer[(winfo.xlen * j) + i], screen_buffer);
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
            framebuffer_set(winfo.xloc + i, winfo.yloc + j, winfo.rearBuffer[(winfo.xlen * j) + i], screen_buffer);
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
    // There might be a way to do this without re-rendering everything

    uint8_t i = 0;
    while (i != MAX_WINDOW_NUM && winmgr.stack.ids[i]!= id){
        i++;
    }
    if (i == MAX_WINDOW_NUM) return; // id not found, window might be an unregistered 

    for (uint8_t j = winmgr.stack.top - 1; j >= i; i--){
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
void winmgr_stack_put_forward(uint16_t id){
    winmgr_stack_remove(id);
    winmgr_stack_add(id);
}




// Syscall functions
void winmgr_register_winfo(window_info* winfo){
    uint8_t id = winmgr_generate_window_id();
    winfo->id = id;
    winmgr.windows_ref[id] = winfo;
    winmgr.windows[id].mainBuffer = winfo->mainBuffer;
    winmgr.windows[id].rearBuffer = winfo->rearBuffer;
    winmgr_update_winfo(*winfo, id);
    winmgr_stack_add(id);
}
void winmgr_update_window(window_info* winfo){
    if(winmgr.windows[winfo->id].mainBuffer == 0) return; //Unregistered window requested

    // TODO: Implement active window mechanism
    if(winfo->active){
        winmgr_stack_put_forward(winfo->id);

        for (uint8_t j = 0; j < MAX_WINDOW_NUM; j++){
            if(winmgr.windows_exist[j] && j != winfo->id){
                winmgr.windows_ref[j]->active = FALSE;
            }
        }
    }
    else{
        uint8_t i = 0;
        while (i != MAX_WINDOW_NUM && winmgr.stack.ids[i]!= winfo->id){
            i++;
        }
        if (i == MAX_WINDOW_NUM) return; // id not found, window might be an unregistered 

        for (uint8_t j = winmgr.stack.top - 1; j > i; i--){
            winmgr_hide_window(winmgr.windows[winmgr.stack.ids[j]]);
        }

        while (i < winmgr.stack.top){
            winmgr_show_window(&winmgr.windows[winmgr.stack.ids[i]]);
            i++;
        }
    }

}
void winmgr_close_window(uint16_t id){
    if(winmgr.windows[id].mainBuffer == 0) return; //Unregistered window requested

    winmgr_stack_remove(id);
    winmgr.windows_exist[id] = 0;

    // TODO: Implement active window mechanism
}
