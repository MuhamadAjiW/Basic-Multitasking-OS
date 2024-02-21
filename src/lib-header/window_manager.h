
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_WINDOW_NUM 64

/**
 * window_info, Information of a graphical window
 * 
 * @param main_buffer   Actual window's frame buffer, what is shown to the screen
 * @param rear_buffer   Framebuffer of whatever's behind the window, for performance reasons
 * @param xloc          Horizontal location of a window
 * @param xlen          Horizontal length of a window
 * @param yloc          Vertical location of a window
 * @param ylen          Vertical length of a window
 * @param id            Designated id of a window for window managing purposes, given when a window is registered
 * @param pid           pid of the window's process for window managing purposes
 * @param active        Whether the window is on focus or not, for window managing purposes
 */
struct window_info{
    uint16_t* main_buffer;
    uint16_t* rear_buffer;
    uint16_t  xloc;
    uint16_t  xlen;
    uint16_t  yloc;
    uint16_t  ylen;
    uint8_t   id;
    uint8_t   pid;
    uint8_t   active;       // Activeness not implemented yet
};

/**
 * window_stack, Interface for stack of windows managed by the window manager
 * 
 * @param ids   Array of ids on the stack
 * @param top   Index of the current top of the stack
 */
struct window_stack{
    uint8_t ids[MAX_WINDOW_NUM];
    uint8_t top;
};

/**
 * window_manager, Information stored for purposes of a window manager
 * 
 * @param windows       Actual information of windows, a window is saved whenever a window is updated
 * @param windows_ref   Reference of a window in its process's own virtual space
 * @param windows_exist Array of bool representing whether an id is already taken or not
 * @param stack         Stack information of windows 
 * @param window_active Id of the currently active window
 */
struct window_manager{
    struct window_info windows[MAX_WINDOW_NUM];
    struct window_info* windows_ref[MAX_WINDOW_NUM];
    bool windows_exist[MAX_WINDOW_NUM];
    struct window_stack stack;
    uint8_t window_active;
};



//Blackbox local functions
/**
 * winmgr_initialize, Initializes the window manager by setting its stack to -1
 */
void winmgr_initalilze();

/**
 * winmgr_set_window, Sets the buffer of a window with a certain value
 * 
 * @param winfo         Window to be written to
 * @param row           Y coordinate of buffer to be written
 * @param col           X coordinate of buffer to be written
 * @param info          information of word to be written to the buffer
 * @param main_buffer   Whether the targeted buffer is the main_buffer or the rear_buffer, true means main_buffer
 */
void winmgr_set_window(struct window_info* winfo, uint8_t row, uint8_t col, uint16_t info, bool main_buffer);

/**
 * winmgr_generate_window_id, Generates an id for a window
 * 
 * @return generated id to be assigned to a window
 */
uint8_t winmgr_generate_window_id();

/**
 * winmgr_show_window, Shows a window in the display
 * 
 * @param winfo         Window to be shown
 */
void winmgr_show_window(struct window_info* winfo);

/**
 * winmgr_hide_window, Hides a window from the display
 * 
 * @param winfo         Window to be hidden
 */
void winmgr_hide_window(struct window_info winfo);

/**
 * winmgr_update_winfo, Updates the window information saved by an id with a new window information
 * 
 * @param winfo         Window information to be saved
 * @param id            Id of the saved window
 */
void winmgr_update_winfo(struct window_info winfo, uint8_t id);

// Stacking mechanism
/**
 * winmgr_stack_remove, Removes a window from the window stack
 * 
 * @param id            Id of the window
 */
void winmgr_stack_remove(uint16_t id);

/**
 * winmgr_stack_add, Adds a window to the window stack
 * 
 * @param id            Id of the window
 */
void winmgr_stack_add(uint16_t id);

// These are for syscalls
/**
 * winmgr_register_winfo, Registers the window to the window manager, gives it an id and saves the information
 * 
 * @param winfo         Window information to be registered
 */
void winmgr_register_winfo(struct window_info* winfo);

/**
 * winmgr_register_window, Registers the window to the window manager, gives it id and assigns the pid 
 * 
 * @param winfo         Window information to be registered
 */
void winmgr_update_window(struct window_info* winfo);

/**
 * winmgr_close_window, Closes the window in the window manager, also frees the id 
 * 
 * @param id            Id of the window to be closed
 */
void winmgr_close_window(uint16_t id);

/**
 * winmgr_clean_window, Cleans the window of a killed process 
 * 
 * @param pid           Pid of a killed process
 */
void winmgr_clean_window(uint16_t pid);

#endif
