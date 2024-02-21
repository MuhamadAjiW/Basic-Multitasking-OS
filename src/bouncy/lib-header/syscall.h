
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Macros for system call codes, should always be synchronized with the kernel side
#define SYSCALL_NULL 0
#define SYSCALL_GET_TICK 1
#define SYSCALL_GET_TIME 2
#define SYSCALL_GET_KEYBOARD_LAST_KEY 3
#define SYSCALL_SET_CURSOR_ACTIVE 4
#define SYSCALL_SET_CURSOR_LOCATION 5

#define SYSCALL_MALLOC 10
#define SYSCALL_REALLOC 11
#define SYSCALL_FREE 12

#define SYSCALL_WINMGR_REG 20
#define SYSCALL_WINMGR_UPDATE 21
#define SYSCALL_WINMGR_CLOSE 22

#define SYSCALL_PROCESS_START 30
#define SYSCALL_PROCESS_STOP 31
#define SYSCALL_PROCESS_EXIT 32
#define SYSCALL_PROCESS_INFO 33

#define SYSCALL_READ_FILE 40
#define SYSCALL_READ_DIR 41
#define SYSCALL_SELF_DIR_INFO 42
#define SYSCALL_WRITE_FILE 43
#define SYSCALL_DELETE_FILE 44
#define SYSCALL_CLOSE_FILE 45
#define SYSCALL_CLOSE_DIR 46

/**
 * Sends syscall to the kernel
 * @attention   make sure to add a response in the kernel side if not already
 *              eax should be filled with system call codes
 *              the rest may be filled with function arguments
*/
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

/**
 *  blocks current process for a certain amount of ticks
 *  
 *  @param tick time in PIT interrupt (value depends on frequency set)
 */
void delay(uint32_t tick);


/**
 *  Closes the process window
 *  
 *  @param id designated id for a window, received when a window is registered
 */
void close_window(uint32_t id);


/**
 *  Syscall to terminate current process
 *  
 */
void exit();

#endif