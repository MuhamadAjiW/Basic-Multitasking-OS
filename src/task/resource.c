#include "../lib-header/resource.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/paging.h"

//global amount of available resources, pages are located by its index
Resource available_resource[RESOURCE_AMOUNT];

uint32_t allocate_resource(uint8_t amount, uint8_t pid){
    uint8_t i = 0;
    uint8_t available = 0;
    
    // Check whether there are contiguous resource available
    while (i < RESOURCE_AMOUNT - amount){
        available = 1;
        for (uint8_t j = i; j < i + amount; j++){
            if (available_resource->used){
                available = 0;
                break;
            }
        }
        
        if(available) break;
    }

    // If not, return 0
    if (!available) return 0;
    
    // If yes, allocate the pages and return the end of stack
    for (uint8_t j = 0; j < amount; j++){
        uint8_t location = i + j;
        allocate_single_user_page_frame((void*) ((i + j) * PAGE_FRAME_SIZE));
        available_resource[location].pid = pid;
        available_resource[location].used = 1;
    }

    return (i + amount) * PAGE_FRAME_SIZE;
}

void deallocate_resource(uint8_t pid){
    uint8_t i = 0;
    while (i < RESOURCE_AMOUNT && available_resource[i].pid != pid){
        i++;
    }

    while (i < RESOURCE_AMOUNT && available_resource[i].pid == pid){
        available_resource[i].used = 0;
        flush_single_tlb((void*) (i * PAGE_FRAME_SIZE));
    }
}