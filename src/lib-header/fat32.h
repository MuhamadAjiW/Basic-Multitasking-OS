
#ifndef _FAT32_H
#define _FAT32_H

#include "stdtype.h"

// Necessary filesystem related macros
#define END_OF_FILE 0xfffffff8

#define RESERVED_CLUSTER_NUMBER 0
#define FAT_CLUSTER_NUMBER 1
#define FAT_CLUSTER_LENGTH 64
#define CLUSTER_COUNT 32768
#define ROOT_CLUSTER_NUMBER 65

#define CLUSTER_SIZE 2048
#define ENTRY_COUNT 63

/* further reading:
        https://en.wikipedia.org/wiki/File_Allocation_Table
        https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system
        https://wiki.osdev.org/FAT
*/



/* Bootsector, commented as the specification explicitly says we do not need it

#define BYTE_PER_SECTOR 4096
#define SECTORS_PER_CLUSTER 64
#define RESERVED_SECTORS 32
#define FAT_COUNT 2
#define DIRECTORY_ENTRY_COUNT 0
#define TOTAL_SECTORS 0
#define MEDIA_DESCRIPTOR_TYPE 0xf8
#define SECTORS_PER_FAT 0
#define SECTORS_PER_TRACK 0
#define HEADS 0
#define HIDDEN_SECTORS 0
#define LARGE_SECTOR_COUNT 1

#define SECTORS_PER_FAT_32 64
#define EXTERNAL_FLAGS 0
#define VERSION 0
#define ROOT_CLUSTER 2
#define FS_INFO 1
#define BK_BOOT_SECTOR 6
#define RESERVED 0
#define DRIVE_NUMBER 0x80
#define RESERVED1 0
#define BOOT_SIGNATURE 1
*/

/*
struct FAT32BootSector{
    uint8_t boot_jump_instrution[3];
    uint8_t oem_identifier[8];
    uint16_t bytes_per_Sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t directory_entry_count;
    uint16_t total_sectors;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t heads;
    uint32_t hidden_sectors;
    uint32_t large_sector_count;

    uint32_t sectors_per_fat_32;
    uint16_t external_flags;
    uint16_t file_system_version;
    uint32_t root_cluster;
    uint16_t file_system_info;
    uint16_t bk_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
}__attribute__((packed));
*/
/*
static FAT32BootSector bootSector ={
    .boot_jump_instrution = {
        0xeb, 0x00, 0x90
    },
    .oem_identifier = {
        'M', 'S', 'W', 'I', 'N', '4', '.', '1'
    },
    .bytes_per_Sector = BYTE_PER_SECTOR,
    .sectors_per_cluster = SECTORS_PER_CLUSTER,
    .reserved_sectors = RESERVED_SECTORS,
    .fat_count = FAT_COUNT,
    .directory_entry_count = DIRECTORY_ENTRY_COUNT,
    .total_sectors = TOTAL_SECTORS,
    .media_descriptor_type = MEDIA_DESCRIPTOR_TYPE,
    .sectors_per_fat = SECTORS_PER_FAT,
    .sectors_per_track = SECTORS_PER_TRACK,
    .heads = HEADS,
    .hidden_sectors = HIDDEN_SECTORS,
    .large_sector_count = LARGE_SECTOR_COUNT,
    
    .sectors_per_fat_32 = SECTORS_PER_FAT_32,
    .external_flags = EXTERNAL_FLAGS,
    .file_system_version = VERSION,
    .root_cluster = ROOT_CLUSTER,
    .file_system_info = FS_INFO,
    .bk_boot_sector = BK_BOOT_SECTOR,
    .reserved = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    },
    .drive_number = DRIVE_NUMBER,
    .reserved1 = RESERVED1,
    .boot_signature = BOOT_SIGNATURE,
    .volume_id = 0,
    .volume_label = {
        'N', 'O', ' ', 'N', 'A', 'M', 'E', ' ', ' ', ' ', ' '
    },
    .file_system_type = {
        'F', 'A', 'T', '3', '2', ' ', ' ', ' '
    }
};
*/

/**
 * Struct containing the value of bytes in a cluster
 * 
 * @param buf           Array of each byte
 */
