#include "../lib-header/resource.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/paging.h"

// global amount of available resources, pages are located by its index
// Used to track physical memory rather than virtual
Resource resource_table[RESOURCE_AMOUNT];

PageDirectory* resource_allocate(uint32_t amount, uint32_t pid, PageDirectory* page_dir){
    uint32_t i = RESOURCE_KERNEL_OFFSET;
    uint32_t available = 0;
    
    // Check whether there are resource available
    while (i < RESOURCE_AMOUNT - amount){
        if (!resource_table[i].used){
            available++;
        }
        
        if(available >= amount) break;
        i++;
    }

    // If not, return 0
    if (available < amount) return 0;
    
    // If yes, allocate the pages
    paging_dir_copy(_paging_kernel_page_directory, page_dir);

    // TODO: add as parameter instead since flag may be kernel for kernel processes
    struct PageDirectoryEntryFlag flag ={
        .present_bit       = 1,
        .user_supervisor = 1,
        .write_bit = 1,
        .use_pagesize_4_mb = 1
    };

    i = RESOURCE_KERNEL_OFFSET;
    for (uint32_t j = 0; j < amount; j++){
        while (i < RESOURCE_AMOUNT && resource_table[i].used) i++;
        
        uint32_t physical_addr = i * PAGE_FRAME_SIZE;
        paging_dir_update((void*) physical_addr, (void*) ((j) * PAGE_FRAME_SIZE), flag, page_dir);    

        resource_table[i].used = 1;
        resource_table[i].pid = pid;
        resource_table[i].page_dir = page_dir;
    }

    return (PageDirectory*) page_dir;
}

void resource_deallocate(uint32_t pid){
    uint8_t i = RESOURCE_KERNEL_OFFSET;

    while (i < RESOURCE_AMOUNT){
        if(resource_table[i].pid == pid) resource_table[i].used = 0;
        i++;
    }
}