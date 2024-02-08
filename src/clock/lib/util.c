#include "../lib-header/util.h"

#include "../lib-header/string.h"
#include "../lib-header/stdio.h"
#include "../lib-header/cmos.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/syscall.h"

#include "../lib-header/parser.h"

FAT32DriverRequest path_to_file_request(char* pathname, uint32_t current_cluster) {
    parser_t pathparser = {0};
    parser_parse(&pathparser, pathname, '/');

    uint8_t isFound = 0;
    int pathLength = pathparser.word_count;
    int counter = 0;

    char fileName[8] = {0};
    char fileExt[3] = {0};

    int dotIdx  = -1;
    int fileNameExtLen = strlen(pathparser.content[pathLength - 1]);

    FAT32DriverRequest emptyReq = {};
    
    // search .
    for (int i = fileNameExtLen - 1; i >= 0; i--) {
        if (pathparser.content[pathLength - 1][i] == '.') {
            dotIdx = i;
            break;
        }
    }

    if (dotIdx == -1) { // dot not found, not a valid file name
        return emptyReq;
    } else {
        // split into file name and file ext
        int minNameLen = 8;
        if (dotIdx < minNameLen) {
            minNameLen = dotIdx;
        }

        int minExtLen = 3;
        if (fileNameExtLen - 1 - dotIdx < minExtLen) {
            minExtLen = fileNameExtLen - dotIdx - 1;
        }

        for (int i = 0; i < minNameLen; i++) {
            fileName[i] = pathparser.content[pathLength - 1][i];
        }

        for (int i = 0; i < minExtLen; i++) {
            fileExt[i] = pathparser.content[pathLength - 1][dotIdx + 1 + i];
        }
    }
    
    if (strcmp(pathparser.content[0], "root") == 0){
        current_cluster = ROOT_CLUSTER_NUMBER;
        counter = 1;
    }

    FAT32DirectoryReader read;
    char emptyString[9] = {0};
    char string[9] = {0};
    char extstring[4] = {0};

    for(uint32_t j = counter; j < pathparser.word_count - 1; j++){
        isFound = 0;
        
        read = get_dir_info(current_cluster);
        for(uint32_t k = 0; k < read.cluster_count; k++){
            for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                memcpy(string, emptyString, 8);
                memcpy(string, &read.content[k].entry[i].filename, 8);
                if ((strcmp(string, pathparser.content[j]) == 0)){
                    if (read.content[k].entry[i].directory){
                        current_cluster = read.content[k].entry[i].cluster_number;
                        isFound = 1;
                        break;
                    }
                }
            }
            if(isFound){
                break;
            }
        }
        closef_dir(&read);
    }

    uint32_t parentCluster = current_cluster;
    uint32_t size = 0;

    if (isFound || pathLength <= 2) {
        isFound = 0;

        read = get_dir_info(current_cluster);
        for(uint32_t k = 0; k < read.cluster_count; k++){
            for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                memcpy(string, emptyString, 8);
                memcpy(string, &read.content[k].entry[i].filename, 8);
                memcpy(extstring, emptyString, 3);
                memcpy(extstring, &read.content[k].entry[i].extension, 3);
                if ((memcmp(string, fileName, 8) == 0) && (memcmp(extstring, fileExt, 3) == 0)){
                    if (!read.content[k].entry[i].directory){
                        current_cluster = read.content[k].entry[i].cluster_number;
                        size = read.content[k].entry[i].size;
                        isFound = 1;
                        break;
                    }
                }
            }
            if(isFound){
                break;
            }
        }
        closef_dir(&read);
    }

    FAT32DriverRequest req = {
        .parent_cluster_number = parentCluster,
        .buffer_size = size
    };

    memcpy(req.name, fileName, 8);
    memcpy(req.ext, fileExt, 3);

    parser_clear(&pathparser);
    return req;
}

FAT32DirectoryReader get_dir_info(uint32_t current_cluster){
    FAT32DirectoryReader retval;
    syscall(SYSCALL_SELF_DIR_INFO, current_cluster, (uint32_t) &retval, 0);
    return retval;
}
