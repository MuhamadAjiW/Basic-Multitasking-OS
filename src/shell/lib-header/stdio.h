
#ifndef _STDIO_H
#define _STDIO_H

#include "stdtype.h"

#define NULL_CHAR 0
#define TAB_CHAR 1
#define LARROW_CHAR 2
#define RARROW_CHAR 3
#define UARROW_CHAR 4
#define DARROW_CHAR 5
#define ESC_CHAR 6
#define BACKSPACE_CHAR 7

/**
 * Gets keyboard last pressed key and clears it
 * 
 * @return          last pressed key
 */
char sys_keyboard_get_char();

//TODO: Document
void sys_cursor_set_active(uint8_t active);
void sys_cursor_set_location(uint8_t r, uint8_t c);
void scroll();
void print(char* str);
void print_color(char* str, uint8_t fg_color, uint8_t bg_color);
void print_char(char c);
void print_char_color(char c, uint8_t fg_color, uint8_t bg_color);

#endif