
#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


//----Screen I/O
#define NULL_CHAR 0
#define TAB_CHAR 1
#define LARROW_CHAR 2
#define RARROW_CHAR 3
#define UARROW_CHAR 4
#define DARROW_CHAR 5
#define ESC_CHAR 6
#define BACKSPACE_CHAR 7

/**
 * Gets keyboard last pressed key and clears it
 * 
 * @return          last pressed key
 */
char sys_keyboard_get_char();

//TODO: Document
void sys_cursor_set_active(uint8_t active);
void sys_cursor_set_location(uint8_t r, uint8_t c);
void scroll();
void print(char* str);
void print_color(char* str, uint8_t fg_color, uint8_t bg_color);
void print_char(char c);
void print_char_color(char c, uint8_t fg_color, uint8_t bg_color);


//----File I/O
// Necessary filesystem related macros
#define END_OF_FILE 0xfffffff8

#define RESERVED_CLUSTER_NUMBER 0
#define FAT_CLUSTER_NUMBER 1
#define FAT_CLUSTER_LENGTH 64
#define CLUSTER_COUNT 32768
#define ROOT_CLUSTER_NUMBER 65

#define CLUSTER_SIZE 2048
#define ENTRY_COUNT 63


/**
 * Struct containing the value of bytes in a cluster
 * 
 * @param buf           Array of each byte
 */
struct ClusterBuffer{
    uint8_t buf[CLUSTER_SIZE];
}__attribute__((packed));

/**
 * Struct for FAT entries
 * Should be self explanatory
 * 
 */
struct DirectoryEntry{
    char filename[8];
    char extension[3];

    uint8_t read_only : 1 ;
    uint8_t hidden : 1 ;
    uint8_t system : 1 ;
    uint8_t volume_id : 1 ;
    uint8_t directory : 1 ;
    uint8_t archive : 1 ;
    uint8_t resbit1 : 1 ;
    uint8_t resbit2 : 1 ;
    
    uint8_t reserved;
    
    uint8_t creation_time_low;
    uint16_t creation_time_seconds : 5;
    uint16_t creation_time_minutes : 6;
    uint16_t creation_time_hours : 5;
    
    uint16_t creation_time_day : 5;
    uint16_t creation_time_month : 4;
    uint16_t creation_time_year : 7;

    uint16_t accessed_time_day : 5;
    uint16_t accessed_time_month : 4;
    uint16_t accessed_time_year : 7;

    uint16_t high_bits;

    uint16_t modification_time_seconds : 5;
    uint16_t modification_time_minutes : 6;
    uint16_t modification_time_hours : 5;

    uint16_t modification_time_day : 5;
    uint16_t modification_time_month : 4;
    uint16_t modifcation_time_year : 7;

    uint16_t cluster_number;
    uint32_t size;
}__attribute__((packed));

/**
 * Struct for FAT entries
 * Should be self explanatory
 * 
 */
struct DirectoryInfo{
    char filename[8];
    char extension[3];

    uint8_t read_only : 1 ;
    uint8_t hidden : 1 ;
    uint8_t system : 1 ;
    uint8_t volume_id : 1 ;
    uint8_t directory : 1 ;
    uint8_t archive : 1 ;
    uint8_t resbit1 : 1 ;
    uint8_t resbit2 : 1 ;
    
    uint8_t reserved;    

    uint16_t cluster_number;
    uint16_t parent_base_cluster;
    uint16_t parent_actual_cluster;
    uint16_t entry_number;

    uint32_t size;

    uint8_t unused[7];
}__attribute__((packed));

/**
 * Struct for folders
 * Index 0 always contains parent info, even in extension tables
 * 
 * @param entry file FAT entry
 */
struct DirectoryTable{
    struct DirectoryInfo info;
    struct DirectoryEntry entry[ENTRY_COUNT];
}__attribute__((packed));

/**
 * Struct for CRUD request
 * 
 * @param buf                   assigned buffer location for load, unused for other operations
 * @param name                  file name
 * @param ext                   file extension
 * @param parent_cluster_number parent cluster location
 * @param buffer_size           assigned buffer size for load, unused for other operations
 */
struct FAT32FileReader{
    uint32_t cluster_count;
    uint32_t size;
    struct ClusterBuffer* content;
}__attribute__((packed));

/**
 * Struct for reading files
 * 
 * @param cluster_count         number of clusters containing the read file
 * @param content               pointer to the actual content of the file divided to clusters
 */
struct FAT32DirectoryReader{
    uint32_t cluster_count;
    struct DirectoryTable* content;
}__attribute__((packed));

/**
 * Struct for reading folders
 * 
 * @param cluster_count         number of clusters containing the read folder
 * @param content               pointer to the actual content of the folder divided to clusters
 */
struct FAT32DriverRequest{
    void* buf;
    char name[8];
    char ext[3];
    uint32_t parent_cluster_number;
    uint32_t buffer_size;
}__attribute__((packed));


/**
 * Read a file from a request
 * @warning         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param request   requested file
 * 
 * @return          a struct containing the read data and number of read clusters
 */
struct FAT32FileReader readf(struct FAT32DriverRequest request);

/**
 * Read a folder from a request
 * @warning         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param request   requested folder
 * 
 * @return          a struct containing the read data and number of read clusters
 */
struct FAT32DirectoryReader readf_dir(struct FAT32DriverRequest request);

/**
 * Unallocates file reader buffer
 * @warning         this function is for allocated pointers, do not use on unallocated pointers
 * 
 * @param pointer   an allocated file reader pointer
 * 
 */
void closef(struct FAT32FileReader request);

/**
 * Unallocates folder reader buffer
 * @warning         this function is for allocated pointers, do not use on unallocated pointers
 * 
 * @param pointer   an allocated folder reader pointer
 * 
 */
void closef_dir(struct FAT32DirectoryReader request);

/**
 * Writes a requested file to the filesystem
 * 
 * @param request   requested file
 * 
 * @return          0 means success, anything other than 0 are error codes
 */
uint8_t writef(struct FAT32DriverRequest request);

/**
 * Deletes a fat directory entry in a directory cluster
 * 
 * @param request    requested file
 * 
 * @return           Status of deletion, 0 is success, other than 0 are error codes
 */
uint8_t deletef(struct FAT32DriverRequest request);


/**
 * Read clusters from the disk
 * 
 * @param reader                    destination buffer to store the read data
 * @param cluster                   cluster index to read
 * @param sector_count              number of clusters to read
 */
void readcluster(void* reader, uint16_t cluster, uint16_t sector_count);

/**
 * Copies a reader buffer and returns it as a directory table
 *  Seems redundant but quite useful in a lot of cases. Might be a subject to refactor.
 * 
 * @param reader                    pointer to a reader buffer
 * 
 * @return                          reader as directory table, contents are copied and not as a casted pointer
 */
struct DirectoryTable asdirectory(uint32_t* reader);

/**
 * Checks whether a cluster is a directory, uses reserved bits and directory flags, still might not be 100% accurate
 * 
 * @param cluster                   Cluster to check
 * 
 * @return                          1 if is a folder, 0 if not
 */
uint8_t isdirectory(uint32_t cluster);

#endif