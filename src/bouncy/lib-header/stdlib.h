
#ifndef _STDMEM_H
#define _STDMEM_H

#include "stdtype.h"
#include "cmos.h"

/**
 *  Allocate memory in heap
 *  
 *  @param size size of allocated memory
 * 
 *  @return address of allocated memory
 */
void* malloc(uint32_t size);

/**
 *  Copies and reallocates allocated memory in heap to new address wth a new size
 *  Deallocates old address
 *  
 *  @param size size of allocated memory
 * 
 *  @return address of allocated memory
 */
void* realloc(void* ptr, uint32_t size);

/**
 *  Unallocate memory in heap
 *  Basically setting the status flag of allocated memory to 0
 *  
 *  @param ptr address of allocated memory
 */
void free(void* ptr);


//TODO: Document
cmos_reader get_time();

#endif