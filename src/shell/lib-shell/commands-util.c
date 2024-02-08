#include "../lib-header/commands-util.h"

#include "../lib-header/stdio.h"
#include "../lib-header/string.h"
#include "../lib-header/time.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/syscall.h"

#include "../lib-header/parser.h"
#include "../lib-header/shell.h"

uint8_t is_entry_empty(DirectoryEntry in){
    char* checker = (char*) &in;
    for(uint32_t i = 0; i < sizeof(DirectoryEntry); i++){
        if(checker[i] != 0) return 0;
    }
    return 1;
}

uint8_t is_directorypath_valid(char* pathname, uint32_t current_cluster){
    uint8_t counter = 0;
    uint8_t isFound = 1;
    
    parser_t pathparser = {0};
    parser_parse(&pathparser, pathname, '/');
    if (strcmp(pathparser.content[0], "root") == 0){
        current_cluster = ROOT_CLUSTER_NUMBER;
        counter = 1;
    } 

    FAT32DirectoryReader read;
    char emptyString[9] = {0};
    char string[9] = {0};


    for(uint32_t j = counter; j < pathparser.word_count; j++){
        read = get_dir_info(current_cluster);
        if (strcmp(pathparser.content[j], "..") == 0){
            current_cluster = read.content[0].info.parent_base_cluster;
        }
        else{
            isFound = 0;
            for(uint32_t k = 0; k < read.cluster_count; k++){
                for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                    memcpy(string, emptyString, 8);
                    memcpy(string, &read.content[k].entry[i].filename, 8);
                    if ((strcmp(string, pathparser.content[j]) == 0)){
                        // print("\n");
                        // print(table.entry[i].filename);
                        if (read.content[k].entry[i].directory){
                            current_cluster = read.content[k].entry[i].cluster_number;
                            isFound = 1;
                            //print("\nfound!");
                            break;
                        }
                    }
                }
                if(isFound){
                    break;
                }
            }
            if(!isFound){
                return isFound;        
            }
        }
        closef_dir(&read);
    }
    parser_clear(&pathparser);
    return isFound;
}