struct ClusterBuffer{
    uint8_t buf[CLUSTER_SIZE];
}__attribute__((packed));

/**
 * Struct containing the FAT table
 * 
 * @param sector_next   Array of next cluster location
 */
struct FAT32FileAllocationTable{
    uint32_t sector_next[CLUSTER_COUNT];
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
struct FAT32DriverRequest{
    void* buf;
    char name[8];
    char ext[3];
    uint32_t parent_cluster_number;
    uint32_t buffer_size;
}__attribute__((packed));

/**
 * Struct for reading files
 * 
 * @param cluster_count         number of clusters containing the read file
 * @param size                  actual size of the file
 * @param content               pointer to the actual content of the file divided to clusters
 */
struct FAT32FileReader{
    uint32_t cluster_count;
    uint32_t size;
    struct ClusterBuffer* content;
}__attribute__((packed));

/**
 * Struct for reading folders
 * 
 * @param cluster_count         number of clusters containing the read folder
 * @param content               pointer to the actual content of the folder divided to clusters
 */
struct FAT32DirectoryReader{
    uint32_t cluster_count;
    struct DirectoryTable* content;
}__attribute__((packed));

/**
 * Check the bootsector of a drive
 * 
 * @return 1 if boot sector is empty, 0 if not
 */
uint8_t is_empty_storage();

/**
 * If drive is empty, initialize boot sector, FAT table, and root directory
 * Else do nothing
 * 
 */
void initialize_filesystem_fat32();

/**
 * Create a new fat directory entry in a directory cluster
 * 
 * @param request
 * @param cluster_number
 */
void create_entry(struct FAT32DriverRequest request, uint16_t cluster_number, struct FAT32FileAllocationTable* fat);

/**
 * Get fat directory entry of request, request must exist in the folder beforehand
 * 
 * @param request                   requested file
 * 
 * @return                          fat entry of request
 */
struct DirectoryEntry get_self_entry(struct FAT32DriverRequest request);

/**
 * Copies a reader buffer and returns it as a directory table
 *  Seems redundant but quite useful in a lot of cases. Might be a subject to refactor.
 * 
 * @param reader                    pointer to a reader buffer
 * 
 * @return                          reader as directory table, contents are copied and not as a casted pointer
 */
struct DirectoryTable as_directory(uint32_t* reader);

/**
 * Translates cluster number to index in lba
 * 
 * @param cluster                   cluster number
 * 
 * @return                          cluster number in lba
 */
int cluster_to_lba(int clusters);


/**
 * Deletes a fat directory entry in a directory cluster
 * 
 * @param request                   requested file
 * 
 * @return                          Status of deletion, 0 is success, other than 0 are error codes
 */
uint8_t delete(struct FAT32DriverRequest request);

/**
 * Inner recursive function for delete
 * 
 * @param cluster_number            cluster of the folder
 */
void deleteFolder(uint16_t cluster_number, struct FAT32FileAllocationTable* fat);

/**
 * Read clusters from the disk
 * 
 * @param reader                    destination buffer to store the read data
 * @param cluster                   cluster index to read
 * @param sector_count              number of clusters to read
 */
void read_clusters(void* reader, uint16_t cluster, uint16_t sector_count);

/**
 * Write clusters to the disk
 * 
 * @param writer                    source buffer of data that is going to be written
 * @param cluster                   cluster index to write
 * @param sector_count              number of clusters to write
 */
void write_clusters(void* writer, uint16_t cluster, uint16_t sector_count);

/**
 * Read a file from a request
 * @warning                         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param request                   requested file
 * 
 * @return                          a struct containing the read data and number of read clusters
 */
struct FAT32FileReader read(struct FAT32DriverRequest request);

/**
 * Read a folder from a request
 * @warning                         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param request                   requested folder
 * 
 * @return                          a struct containing the read data and number of read clusters
 */
struct FAT32DirectoryReader read_directory(struct FAT32DriverRequest request);

/**
 * Reads a cluster as a folder without input handling
 * @warning                         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param cluster_number            requested folder cluster number
 * 
 * @return                          a struct containing the read data and number of read clusters
 */
struct FAT32DirectoryReader self_directory_info(uint32_t cluster_number);

/**
 * Unallocates file reader buffer
 * @warning                         this function is for allocated pointers, do not use on unallocated pointers
 * 
 * @param pointer                   an allocated file reader pointer
 * 
 */
void close_file(struct FAT32FileReader pointer);

/**
 * Unallocates folder reader buffer
 * @warning                         this function is for allocated pointers, do not use on unallocated pointers
 * 
 * @param pointer                   an allocated folder reader pointer
 * 
 */
void close_directory(struct FAT32DirectoryReader pointer);

/**
 * Copies a requested file to a requested address
 * @details this function is referred to by the specification as the read function
 *          due to malloc being possible and the second milestone was done without the guidebook
 *          the read function in this operating system was modified to copy the file into heap memory
 *          this function was made to compensate for the missing functionalities of the original function
 *          Might cause some confusion here and there, sorry for the inconvenience.
 * 
 * @param request                   requested file
 *                                  request.buf is the requested buffer destination
 * 
 * @return                          0 means success, anything other than 0 are error codes
 */
uint8_t load(struct FAT32DriverRequest request);

/**
 * Writes a requested file to the filesystem
 * 
 * @param request                   requested file
 * 
 * @return                          0 means success, anything other than 0 are error codes
 */
uint8_t write(struct FAT32DriverRequest request);

/**
 * Expands folder to a new cluster
 * 
 * @param cluster_number            last cluster of the folder
 * 
 * @return                          cluster index of the expansion
 */
uint32_t expand_folder(int cluster_number, struct FAT32FileAllocationTable* fat);

/**
 * Updates file time modified data by inserting CMOS data
 * @warning reading of CMOS is not done inside this function in case of a complex long running operation,
 *          be sure to call read_cmos before updating file time
 * 
 * @param entry                     FAT entry to be updated
 * 
 */
void update_file_time(struct DirectoryEntry *entry);

/**
 * Updates file size
 * 
 * @param entry                     FAT entry to be updated
 * @param size                      size to be added or subtracted, 0 will be converted to 32 as folders are treated to use 32 bytes of memory
 * @param category                  used to categorize addition or subtraction. '+' and '-' respectively
 * 
 */
void update_file_size(struct DirectoryEntry* entry, uint32_t size, char category);

/**
 * Checks whether a cluster is a directory, uses reserved bits and directory flags, still might not be 100% accurate
 * 
 * @param cluster                   Cluster to check
 * 
 * @return                          1 if is a folder, 0 if not
 */
uint8_t is_directory(uint32_t cluster);

/**
 * Checks whether an entry name and extension already exists in the parent folder
 * 
 * @param request                   requested file to check
 * 
 * @return                          1 if is a folder, 0 if not
 */
uint8_t name_exists(struct FAT32DriverRequest request);

/**
 * Searches for all files in a given folder with the same name and extension as the requested file
 * 
 * @param cluster_number        the cluster number of the parent folder
 * @param result_array          an array to hold the results
 * @param result_count          a pointer to a variable to store the number of results found
 * 
 * @return                      none
 */
// void whereis(uint16_t cluster_number, FAT32DriverRequest* result_array, uint16_t* result_count);

//TODO: Document
void write_fat(struct FAT32FileAllocationTable* FAT_request);
void read_fat(struct FAT32FileAllocationTable* FAT_destination);
void deleteRecurse(struct FAT32DriverRequest request, struct FAT32FileAllocationTable* fat);
void init_directory_table(
    struct FAT32DriverRequest request,
    uint16_t cluster_number,
    uint16_t parent_actual_cluster,
    uint16_t entry_number
);

struct entryflags{
    uint8_t read_only : 1 ;
    uint8_t hidden : 1 ;
    uint8_t system : 1 ;
    uint8_t volume_id : 1 ;
    uint8_t directory : 1 ;
    uint8_t archive : 1 ;
    uint8_t resbit1 : 1 ;
    uint8_t resbit2 : 1 ;
};

void set_entry_flag(struct FAT32DriverRequest request, struct entryflags flag);

#endif