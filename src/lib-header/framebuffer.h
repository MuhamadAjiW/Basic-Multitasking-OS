
#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MEMORY_FRAMEBUFFER (uint8_t *) 0xC00B8000
#define CURSOR_PORT_CMD    0x03D4
#define CURSOR_PORT_DATA   0x03D5

#define SCREEN_WIDTH       80
#define SCREEN_HEIGHT      25

/**
 * Terminal framebuffer
 * Resolution: 80x25
 * Starting at MEMORY_FRAMEBUFFER,
 * - Even number memory: Character, 8-bit
 * - Odd number memory:  Character color lower 4-bit, Background color upper 4-bit
*/

/**
 * Set framebuffer character and color with corresponding parameter values.
 * More details: https://en.wikipedia.org/wiki/BIOS_color_attributes
 *
 * @param row       Vertical location (index start 0)
 * @param col       Horizontal location (index start 0)
 * @param c         Character
 * @param fg        Foreground / Character color
 * @param bg        Background color
 */
void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg);

/**
 * Set cursor to specified location. Row and column starts from 0
 * 
 * @param r row
 * @param c column
*/
void framebuffer_set_cursor(uint8_t r, uint8_t c);

/** 
 * Set all cell in framebuffer character to 0x00 (empty character)
 * and color to 0x07 (gray character & black background)
 * 
 */
void framebuffer_clear(void);

/** 
 * Displays the double buffer to the primary graphics memory
 */
void framebuffer_display();

/** 
 * Enables the built in cursor from the VGA
 */
void framebuffer_enable_cursor();

/** 
 * Disables the built in cursor from the VGA
 */
void framebuffer_disable_cursor();

/**
 * framebuffer_set, Sets the secondary buffer with a certain value
 * 
 * @param row           Y coordinate of buffer to be written
 * @param col           X coordinate of buffer to be written
 * @param info          information of word to be written to the buffer
 */
void framebuffer_set(uint8_t row, uint8_t col, uint16_t info);

#endif