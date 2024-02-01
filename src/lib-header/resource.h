
#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "stdtype.h"
#include "memory_manager.h"
#include "paging.h"

// Max memory in qemu by default is 128 MB (changable up to 4GB)
// Each page being 4 MB, total memory available is 32
// On this case max memory is modified to be 1gb
#define RESOURCE_AMOUNT 256

// Kernel and heap is assumed to be always at the bottom
#define RESOURCE_KERNEL_OFFSET (KERNEL_PAGE_COUNT + HEAP_PAGE_COUNT)

//TODO: Document
enum ResourceType { NULL_RESOURCE, KERNEL, HEAP, USER };
typedef struct Resource{
    uint32_t            pid;
    bool                used;
    enum ResourceType   type;
} __attribute__((packed)) Resource;

bool resource_check(uint32_t amount);

//Returns bottom of the stack given
uint32_t resource_allocate(uint32_t amount, uint32_t pid, PageDirectory* page_dir);
uint32_t resource_allocate_kernel(uint32_t pid, PageDirectory* page_dir);
void resource_deallocate(uint32_t pid, uint32_t resource_count);

#endif