
#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "stdtype.h"

// Macros for system call codes, should always be synchronized with the kernel side
#define SYSCALL_NULL 0

/**
 * Sends syscall to the kernel
 * @attention   make sure to add a response in the kernel side if not already
 *              eax should be filled with system call codes
 *              the rest may be filled with function arguments
*/
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif