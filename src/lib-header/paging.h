#ifndef _PAGING_H
#define _PAGING_H

#include "stdtype.h"

#define PAGE_ENTRY_COUNT 1024
#define PAGE_FRAME_SIZE  (4*1024*1024)

// Operating system page directory, using page size PAGE_FRAME_SIZE (4 MiB)
extern struct PageDirectory _paging_kernel_page_directory;

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
    // TODO : Implement
} __attribute__((aligned(0x1000)));

/**
 * Containing page driver states
 * 
 * @param last_available_physical_addr Pointer to last empty physical addr (multiple of 4 MiB)
 */
struct PageDriverState {
    uint8_t *last_available_physical_addr;
} __attribute__((packed));

/**
 * update_page_directory,
 * Edit _paging_kernel_page_directory with respective parameter
 * 
 * @param physical_addr Physical address to map
 * @param virtual_addr  Virtual address to map
 * @param flag          Page entry flags
 */
void update_page_directory_entry(void *physical_addr, void *virtual_addr, struct PageDirectoryEntryFlag flag);

/**
 * flush_single_tlb, 
 * invalidate page that contain virtual address in parameter
 * 
 * @param virtual_addr Virtual address to flush
 */
void flush_single_tlb(void *virtual_addr);

/**
 * Allocate user memory into specified virtual memory address.
 * Multiple call on same virtual address will unmap previous physical address and change it into new one.
 * 
 * @param  virtual_addr Virtual address to be mapped
 * @return int8_t       0 success, -1 for failed allocation
 */
int8_t allocate_single_user_page_frame(void *virtual_addr);

#endif