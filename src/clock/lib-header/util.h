#ifndef COMMANDS_UTIL
#define COMMANDS_UTIL

#include "stdtype.h"
#include "stdio.h"

/**
 * Returns request to file from a path string
 * 
 * @param pathname          Pathname to be checked
 * @param current_cluster   The cluster that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
FAT32DriverRequest path_to_file_request(char* pathname, uint32_t current_cluster);

/**
 * Reads current cluster as a folder
 * @warning                         this function uses malloc, be sure to close the reader afterwards
 * 
 * @param cluster_number            current folder cluster number
 * 
 * @return                          a struct containing the read data and number of read clusters
 */
FAT32DirectoryReader get_dir_info(uint32_t current_cluster);

#endif