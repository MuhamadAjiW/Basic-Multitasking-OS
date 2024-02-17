
#include "../lib-header/memory_manager.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/paging.h"

static uint32_t last_alloc = 0;
static uint32_t heap_start = 0;
static uint32_t heap_end = 0;

static uint32_t dynamic_pointers = 0;
extern bool paging_phys_memory_used[PAGE_PHYS_COUNT];
extern uint32_t paging_phys_memory_used_amount;

// Note:
// heap is placed under kernel space
// heap size is HEAP_PAGE_COUNT * PAGE_FRAME_SIZE
// Also don't forget that available_resources is used to track physical memory usage, not virtual
void memory_initialize(){
    // Assign pages used by the kernel, kernel is assumed to always start at 0
    for (uint16_t i = 0; i < KERNEL_PAGE_COUNT; i++){
        paging_phys_memory_used[i] = 1;
    }

    struct PageDirectoryEntryFlag flags ={
        .present_bit       = 1,
        .user_supervisor   = 1,
        .write_bit         = 1,
        .use_pagesize_4_mb = 1
    };
    
    for (uint16_t i = 0; i < HEAP_PAGE_COUNT; i++)    {
        paging_dir_update(
            (void *)((KERNEL_PAGE_COUNT + i) * PAGE_FRAME_SIZE),
            (void *)(HEAP_VMEMORY_OFFSET + (i * PAGE_FRAME_SIZE)),
            flags, &_paging_kernel_page_directory);
        
        uint32_t resource_index = KERNEL_PAGE_COUNT + i;
        paging_phys_memory_used[resource_index] = 1;
        paging_phys_memory_used_amount++;
    }

    last_alloc = HEAP_VMEMORY_OFFSET; //start alignment
    heap_start = last_alloc;
    heap_end = HEAP_VMEMORY_OFFSET + HEAP_PAGE_COUNT * PAGE_FRAME_SIZE;
    memset((char*) heap_start, 0, heap_end - heap_start);
}

void memory_clean(){
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
            struct allocator* a = (struct allocator*) memory;

            if(a->status){
                memory += a->size;
                memory += sizeof(struct allocator);
            }
            else{
                if(a->size >= size + sizeof(struct allocator) || a->size == size){
                    uint32_t oldsize = a->size;
                    a->status = 1;
                    a->size = size;

                    memset(memory + sizeof(struct allocator), 0, size);

                    if (oldsize != size){
                        a = (struct allocator*) ((uint32_t) a + sizeof(struct allocator) + size);
                        a->status = 0;
                        a->size = oldsize - size - sizeof(struct allocator);                        
                    }

                    dynamic_pointers++;
                    
                    return (void*)(memory + sizeof(struct allocator));
                }
                else{
                    memory += a->size;
                    memory += sizeof(struct allocator);
                }
            }
        }
    }

    if (last_alloc + size + sizeof(struct allocator) >= heap_end){
        __asm__ volatile ("int $4");
        return 0;
    }
    else{
        struct allocator* a = (struct allocator*) last_alloc;
        a->status = 1;
        a->size = size;

        last_alloc += size;
        last_alloc += sizeof(struct allocator);
        memset((char*)((uint32_t)a + sizeof(struct allocator)), 0, size);

        dynamic_pointers++;

        return (void*)((uint32_t)a + sizeof(struct allocator));
    }
}

void* krealloc(void* ptr, uint32_t size){
    struct allocator* alloc = (struct allocator*)((uint32_t)ptr - sizeof(struct allocator));
    uint32_t oldsize = alloc->size;

    void* newptr = kmalloc(size);

    if (oldsize > size) memcpy(newptr, ptr, size);
    else memcpy(newptr, ptr, oldsize);
    
    kfree(ptr);

    return newptr;
}



void kfree(void* memory){
    struct allocator* alloc = (memory - sizeof(struct allocator));
    alloc->status = 0;

    // TODO: Cleaner can still be improved

    void* cleaner = memory + alloc->size;
    struct allocator* roam = (struct allocator*) cleaner;
    while (!roam->status && (uint32_t) cleaner < last_alloc){
        
        alloc->size += roam->size + sizeof(struct allocator);
        
        cleaner += roam->size;
        cleaner += sizeof(struct allocator);

        roam = (struct allocator*) cleaner;
    }

    dynamic_pointers--;

    if(dynamic_pointers == 0) memory_clean();
}
