
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "interrupt.h"

#define SYSCALL_OFFSET 0x30
#define SYSCALL_COUNT 128

// Macros for system call codes, should always be synchronized with the user side
#define SYSCALL_NULL 0
#define SYSCALL_GET_TICK 1
#define SYSCALL_GET_TIME 2

#define SYSCALL_MALLOC 10
#define SYSCALL_REALLOC 11
#define SYSCALL_FREE 12

#define SYSCALL_WINMGR_REG 20
#define SYSCALL_WINMGR_UPDATE 21
#define SYSCALL_WINMGR_CLOSE 22

#define SYSCALL_TASK_EXIT 23

/**
 * Assigns system response functions to system call number
 * 
*/
void enable_system_calls();

#endif
