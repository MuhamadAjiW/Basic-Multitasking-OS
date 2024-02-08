
#ifndef _STDIO_H
#define _STDIO_H

#include "stdtype.h"

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
typedef struct ClusterBuffer{
    uint8_t buf[CLUSTER_SIZE];
}__attribute__((packed)) ClusterBuffer;

/**
 * Struct for FAT entries
 * Should be self explanatory
 * 
 */
typedef struct DirectoryEntry{
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
}__attribute__((packed)) DirectoryEntry;

/**
 * Struct for FAT entries
 * Should be self explanatory
 * 
 */
typedef struct DirectoryInfo{
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
}__attribute__((packed)) DirectoryInfo;

/**
 * Struct for folders
 * Index 0 always contains parent info, even in extension tables
 * 
 * @param entry file FAT entry
 */
typedef struct DirectoryTable{
    DirectoryInfo info;
    DirectoryEntry entry[ENTRY_COUNT];
}__attribute__((packed)) DirectoryTable;

/**
 * Struct for CRUD request
 * 
 * @param buf                   assigned buffer location for load, unused for other operations
 * @param name                  file name
 * @param ext                   file extension
 * @param parent_cluster_number parent cluster location
 * @param buffer_size           assigned buffer size for load, unused for other operations
 */
typedef struct FAT32FileReader{
    uint32_t cluster_count;
    uint32_t size;
    ClusterBuffer* content;
}__attribute__((packed)) FAT32FileReader;

/**
 * Struct for reading files
 * 
 * @param cluster_count         number of clusters containing the read file
 * @param content               pointer to the actual content of the file divided to clusters
 */
typedef struct FAT32DirectoryReader{
    uint32_t cluster_count;
    DirectoryTable* content;
}__attribute__((packed)) FAT32DirectoryReader;

/**
 * Struct for reading folders
 * 
 * @param cluster_count         number of clusters containing the read folder
 * @param content               pointer to the actual content of the folder divided to clusters
 */
typedef struct FAT32DriverRequest{
    void* buf;
    char name[8];
    char ext[3];
    uint32_t parent_cluster_number;
    uint32_t buffer_size;
}__attribute__((packed)) FAT32DriverRequest;


/**
 * Read a file from a request
 * @warning         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param request   requested file
 * 
 * @return          a struct containing the read data and number of read clusters
 */
FAT32FileReader readf(FAT32DriverRequest* request);

/**
 * Read a folder from a request
 * @warning         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param request   requested folder
 * 
 * @return          a struct containing the read data and number of read clusters
 */
FAT32DirectoryReader readf_dir(FAT32DriverRequest* request);

/**
 * Unallocates file reader buffer
 * @warning         this function is for allocated pointers, do not use on unallocated pointers
 * 
 * @param pointer   an allocated file reader pointer
 * 
 */
void closef(FAT32FileReader* request);

/**
 * Unallocates folder reader buffer
 * @warning         this function is for allocated pointers, do not use on unallocated pointers
 * 
 * @param pointer   an allocated folder reader pointer
 * 
 */
void closef_dir(FAT32DirectoryReader* request);

/**
 * Writes a requested file to the filesystem
 * 
 * @param request   requested file
 * 
 * @return          0 means success, anything other than 0 are error codes
 */
uint8_t writef(FAT32DriverRequest* request);

/**
 * Deletes a fat directory entry in a directory cluster
 * 
 * @param request    requested file
 * 
 * @return           Status of deletion, 0 is success, other than 0 are error codes
 */
uint8_t deletef(FAT32DriverRequest* request);

#endif