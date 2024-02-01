#include "../lib-header/resource.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/paging.h"

// global amount of available resources, pages are located by its index
// Used to track physical memory rather than virtual
Resource resource_table[RESOURCE_AMOUNT];

bool resource_check(uint32_t amount){
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
    return (available >= amount);
}

uint32_t resource_allocate(uint32_t amount, uint32_t pid, PageDirectory* page_dir){
    // Always check with resource_check

    struct PageDirectoryEntryFlag flag ={
        .present_bit       = 1,
        .user_supervisor = 1,
        .write_bit = 1,
        .use_pagesize_4_mb = 1
    };

    // User stack are always allocated from 0
    uint32_t i = RESOURCE_KERNEL_OFFSET;
    for (uint32_t j = 0; j < amount; j++){
        while (i < RESOURCE_AMOUNT && resource_table[i].used) i++;
        
        uint32_t physical_addr = i * PAGE_FRAME_SIZE;
        uint32_t virtual_addr = j * PAGE_FRAME_SIZE;

        paging_dir_update((void*) physical_addr, (void*) virtual_addr, flag, page_dir);    

        resource_table[i].used = 1;
        resource_table[i].pid = pid;
        resource_table[i].type = USER;
    }

    return amount * PAGE_FRAME_SIZE;
}

// TODO: Improve, for now resource can only allocate one page
uint32_t resource_allocate_kernel(uint32_t pid, PageDirectory* page_dir){
    // Always check with resource_check
    
    struct PageDirectoryEntryFlag flag ={
        .present_bit       = 1,
        .write_bit = 1,
        .use_pagesize_4_mb = 1
    };

    // Kernel stack is allocated differently than user since it always has to be different from the current running processes
    // If it's a kernel stack, then it's mapped to above 0xC0000000
    // Won't collide with heap or process 0 since i starts from RESOURCE_KERNEL_OFFSET
    // Also won't collide with other processes since it's practically offset-ed physical address
    uint32_t i = RESOURCE_KERNEL_OFFSET;

    while (i < RESOURCE_AMOUNT && resource_table[i].used) i++;
    
    uint32_t physical_addr = i * PAGE_FRAME_SIZE;
    uint32_t virtual_addr = KERNEL_VMEMORY_OFFSET + physical_addr;

    paging_dir_update((void*) physical_addr, (void*) virtual_addr, flag, page_dir);

    resource_table[i].used = 1;
    resource_table[i].pid = pid;
    resource_table[i].type = KERNEL;

    return virtual_addr + PAGE_FRAME_SIZE;
}


// Very naive implementation
void resource_deallocate(uint32_t pid, uint32_t resource_count){
    uint32_t i = RESOURCE_KERNEL_OFFSET;
    uint32_t resource_counter = 0;
    while (i < RESOURCE_AMOUNT){
        if(resource_table[i].pid == pid){
            resource_table[i].used = 0;
            resource_counter++;
        }

        if(resource_counter == resource_count) break;
        
        i++;
    }
}