
#ifndef _MEMMNG_H
#define _MEMMNG_H

#include "stdtype.h"

/**
 *  Struct to segment heap linearly
 *  Should be self explanatory
 * 
 */
typedef struct allocator{
    uint8_t status;
    uint32_t size;
} __attribute__((packed)) allocator;

/**
 *  Page heap memory with appropriate flags and set heap with 0
 * 
 */
void initialize_memory();

/**
 *  Reset heap memory with 0
 *  Should only be done if no pointers are active
 * 
 */
void clean_memory();

/**
 *  Allocate memory in heap
 *  
 *  @param size size of allocated memory
 * 
 *  @return address of allocated memory
 */
void* kmalloc(uint32_t size);

/**
 *  Copies and reallocates allocated memory in heap to new address wth a new size
 *  Deallocates old address
 *  
 *  @param size size of allocated memory
 * 
 *  @return address of allocated memory
 */
void* krealloc(void* ptr, uint32_t size);

/**
 *  Unallocate memory in heap
 *  Basically setting the status flag of allocated memory to 0
 *  
 *  @param ptr address of allocated memory
 * 
 */
void kfree(void* ptr);


#endif