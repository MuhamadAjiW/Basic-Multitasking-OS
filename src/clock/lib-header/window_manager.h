
#ifndef _WINMNGR_H
#define _WINMNGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_WINDOW_NUM 64
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

/**
 * Window_info, Information of a graphical window
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
 * window_init, allocates buffers of a window
 * 
 * @param winfo   Window to be initialized
 * @warning Initializing a window twice will cause memory leaks
 */
void window_init(struct window_info* winfo);

/**
 * window_clear, frees buffers of a window
 * 
 * @param winfo   Window to be cleared
 */
void window_clear(struct window_info* winfo);

/**
 * window_write, writes a character in the window's grid
 * 
 * @param winfo   Window to be written to
 * @param row     Y coordinate of the character to be written
 * @param col     X coordinate of the character to be written
 * @param c       Character to be written
 * @param fg      Foreground color
 * @param bg      Background color
 */
void window_write(struct window_info* winfo, uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg);

/**
 * window_register, registers a window to the window manager
 * 
 * @param winfo   Window to be registered
 * @warning Registering a window twice will potentially break the system
 */
void window_register(struct window_info* winfo);

/**
 * window_update, updates the current window on the screen, also refreshes the screen
 * 
 * @param winfo   Window to be updated
 * @attention Operation to update the current window and refreshing the screen is expensive. Minimize the use of this function whenever possible.
 */
void window_update(struct window_info* winfo);

/**
 * window_close, closes a window in the window manager
 * 
 * @param winfo   Window to be closed
 * @attention Closing a window does not deallocate the buffers, hence you can re-register it to show it again
 * @warning Closing a window twice will potentially break the system
 */
void window_close(struct window_info* winfo);

#endif
