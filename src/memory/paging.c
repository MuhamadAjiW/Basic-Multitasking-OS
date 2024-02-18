#include "../lib-header/paging.h"
#include "../lib-header/process.h"

// Map for used physical frames, TRUE means a physical frame is used
struct PageManagerState page_manager_state = {
    .page_frame_map = {[0 ... PAGE_FRAME_MAX_COUNT-1] = FALSE},
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT
};
extern struct PageDirectory process_page_dir[MAX_PROCESS];

struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .lower_address          = 0,
            .flag.use_pagesize_4_mb = 1,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .lower_address          = 0,
            .flag.use_pagesize_4_mb = 1,
        },
    }
};

void paging_dir_update(void *physical_addr, void *virtual_addr, struct PageDirectoryEntryFlag flag, struct PageDirectory* page_dir) {
    if(page_manager_state.page_frame_map[(uint32_t) physical_addr / PAGE_FRAME_SIZE]) return;     // Physical address is already used

    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;

    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = ((uint32_t)physical_addr >> 22) & 0x3FF;

    paging_flush_tlb_single(virtual_addr);
}

void paging_flush_tlb_single(void *virtual_addr) {
    __asm__ volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}

void paging_flush_tlb_range(void *start_addr, void *end_addr) {
    uint32_t start = (uint32_t)start_addr;
    uint32_t end = (uint32_t)end_addr;

    for (uint32_t addr = start; addr < end; addr += PAGE_FRAME_SIZE) {
        asm volatile ("invlpg (%0)" : /* <Empty> */ : "b"(addr) : "memory");
    }
}

void paging_use_page_dir(struct PageDirectory* page_dir) {
    __asm__ volatile("mov %0, %%cr3" : /* <Empty> */ : "r"(page_dir): "memory");
}


void paging_dirtable_init(struct PageDirectory* dest) {
    // TODO: Copy the entirety of _paging_kernel_page_directory to dest

        // Kunjaw:
        for (uint32_t i = 0; i < PAGE_ENTRY_COUNT; i++){
            dest->table[i] = _paging_kernel_page_directory.table[i];
        }
}

void paging_clone_directory_entry(void *virt_addr, struct PageDirectory* src, struct PageDirectory* dest){
    // TODO: Copy entry of virt_addr from src to dest

        // Kunjaw:
        uint32_t index = (uint32_t) virt_addr / PAGE_FRAME_SIZE;
        dest->table[index] = src->table[index];
}

bool paging_allocate_check(uint32_t amount){
    // TODO: check whether requested amount is available

        // Kunjaw
        return page_manager_state.free_page_frame_count >= amount;
}

void paging_allocate_page_frame(void *virt_addr, struct PageDirectory* page_dir){
    // Assumed that the user will always check beforehand
    // TODO: allocate a physical frame; find free physical frame and mark it as used

        // Kunjaw:
        struct PageDirectoryEntryFlag flag ={
            .present_bit       = 1,
            .user_supervisor = 1,
            .write_bit = 1,
            .use_pagesize_4_mb = 1
        };

        uint32_t i = 0;
        while (i < PAGE_FRAME_MAX_COUNT && page_manager_state.page_frame_map[i]) i++;

        void* phys_addr = (void*) (i * PAGE_FRAME_SIZE);

        paging_dir_update(phys_addr, virt_addr, flag, page_dir);
        page_manager_state.page_frame_map[i] = TRUE;
        page_manager_state.free_page_frame_count--;
}

bool paging_free_page_frame(void *virt_addr, struct PageDirectory* page_dir){
    // TODO: deallocate a physical frame; mark it as unused and remove it from the page directory
        
        // Kunjaw:
        uint32_t index = page_dir->table[(uint32_t) virt_addr / PAGE_FRAME_SIZE].lower_address;
        if(!page_manager_state.page_frame_map[index]) return FALSE;

        struct PageDirectoryEntryFlag flag ={0};
        paging_dir_update(0, (void*) virt_addr, flag, page_dir);
        page_manager_state.page_frame_map[index] = FALSE;
        page_manager_state.free_page_frame_count++;

        return TRUE;
}
