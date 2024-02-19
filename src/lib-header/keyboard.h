
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

#include "interrupt.h"

#define KEYBOARD_DATA_PORT     0x60
#define EXTENDED_SCANCODE_BYTE 0xE0

#define NULL_CHAR 0
#define TAB_CHAR 1
#define LARROW_CHAR 2
#define RARROW_CHAR 3
#define UARROW_CHAR 4
#define DARROW_CHAR 5
#define ESC_CHAR 6
#define BACKSPACE_CHAR 7

/**
 * keyboard_scancode_1_to_ascii_map[256], Convert scancode values that correspond to ASCII printables
 * How to use this array: ascii_char = k[scancode]
 * 
 * By default, QEMU using scancode set 1 (from empirical testing)
 */
extern const char keyboard_scancode_1_to_ascii_map[256];

/**
 * KeyboardDriverState - Contain all driver states
 * 
 * @param buffer             Saves the last key pressed on the keyboard
 * @param caps               Indicate whether capslock is active
 * @param shigt              Indicate whether shift is active
 * @param on                 Indicate whether keyboard ISR is activated or not
 */
struct KeyboardDriverState {
    char    buffer;
    uint8_t caps;
    uint8_t shift;
    bool    on;
} __attribute((packed));





/* -- Driver Interfaces -- */

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void);

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void);

// Get keyboard buffer values - @param buf Pointer to char buffer
void keyboard_flush_buffer(char *buf);

// Check whether keyboard ISR is active or not - @return Equal with keyboard_input_on value
bool keyboard_is_blocking(void);


/* -- Keyboard Interrupt Service Routine -- */

/**
 * Handling keyboard interrupt & process scancodes into ASCII character.
 * Will start listen and process keyboard scancode if keyboard_input_on.
 * 
 * Will only print printable character into framebuffer.
 * Stop processing when enter key (line feed) is pressed.
 * 
 * Note that, with keyboard interrupt & ISR, keyboard reading is non-blocking.
 * This can be made into blocking input with `while (keyboard_is_blocking());` 
 * after calling `keyboard_state_activate();`
 */
void keyboard_isr();


// TODO: Document
void keyboard_flush_buffer(char *buf);
void keyboard_process_input(uint8_t input);


#endif