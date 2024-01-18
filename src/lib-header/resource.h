
#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "stdtype.h"
#include "memory_manager.h"

// Max memory in qemu by default is 128 MB (changable up to 4GB)
// Each page being 4 MB, total memory available is 32
#define RESOURCE_AMOUNT 32

//TODO: Document
typedef struct Resource{
    uint8_t  pid;
    bool     used;
} __attribute__((packed)) Resource;

//returns address of paging table for the process
uint32_t allocate_resource(uint8_t amount, uint8_t pid);

//should be self explanatory
void deallocate_resource(uint8_t pid);

#endif