
#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "stdtype.h"
#include "memory_manager.h"
#include "paging.h"

// Max memory in qemu by default is 128 MB (changable up to 4GB)
// Each page being 4 MB, total memory available is 32
#define RESOURCE_AMOUNT 32

// Kernel and heap is assumed to be always at the bottom
#define RESOURCE_KERNEL_OFFSET (KERNEL_PAGE_COUNT + HEAP_PAGE_COUNT)

//TODO: Document
typedef struct Resource{
    uint32_t        pid;
    bool            used;
    PageDirectory*  page_dir;
} __attribute__((packed)) Resource;

//returns address of paging table for the process
PageDirectory* resource_allocate(uint32_t amount, uint32_t pid, PageDirectory* page_dir);

//should be self explanatory
void resource_deallocate(uint32_t pid);

#endif