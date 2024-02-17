#ifndef COMMANDS_UTIL
#define COMMANDS_UTIL

#include "stdtype.h"
#include "stdio.h"

#include "shell.h"

/**
 * Compares a directory entry with an empty directory
 * 
 * @param in    DirectoryEntry to be compared
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
uint8_t is_entry_empty(struct DirectoryEntry in);

/**
 * Check if the path is valid
 * 
 * @param pathname          Pathname to be checked
 * @param current_cluster   The cluster that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
uint8_t is_directorypath_valid(char* pathname, uint32_t current_cluster);

/**
 * Check if the path is valid
 * 
 * @param pathname          Pathname to be checked
 * @param current_cluster   The cluster that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
uint8_t is_filepath_valid(char* pathname, uint32_t current_cluster);

/**
 * Returns cluster from a path string
 * 
 * @param pathname          Pathname to be checked
 * @param current_cluster   The cluster that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
uint32_t path_to_cluster(char* pathname, uint32_t current_cluster);

/**
 * Returns request to file from a path string
 * 
 * @param pathname          Pathname to be checked
 * @param current_cluster   The cluster that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
struct FAT32DriverRequest path_to_file_request(char* pathname, uint32_t current_cluster);

/**
 * Returns request to file from a path string
 * 
 * @param pathname          Pathname to be checked
 * @param current_cluster   The cluster that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
struct FAT32DriverRequest path_to_dir_request(char* pathname, uint32_t current_cluster);

/**
 * Reads current cluster as a folder
 * @warning                         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param cluster_number            current folder cluster number
 * 
 * @return                          a struct containing the read data and number of read clusters
 */
struct FAT32DirectoryReader get_dir_info(uint32_t current_cluster);

//TODO: document
uint8_t check_contain(uint32_t cluster_child, uint32_t cluster_parent);
struct DirectoryEntry get_info(struct FAT32DriverRequest request);
uint8_t copy_create_folders(char* path, uint32_t currentCluster, uint8_t named);
uint8_t is_filename(char* filename);

#endif