uint8_t is_filepath_valid(char* pathname, uint32_t current_cluster){    
    parser_t pathparser = {0};
    parser_parse(&pathparser, pathname, '/');

    uint8_t isFound = 0;
    int pathLength = pathparser.word_count;
    int counter = 0;

    char fileName[8] = {0};
    char fileExt[3] = {0};

    int dotIdx  = -1;
    int fileNameExtLen = strlen(pathparser.content[pathLength - 1]);
    
    // search .
    for (int i = fileNameExtLen - 1; i >= 0; i--) {
        if (pathparser.content[pathLength - 1][i] == '.') {
            dotIdx = i;
            break;
        }
    }

    if (dotIdx == -1) { // dot not found, not a valid file name
        return 0;
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
            if(pathparser.content[pathLength - 1][i] == 0){
                break;
            }
            fileName[i] = pathparser.content[pathLength - 1][i];
        }

        for (int i = 0; i < minExtLen; i++) {
            if(pathparser.content[pathLength - 1][dotIdx + 1 + i] == 0){
                break;
            }
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

    for(int j = counter; j < pathLength - 1; j++){ // cari sampe ujung - 1
        isFound = 0;
        
        read = get_dir_info(current_cluster);
        for(uint32_t k = 0; k < read.cluster_count; k++){
            for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                memcpy(string, emptyString, 8);
                memcpy(string, &read.content[k].entry[i].filename, 8);
                if ((strcmp(string, pathparser.content[j]) == 0)){
                    // print("\n");
                    // print(table.entry[i].filename);
                    if (read.content[k].entry[i].directory){
                        current_cluster = read.content[k].entry[i].cluster_number;
                        isFound = 1;
                        //print("\nfound! zzzz");
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

    if (isFound || pathLength <= 2) {
        isFound = 0;

        read = get_dir_info(current_cluster);
        for(uint32_t k = 0; k < read.cluster_count; k++){
            for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                memcpy(string, emptyString, 8);
                memcpy(string, &read.content[k].entry[i].filename, 8);
                memcpy(extstring, emptyString, 3);
                memcpy(extstring, &read.content[k].entry[i].extension, 3);
                if ((memcmp(string, fileName, 8) == 0) && (memcmp(extstring, fileExt, 3) == 0) && !(read.content[k].entry[i].directory)){
                    isFound = 1;
                   // print("\nfound! hahaya");
                    break;
                }
            }
            if(isFound){
                break;
            }
        }
        closef_dir(&read);
    }
    parser_clear(&pathparser);
    return isFound;
}

uint32_t path_to_cluster(char* pathname, uint32_t current_cluster){
    // prekondisi: path sudah valid
    parser_t pathparser = {0};
    parser_parse(&pathparser, pathname, '/');
    
    uint8_t counter = 0;
    uint8_t isFound = 0;


    if (strcmp(pathparser.content[0], "root") == 0){
        current_cluster = ROOT_CLUSTER_NUMBER;
        counter = 1;
    }

    FAT32DirectoryReader read;
    char emptyString[9] = {0};
    char string[9] = {0};

    for(uint32_t j = counter; j < pathparser.word_count; j++){
        read = get_dir_info(current_cluster);
        if (strcmp(pathparser.content[j], "..") == 0){
            current_cluster = read.content[0].info.parent_base_cluster;
        }
        else{
            isFound = 0;
            
            for(uint32_t k = 0; k < read.cluster_count; k++){
                for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                    memcpy(string, emptyString, 8);
                    memcpy(string, &read.content[k].entry[i].filename, 8);
                    if ((strcmp(string, pathparser.content[j]) == 0)){
                        // print("\n");
                        // print(table.entry[i].filename);
                        if (read.content[k].entry[i].directory){
                            current_cluster = read.content[k].entry[i].cluster_number;
                            isFound = 1;
                            //print("\nfound!");
                            break;
                        }
                    }
                }
                if(isFound){
                    break;
                }
            }
        }
        closef_dir(&read);
    }
    parser_clear(&pathparser);
    return current_cluster;
}

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
                    // print("\n");
                    // print(table.entry[i].filename);
                    if (read.content[k].entry[i].directory){
                        current_cluster = read.content[k].entry[i].cluster_number;
                        isFound = 1;
                        //print("\nfound!");
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
                    // print("\n");
                    // print(table.entry[i].filename);
                    if (!read.content[k].entry[i].directory){
                        current_cluster = read.content[k].entry[i].cluster_number;
                        size = read.content[k].entry[i].size;
                        isFound = 1;
                        //print("\nfound! 0000");
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

FAT32DriverRequest path_to_dir_request(char* pathname, uint32_t current_cluster) {
    //prekondisi: path to dir valid

    parser_t pathparser = {0};
    parser_parse(&pathparser, pathname, '/');

    int counter = 0;
    uint8_t isFound = 0;
    uint32_t parentCluster = current_cluster;

    if (strcmp(pathparser.content[0], "root") == 0){
        current_cluster = ROOT_CLUSTER_NUMBER;
        counter = 1;
    } 

    FAT32DirectoryReader read;
    char emptyString[9] = {0};
    char string[9] = {0};

    for(uint32_t j = counter; j < pathparser.word_count; j++){
        isFound = 0;
        
        read = get_dir_info(current_cluster);
        for(uint32_t k = 0; k < read.cluster_count; k++){
            for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                memcpy(string, emptyString, 8);
                memcpy(string, &read.content[k].entry[i].filename, 8);
                if ((strcmp(string, pathparser.content[j]) == 0)){
                    // print("\n");
                    // print(table.entry[i].filename);
                    if (read.content[k].entry[i].directory){
                        current_cluster = read.content[k].entry[i].cluster_number;
                        
                        if (j != pathparser.word_count - 1) {
                            parentCluster = current_cluster;
                        }

                        isFound = 1;
                        //print("\nfound!");
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
        .buffer_size = 0
    };


    for(uint8_t i = 0; i < 8; i++){
        if(pathparser.content[pathparser.word_count - 1][i] == 0){
            break;
        }
        req.name[i] = pathparser.content[pathparser.word_count - 1][i];
    }
    
    parser_clear(&pathparser);
    return req;
}

FAT32DirectoryReader get_dir_info(uint32_t current_cluster){
    FAT32DirectoryReader retval;
    syscall(SYSCALL_SELF_DIR_INFO, current_cluster, (uint32_t) &retval, 0);
    return retval;
}

uint8_t check_contain(uint32_t cluster_child, uint32_t cluster_parent){
    if(cluster_child == ROOT_CLUSTER_NUMBER) return 0;
    if(cluster_parent == ROOT_CLUSTER_NUMBER) return 1;
    else{
        uint32_t traversal_cluster = cluster_child;
        FAT32DirectoryReader read = get_dir_info(traversal_cluster);

        while (traversal_cluster != ROOT_CLUSTER_NUMBER)
        {
            if(traversal_cluster == cluster_parent){
                closef_dir(&read);
                return 1;
            }
            traversal_cluster = read.content[0].info.parent_base_cluster;
            read = get_dir_info(traversal_cluster);
        }

        closef_dir(&read);
        return 0;
        
    }

}

DirectoryEntry get_info(FAT32DriverRequest request){
    FAT32DirectoryReader read = get_dir_info(request.parent_cluster_number);
    DirectoryEntry self;
    for(uint32_t i = 0; i < read.cluster_count; i++){
        for(uint32_t j = 0; j < ENTRY_COUNT; j++){
            // read.content[i].entry[j];
            if(memcmp(&read.content[i].entry[j].filename, request.name, 8) == 0 &&
                memcmp(&read.content[i].entry[j].extension, request.ext, 3) == 0
            ){
                self = read.content[i].entry[j];
                break;
            }
        }
    }
    closef_dir(&read);

    return self;
}

uint8_t copy_create_folders(char* path, uint32_t currentCluster, uint8_t named){    
    parser_t pathparser = {0};
    parser_parse(&pathparser, path, '/');
    
    uint8_t counter = 0;
    uint8_t isFound = 1;
    
    if (pathparser.word_count - 1 < 1){
        parser_clear(&pathparser);
        return 0;
    }

    uint32_t current_cluster = currentCluster;
    if (strcmp(pathparser.content[0], "root") == 0){
        current_cluster = ROOT_CLUSTER_NUMBER;
        counter = 1;
    } 
    FAT32DirectoryReader read;
    char emptyString[9] = {0};
    char string[9] = {0};
    while (counter < pathparser.word_count - 1 && isFound){
        read = get_dir_info(current_cluster);
        if (strcmp(pathparser.content[counter], "..") == 0){
            current_cluster = read.content[0].info.parent_base_cluster;;
            counter++;
        }
        else{
            isFound = 0;
            for(uint32_t k = 0; k < read.cluster_count; k++){
                for(uint8_t i= 0; i < ENTRY_COUNT; i++){
                    memcpy(string, emptyString, 8);
                    memcpy(string, &read.content[k].entry[i].filename, 8);
                    if ((strcmp(string, pathparser.content[counter]) == 0)){
                        if (read.content[k].entry[i].directory){
                            current_cluster = read.content[k].entry[i].cluster_number;
                            isFound = 1;
                            //print("\nfound!");
                            break;
                        }
                    }
                }
                if(isFound){
                    counter++;
                    break;
                }
            }
        }
        closef_dir(&read);
    }

    while (counter < pathparser.word_count - named){
        FAT32DriverRequest req = {0};
        req.parent_cluster_number = current_cluster;
        req.buffer_size = 0;
        for(uint8_t i = 0; i < 8; i++){
            if(pathparser.content[counter][i] == 0){
                break;
            }
            req.name[i] = pathparser.content[counter][i];
        }
        char forbidden[8] = "..\0\0\0\0\0\0";
        if(memcmp(forbidden, req.name, 8) == 0){
            print("\ncp: Operation halted because of forbidden folder name (..)\n");
            return 1;
        }
        writef(&req);
        read = get_dir_info(current_cluster);
        DirectoryEntry self;
        for(uint32_t i = 0; i < read.cluster_count; i++){
            for(uint32_t j = 0; j < ENTRY_COUNT; j++){
                if(memcmp(&read.content[i].entry[j].filename, req.name, 8) == 0 &&
                    memcmp(&read.content[i].entry[j].extension, req.ext, 3) == 0
                ){
                    self = read.content[i].entry[j];
                    break;
                }
            }
        }
        current_cluster = self.cluster_number;
        counter++;
        closef_dir(&read);
    }
    parser_clear(&pathparser);
    return 0;
}

uint8_t is_filename(char* filename) {
    int dotIdx  = -1;
    int fileNameExtLen = strlen(filename);

    // search .
    for (int i = fileNameExtLen - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            dotIdx = i;
            break;
        }
    }

    if (dotIdx == -1) { // dot not found, not a valid file name
        return 0;
    } else {
        return 1;
    }
}