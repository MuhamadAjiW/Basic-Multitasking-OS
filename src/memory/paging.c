#include "../lib-header/paging.h"
#include "../lib-header/resource.h"

extern Resource resource_table[RESOURCE_AMOUNT];

struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .lower_address          = 0,
            .flag.use_pagesize_4_mb = 1,
        },
        [1] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .lower_address          = 1,
            .flag.use_pagesize_4_mb = 1,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .lower_address          = 0,
            .flag.use_pagesize_4_mb = 1,
        },
        [0x301] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .lower_address          = 1,
            .flag.use_pagesize_4_mb = 1,
        },
    }
};

void paging_dir_copy(PageDirectory origin, PageDirectory* target) {
    for (uint32_t i = 0; i < PAGE_ENTRY_COUNT; i++){
        target->table[i] = origin.table[i];
    }
}

void paging_dir_update(void *physical_addr, void *virtual_addr, struct PageDirectoryEntryFlag flag, PageDirectory* page_dir) {
    if(resource_table[(uint32_t) physical_addr / PAGE_FRAME_SIZE].used) return;     // Physical address is already used

    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;

    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t)physical_addr >> 22) & 0x3FF;

    paging_flush_tlb_single(virtual_addr);
}

void paging_flush_tlb_single(void *virtual_addr) {
    __asm__ volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}

void paging_flush_tlb_kernel(){
    paging_flush_tlb_range((void*)KERNEL_VMEMORY_OFFSET, (void*) (KERNEL_VMEMORY_OFFSET + KERNEL_PAGE_COUNT * PAGE_ENTRY_COUNT));
}

void paging_flush_tlb_heap(){
    paging_flush_tlb_range((void*)HEAP_VMEMORY_OFFSET, (void*) (HEAP_VMEMORY_OFFSET + HEAP_PAGE_COUNT * PAGE_ENTRY_COUNT));
}

void paging_flush_tlb_range(void *start_addr, void *end_addr) {
    uint32_t start = (uint32_t)start_addr;
    uint32_t end = (uint32_t)end_addr;

    for (uint32_t addr = start; addr < end; addr += PAGE_FRAME_SIZE) {
        asm volatile ("invlpg (%0)" : /* <Empty> */ : "b"(addr) : "memory");
    }
}

void paging_use_page_dir(PageDirectory* page_dir) {
    __asm__ volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(page_dir): "memory");
}
