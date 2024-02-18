
#ifndef _PAGING_H
#define _PAGING_H

#include "stdtype.h"

#define PAGE_ENTRY_COUNT 1024
#define PAGE_FRAME_SIZE  0x400000
#define PAGE_FRAME_MAX_COUNT 32

// Operating system page directory, using page size PAGE_FRAME_SIZE (4 MiB)
extern struct PageDirectory _paging_kernel_page_directory;

/**
 * Containing page manager states.
 * 
 * @param page_frame_map            Keeping track empty space on the physical frame
 * @param free_page_frame_count     Keeping track empty space amount left
 * ...
 */
struct PageManagerState {
    bool     page_frame_map[PAGE_FRAME_MAX_COUNT];
    uint32_t free_page_frame_count;
    // TODO: Add if needed ...
} __attribute__((packed));

/**
 * Page Directory Entry Flag, only first 8 bit
 * 
 * @param present_bit           Indicate whether this entry is exist or not
 * @param write_bit             Indicate whether this entry is writable or not
 * @param user_supervisor       Indicate whether this entry is accessible for users
 * @param write_through         Indicate whether this entry uses write through, if not, write-back is used
 * @param cache_disable         Indicate whether this entry is cachable
 * @param accessed              Indicate whether this entry has been accessed or not. This flag should only be set by the CPU
 * @param dirty                 Indicate whether this entry has been written into. This flag should only be set by the CPU
 * @param use_pagesize_4_mb     Indicate whether this entry is using 4mb page size
 */
struct PageDirectoryEntryFlag {
    uint8_t present_bit        : 1;
    uint8_t write_bit          : 1;
    uint8_t user_supervisor    : 1;
    uint8_t write_through      : 1;
    uint8_t cache_disable      : 1;
    uint8_t accessed           : 1;
    uint8_t dirty              : 1;
    uint8_t use_pagesize_4_mb  : 1;
} __attribute__((packed));

/**
 * Page Directory Entry, for page size 4 MB.
 * Check Intel Manual 3a - Ch 4 Paging - Figure 4-4 PDE: 4MB page
 * 
 * @param flag            Contain 8-bit page directory entry flag
 * @param global_page     Is this page translation global (also cannot be flushed)
 * @param AVL             Free flags for programmers use
 * @param PAT             Page Attribute Table flag, indicates memory caching type
 * @param higher_address  39-32 bits of address, should be set to 0 for 32 bit operating systems  
 * @param RSVD            Indicates whether a reserved bit was set in some page-structure entry
 * @param lower_address   31-22 bits of paged address
 */
struct PageDirectoryEntry {
    struct PageDirectoryEntryFlag flag;
    uint16_t global_page    : 1;
    uint16_t AVL            : 3;
    uint16_t PAT            : 1;
    uint16_t higher_address : 8;
    uint16_t RSVD           : 1;
    uint16_t lower_address  : 10;
} __attribute__((packed));

/**
 * Page Directory, contain array of PageDirectoryEntry.
 * Note: This data structure not only can be manipulated by kernel, 
 *   MMU operation, TLB hit & miss also affecting this data structure (dirty, accessed bit, etc).
 * Warning: Address must be aligned in 4 KB (listed on Intel Manual), use __attribute__((aligned(0x1000))), 
 *   unaligned definition of PageDirectory will cause triple fault
 * 
 * @param table Fixed-width array of PageDirectoryEntry with size PAGE_ENTRY_COUNT
 */
struct PageDirectory {
    struct PageDirectoryEntry table[PAGE_ENTRY_COUNT];
} __attribute__((aligned(0x1000)));

/**
 * paging_dir_update,
 * Edit _paging_kernel_page_directory with respective parameter
 * 
 * @param physical_addr Physical address to map
 * @param virtual_addr  Virtual address to map
 * @param flag          Page entry flags
 * @param page_dir      Page directory to update
 */
void paging_dir_update(void *physical_addr, void *virtual_addr, struct PageDirectoryEntryFlag flag, struct PageDirectory* page_dir);

/**
 * paging_flush_tlb_single, 
 * invalidate page that contain virtual address in parameter
 * 
 * @param virtual_addr Virtual address to flush
 */
void paging_flush_tlb_single(void *virtual_addr);


/**
 * paging_flush_tlb_range, 
 * invalidate page that is in range of virtual address in parameter
 * 
 * @param start_addr    Start of Virtual address range to flush
 * @param end_addr      End of Virtual address range to flush
 */
void paging_flush_tlb_range(void *start_addr, void *end_addr);

/**
 * paging_use_page_dir, 
 * change active Page Directory to the inserted address value
 * 
 * @param page_dir      Physical address of page directory to be used
 */
void paging_use_page_dir(struct PageDirectory* page_dir); //Page dir must be physical address and aligned to 0x1000

/**
 * paging_dirtable_init, 
 * initialize a new page table by copying kernel page directory entries to it
 * 
 * @param dest          address of page directory to be initialized
 */
void paging_dirtable_init(struct PageDirectory* dest);

/**
 * paging_allocate_page_frame, 
 * allocate a physical address to a virtual address in a certain page directory
 * 
 * @param virt_addr           address of virtual address page to be allocated
 * @param page_dir            address of page directory to be given physical frame
 * @return                    address of given physical frame
 */
void paging_allocate_page_frame(void *virt_addr, struct PageDirectory* page_dir);

/**
 * paging_allocate_page_frame, 
 * deallocate a physical frame from a virtual address in a certain page directory
 * 
 * @param virt_addr           address of virtual address page to be allocated
 * @param page_dir            address of page directory to be given physical frame
 * @return                    success state of function with TRUE being page is freed successfully
 */
bool paging_free_page_frame(void *virt_addr, struct PageDirectory* page_dir);


/**
 * paging_allocate_check, 
 * check whether a certain amount of physical frames are available
 * 
 * @param amount              requested amount of physical frames 
 * @return                    availability of physical frame with TRUE being amount of available physical frame is more than requested amount 
 */
bool paging_allocate_check(uint32_t amount);


/**
 * Page Frame, map of virtual address and physical address
 * 
 * @param virtual_addr           Mapped virtual address of physical address
 * @param physical_addr          Mapped physical address of virtual address
 */
struct PageFrame {
   void *virtual_addr;
   void *physical_addr;
};

/**
 * paging_clone_directory_entry
 * copy the entry of a particular virtual address of a page directory to another
 *  
 * @param virt_addr              Mapped virtual address of entry to be copied
 * @param src                    Origin page directory of virtual address to be copied
 * @param dest                   Destination page directory of virtual address to be copied to
 */
void paging_clone_directory_entry(void *virt_addr, struct PageDirectory* src, struct PageDirectory* dest);

#endif