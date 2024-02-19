
#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

#include "stdio.h"
#include "shell.h"

/**
 * Prints information of current cluster
 * 
 * @param clusterCluster            current folder cluster number
 */
void dir(uint32_t currentCluster);

/**
 * Prints information of current cluster
 * 
 * @param clusterCluster            current folder cluster number
 */
void ls(uint32_t currentCluster);

/**
 * Move directory
 * 
 * @param pathname         Pathname to be checked
 * @param directory_info   Struct directory that it is on
 * 
 * @return      If in is empty, returns 1. 0 otherwise
 */
void cd(char* pathname, struct directory_info* directory_info);

void rm(uint32_t currentCluster);

void cat(uint32_t currentCluster);

void cp(uint32_t currentCluster);

void mv(uint32_t currentCluster);

void copy1Folder(struct FAT32DriverRequest src, struct FAT32DriverRequest dest);
void copy1File(struct FAT32DriverRequest src, struct FAT32DriverRequest dest);

/**
 * Creates new folder
 */
void mkdir(char *dirname, uint32_t currentCluster);

void whereis(uint16_t current_cluster, char* filename, char* path);
void exec(struct FAT32DriverRequest* req);
void ps();
void kill(__attribute__((unused)) uint32_t pid);

#endif