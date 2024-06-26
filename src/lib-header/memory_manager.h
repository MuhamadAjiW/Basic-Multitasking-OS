
#ifndef _MEMMNG_H
#define _MEMMNG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HEAP_PAGE_COUNT          4
#define KERNEL_PAGE_COUNT        1

extern uint32_t _linker_kernel_virtual_addr_start;
extern uint32_t _linker_kernel_virtual_addr_end;
extern uint32_t _linker_kernel_physical_addr_start;
extern uint32_t _linker_kernel_physical_addr_end;
extern uint32_t _linker_kernel_stack_top;


#define KERNEL_PMEMORY_OFFSET    0
#define KERNEL_VMEMORY_OFFSET    0xc0000000
#define HEAP_PMEMORY_OFFSET      KERNEL_PAGE_COUNT * PAGE_FRAME_SIZE
#define HEAP_VMEMORY_OFFSET      0xb0000000
#define END_OF_MEMORY            0xffffffff

/**
 *  Struct to segment heap linearly
 *  Should be self explanatory
 * 
 */
struct allocator{
    uint8_t status;
    uint32_t size;
} __attribute__((packed));

/**
 *  Page heap memory with appropriate flags and set heap with 0
 * 
 */
void memory_initialize();

/**
 *  Reset heap memory with 0
 *  Should only be done if no pointers are active
 * 
 */
void memory_clean();

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