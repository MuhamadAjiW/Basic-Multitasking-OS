
#include "../lib-header/memory_manager.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/paging.h"

static uint32_t last_alloc = 0;
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;

static uint32_t dynamic_pointers = 0;

// heap is placed under kernel space
// heap size is HEAP_PAGE_COUNT * PAGE_FRAME_SIZE
void initialize_memory(){
    struct PageDirectoryEntryFlag flags ={
        .present_bit       = 1,
        .user_supervisor   = 1,
        .write_bit         = 1,
        .use_pagesize_4_mb = 1
    };
    
    for (int i = 0; i < HEAP_PAGE_COUNT; i++)    {
        update_page_directory_entry(
            (void *)(KERNEL_PMEMORY_END + (i * PAGE_FRAME_SIZE)),
            (void *)(HEAP_VMEMORY_OFFSET + (i * PAGE_FRAME_SIZE)),
            flags);
    }

    last_alloc = HEAP_VMEMORY_OFFSET; //start alignment
    heap_start = last_alloc;
    heap_end = HEAP_VMEMORY_OFFSET + HEAP_PAGE_COUNT * PAGE_FRAME_SIZE;
    memset((char*) heap_start, 0, heap_end - heap_start);
}

void clean_memory(){
    memset((char*) heap_start, 0, last_alloc - heap_start);
    last_alloc = heap_start;
}

void* kmalloc(uint32_t size){
    void* memory = (void*) heap_start;

    if (size == 0){
        return 0;
    }
    else{
        while((uint32_t) memory < last_alloc){
            allocator* a = (allocator*) memory;

            if(a->status){
                memory += a->size;
                memory += sizeof(allocator);
            }
            else{
                if(a->size >= size + sizeof(allocator) || a->size == size){
                    uint32_t oldsize = a->size;
                    a->status = 1;
                    a->size = size;

                    memset(memory + sizeof(allocator), 0, size);

                    if (oldsize != size){
                        a = (allocator*) ((uint32_t) a + sizeof(allocator) + size);
                        a->status = 0;
                        a->size = oldsize - size - sizeof(allocator);                        
                    }

                    dynamic_pointers++;
                    
                    return (void*)(memory + sizeof(allocator));
                }
                else{
                    memory += a->size;
                    memory += sizeof(allocator);
                }
            }
        }
    }

    if (last_alloc + size + sizeof(allocator) >= heap_end){
        __asm__ volatile ("int $4");
        return 0;
    }
    else{
        allocator* a = (allocator*) last_alloc;
        a->status = 1;
        a->size = size;

        last_alloc += size;
        last_alloc += sizeof(allocator);
        memset((char*)((uint32_t)a + sizeof(allocator)), 0, size);

        dynamic_pointers++;

        return (void*)((uint32_t)a + sizeof(allocator));
    }
}

void* krealloc(void* ptr, uint32_t size){
    allocator* alloc = (allocator*)((uint32_t)ptr - sizeof(allocator));
    uint32_t oldsize = alloc->size;

    void* newptr = kmalloc(size);

    if (oldsize > size) memcpy(newptr, ptr, size);
    else memcpy(newptr, ptr, oldsize);
    
    kfree(ptr);

    return newptr;
}



void kfree(void* memory){
    allocator* alloc = (memory - sizeof(allocator));
    alloc->status = 0;

    // TODO: Fix cleaner?

    void* cleaner = memory + alloc->size;
    allocator* roam = (allocator*) cleaner;
    while (!roam->status && (uint32_t) cleaner < last_alloc){
        
        alloc->size += roam->size + sizeof(allocator);
        
        cleaner += roam->size;
        cleaner += sizeof(allocator);

        roam = (allocator*) cleaner;
    }

    dynamic_pointers--;

    if(dynamic_pointers == 0) clean_memory();
}
