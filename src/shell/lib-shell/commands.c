
#include "../lib-header/commands.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/stdio.h"
#include "../lib-header/string.h"
#include "../lib-header/time.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/syscall.h"
#include "../lib-header/commands-util.h"
#include "../lib-header/iostream.h"

#include "../lib-header/parser.h"
#include "../lib-header/shell.h"
#include "../lib-header/commands-util.h"

extern shell_app shell;
extern parser_t sh_parser;

void delay(uint32_t ms){
    uint32_t currentTick = 0;
    uint32_t cachedTick = 0;
    syscall(SYSCALL_GET_TICK, (uint32_t) &currentTick, 0, 0);
    cachedTick = currentTick + ms;

    while (currentTick < cachedTick){
        syscall(SYSCALL_GET_TICK, (uint32_t) &currentTick, 0, 0);
    }
}

void dir(uint32_t currentCluster){
    FAT32DirectoryReader directory_reader;

    directory_reader = get_dir_info(currentCluster);
    char char_buffer[9];
    uint32_t counter = 1;
    DirectoryEntry read_entry;

    string_t string = str_new("\n    No   Name        Ext    Size      Creation time");
    for(uint32_t i = 0; i < directory_reader.cluster_count; i++){
        for(uint16_t j = 0; j < ENTRY_COUNT; j++){
            read_entry = directory_reader.content[i].entry[j];
            
            if(!is_entry_empty(read_entry)){
                str_add(&string, "\n    ");
                int_to_string(counter, char_buffer);
                str_add(&string, char_buffer);
                str_addc(&string, '.');
                for(int i = strlen(char_buffer); i < 4; i++){
                    str_addc(&string, ' ');
                }

                for(int j = 0; j < 8; j++){
                    if(read_entry.filename[j] == 0) str_addc(&string, ' ');
                    else str_addc(&string, read_entry.filename[j]);
                }

                str_add(&string, "    ");
                for(int j = 0; j < 3; j++){
                    if(read_entry.extension[j] == 0) str_addc(&string, ' ');
                    else str_addc(&string, read_entry.extension[j]);
                }
                
                str_add(&string, "    ");
                int_to_string(read_entry.size, char_buffer);
                str_add(&string, char_buffer);

                for(int i = strlen(char_buffer); i < 10; i++){
                    str_addc(&string, ' ');
                }

                int_to_string(read_entry.creation_time_hours, char_buffer);
                str_add(&string, char_buffer);
                str_addc(&string, ':');
                int_to_string(read_entry.creation_time_minutes, char_buffer);
                str_add(&string, char_buffer);
                str_addc(&string, ':');
                int_to_string(read_entry.creation_time_seconds, char_buffer);
                str_add(&string, char_buffer);

                str_add(&string, "    ");
                int_to_string(read_entry.creation_time_day, char_buffer);
                str_add(&string, char_buffer);
                str_addc(&string, '/');
                int_to_string(read_entry.creation_time_month, char_buffer);
                str_add(&string, char_buffer);
                str_addc(&string, '/');
                time current_time = get_time();
                int_to_string((read_entry.creation_time_year) + current_time.century * 100, char_buffer);
                str_add(&string, char_buffer);

                counter++;
            }
        }
    }
    sout_t sout = sout_newstr(string);
    
    sout_printall_ws(&sout);

    sout_clear(&sout);
    str_delete(&string);
    closef_dir(directory_reader);
}

// void mkdir(char *dirname, uint32_t currentCluster){
//     uint8_t counter = 0;
//     uint8_t isFound = 1;
    
//     uint32_t current_cluster = currentCluster;

//     parser_t pathparser = {0};
//     parse(&pathparser, dirname, '/');
//     if (strcmp(pathparser.content[0], "root") == 0){
//         current_cluster = 2;
//         counter = 1;
//     } 

//     FAT32DirectoryReader read;
//     char emptyString[9] = {0};
//     char string[9] = {0};

//     while (counter < pathparser.word_count && isFound){
//         read = get_dir_info(current_cluster);
//         if (strcmp(pathparser.content[counter], "..") == 0){
//             current_cluster = read.content[0].entry[0].cluster_number;
//             counter++;
//         }
//         else{
//             isFound = 0;
//             for(uint32_t k = 0; k < read.cluster_count; k++){
//                 for(uint8_t i= 0; i < ENTRY_COUNT; i++){
//                     memcpy(string, emptyString, 8);
//                     memcpy(string, &read.content[k].entry[i].filename, 8);
//                     if ((strcmp(string, pathparser.content[counter]) == 0)){
//                         if (read.content[k].entry[i].directory){
//                             current_cluster = read.content[k].entry[i].cluster_number;
//                             isFound = 1;
//                             //print("\nfound!");
//                             break;
//                         }
//                     }
//                 }
//                 if(isFound){
//                     counter++;
//                     break;
//                 }
//             }
//         }
//         closef_dir(read);
//     }

//     if(isFound){
//         print("\nmkdir: Directory already exists\n");
//     }
//     else{
//         while (counter < pathparser.word_count){
//             FAT32DriverRequest req = {0};
//             req.parent_cluster_number = current_cluster;
//             req.buffer_size = 0;

//             for(uint8_t i = 0; i < 8; i++){
//                 if(pathparser.content[counter][i] == 0){
//                     break;
//                 }
//                 req.name[i] = pathparser.content[counter][i];
//             }

//             char forbidden[8] = "..\0\0\0\0\0\0";
//             if(memcmp(forbidden, req.name, 8) == 0){
//                 print("\nmkdir: Operation halted because of forbidden folder name (..)\n");
//                 return;
//             }

//             writef(req);
            
//             read = get_dir_info(current_cluster);
//             DirectoryEntry self;
//             for(uint32_t i = 0; i < read.cluster_count; i++){
//                 for(uint32_t j = 0; j < ENTRY_COUNT; j++){
//                     if(memcmp(&read.content[i].entry[j].filename, req.name, 8) == 0 &&
//                         memcmp(&read.content[i].entry[j].extension, req.ext, 3) == 0
//                     ){
//                         self = read.content[i].entry[j];
//                         break;
//                     }
//                 }
//             }
//             current_cluster = self.cluster_number;
//             counter++;

//             closef_dir(read);
//         }


//         print("\nDirectory created successfully\n");
//     }
//     parser_clear(&pathparser);
// }

// DirectoryEntry emptyEntry = {0};

// void whereis(uint16_t current_cluster, char* filename, char* path){
//     char fileName[8] = {0};
//     char fileExt[3] = {0};

//     int dotIdx  = -1;
//     int fileNameExtLen = strlen(filename);
    
//     // search .
//     for (int i = fileNameExtLen - 1; i >= 0; i--) {
//         if (filename[i] == '.') {
//             dotIdx = i;
//             break;
//         }
//     }

//     if (dotIdx == -1) { // dot not found, not a valid file name
//         for (int i = 0; i < 8; i++) {
//             if(filename[i] == 0){
//                 break;
//             }
//             fileName[i] = filename[i];
//         }
//     } else {
//         // split into file name and file ext
//         int minNameLen = 8;
//         if (dotIdx < minNameLen) {
//             minNameLen = dotIdx;
//         }

//         for (int i = 0; i < minNameLen; i++) {
//             if(filename[i] == 0){
//                 break;
//             }
//             fileName[i] = filename[i];
//         }

//         int minExtLen = 3;
//         if (fileNameExtLen - 1 - dotIdx < minExtLen) {
//             minExtLen = fileNameExtLen - dotIdx - 1;
//         }
//         for (int i = 0; i < minExtLen; i++) {
//             if(filename[dotIdx + 1 + i] == 0){
//                 break;
//             }
//             fileExt[i] = filename[dotIdx + 1 + i];
//         }
//     }

//     FAT32DirectoryReader read;
//     char emptyString[9] = {0};
//     char string[9] = {0};
//     char extstring[4] = {0};


//     read = get_dir_info(current_cluster);
//     for(uint32_t k = 0; k < read.cluster_count; k++){
//         for(uint8_t i= 0; i < ENTRY_COUNT; i++){
//             memcpy(string, emptyString, 8);
//             memcpy(string, &read.content[k].entry[i].filename, 8);
//             memcpy(extstring, emptyString, 3);
//             memcpy(extstring, &read.content[k].entry[i].extension, 3);
//             if (read.content[k].entry[i].directory){
                
//                 uint8_t lencounter = 0;
//                 while(lencounter < 8 && read.content[k].entry[i].filename[lencounter] != 0){
//                     lencounter++;
//                 }

//                 char* appended = (char*) malloc(strlen(path) + lencounter + 2);
                
//                 uint8_t lencounter2 = 0;
//                 while (lencounter2 < strlen(path)){
//                     appended[lencounter2] = path[lencounter2];
//                     lencounter2++;
//                 }
//                 appended[lencounter2] = '/';
//                 lencounter2++;

//                 for(uint8_t l = 0; l < lencounter; l++){
//                     appended[lencounter2] = read.content[k].entry[i].filename[l];
//                     lencounter2++;
//                 }
//                 appended[lencounter2] = '\0';


//                 whereis(read.content[k].entry[i].cluster_number, filename, appended);
//                 free(appended);
//             }

//             if ((memcmp(string, fileName, 8) == 0) && (memcmp(extstring, fileExt, 3) == 0)){
//                 string_t printstr = str_new("\n    ");
//                 str_add(&printstr, path);
//                 str_addc(&printstr, '/');
//                 str_add(&printstr, filename);
//                 print(printstr.content);
//             }

//         }
//     }
//     closef_dir(read);
// }

// void ls(uint32_t currentCluster){
//     FAT32DirectoryReader directory_reader;

//     directory_reader = get_dir_info(currentCluster);
//     // char char_buffer[9];
//     // uint32_t counter = 1;
//     DirectoryEntry read_entry;

//     string_t string = str_new("\nfiles and directories: \n");
//     for(uint32_t i = 0; i < directory_reader.cluster_count; i++){
//         for(uint16_t j = 0; j < ENTRY_COUNT; j++){
//             read_entry = directory_reader.content[i].entry[j];
            
//             if(!is_entry_empty(read_entry)){
//                 for(int j = 0; j < 8; j++){
//                     if(read_entry.filename[j] == 0){
//                         str_addc(&string, ' ');
//                     }
//                     else{
//                         str_addc(&string, read_entry.filename[j]);
//                     }
//                 }

//                 for(int k = 0; k < 3; k++){
//                     if(read_entry.extension[k] == 0){
//                         str_addc(&string, ' ');
//                     }
//                     else{
//                         if(k == 0){
//                             str_addc(&string, '.');
//                         }
//                         str_addc(&string, read_entry.extension[k]);
//                     }
//                 }


//                 // cleaning things out
//                 for(int k = 0; k < 4; k++){
//                     str_addc(&string, ' ');
//                 }
//             }
//         }
//     }

//     sout_t sout = sout_newstr(string);

//     sout_printall_ws(&sout);

//     str_delete(&string);
//     sout_clear(&sout);
// }

// void cd(char* pathname, directory_info* current_dir){
//     current_dir->cluster_number = path_to_cluster(pathname, current_dir->cluster_number);
    
//     uint32_t startleng = 0;
//     uint32_t maxleng = INPUT_BUFFER_SIZE;
//     char* appended = (char*) malloc (maxleng);

//     parser_t pathparser = {0};
//     parse(&pathparser, pathname, '/');
//     if (strcmp(pathparser.content[0], "root") == 0){
//         startleng = 5;
//         memcpy(appended, "/root", 6);

//         for(uint32_t j = 1; j < pathparser.word_count; j++){
//             if (strcmp(pathparser.content[j], "..") == 0){
//                 while (current_dir->path[startleng] != '/' && startleng > 5){
//                     startleng--;
//                 }
//                 appended[startleng] = 0;
//                 startleng--;
//             }
//             else{
//                 appended[startleng] = '/';
//                 startleng++;

//                 if (startleng > maxleng){    
//                     maxleng += INPUT_BUFFER_SIZE;
//                     appended = (char*) realloc (appended, maxleng);
//                 }

//                 for(int i = 0; i < strlen(pathparser.content[j]); i++){
//                     appended[startleng] = pathparser.content[j][i];
//                     startleng++;
                    
//                     if (startleng > maxleng){    
//                         maxleng += INPUT_BUFFER_SIZE;
//                         appended = (char*) realloc (appended, maxleng);
//                     }
//                 }
//                 appended[startleng] = 0;
//             }
//         }
//     }

//     else{
//         startleng = strlen(current_dir->path);
//         strcpy(appended, current_dir->path);
//         for(uint32_t j = 0; j < pathparser.word_count; j++){
//             if (strcmp(pathparser.content[j], "..") == 0){
//                 while (current_dir->path[startleng] != '/' && startleng > 5){
//                     startleng--;
//                 }
//                 current_dir->path[startleng] = 0;
//                 appended[startleng] = 0;
//             }
//             else{
//                 appended[startleng] = '/';
//                 startleng++;
                
//                 if (startleng > maxleng){    
//                     maxleng += INPUT_BUFFER_SIZE;
//                     appended = (char*) realloc (appended, maxleng);
//                 }

//                 for(int i = 0; i < strlen(pathparser.content[j]); i++){
//                     appended[startleng] = pathparser.content[j][i];
//                     startleng++;
                    
//                     if (startleng > maxleng){    
//                         maxleng += INPUT_BUFFER_SIZE;
//                         appended = (char*) realloc (appended, maxleng);
//                     }
//                 }
//                 appended[startleng] = 0;
//             }
//         }
//     }

//     free(current_dir->path);
//     current_dir->path = appended;
    
//     parser_clear(&pathparser);
// }

// void rm(uint32_t currentCluster) {
//     int length = shellparser.word_count;

//     if (length >= 2) {
//         if (is_directorypath_valid(shellparser.content[length - 1], currentCluster)) {
//             // TODO [minor]: check dir kosong / ngga
//             uint8_t isEmptyDir = 0; 
//             if (strcmp(shellparser.content[1],"-r") != 0 && !isEmptyDir) { // flag -r selalu di tengah
//                 print("\nrm: cannot remove '");
//                 print(shellparser.content[length - 1]);
//                 print("': Is a directory\n");
//             } else {
//                 //print("\ndianggep dir");
//                 deletef(path_to_dir_request(shellparser.content[length - 1], currentCluster));
//             }
//         } else if (is_filepath_valid(shellparser.content[length - 1], currentCluster)) {
//             //print("\ndianggep file");
//             deletef(path_to_file_request(shellparser.content[length - 1], currentCluster));
//         } else {
//             print("\nrm: Invalid command\n");
//         }
//     } else {
//         print("\nrm: Invalid command\n");
//     }
// }

// void cp(uint32_t currentCluster) {
//     int len = shellparser.word_count;
//     if (len >= 3) { // cp minimal len cmd 3
//         uint8_t isdir = 0;
//         uint8_t isfile = 0;
//         uint8_t hasR = 0;
//         uint8_t hasDir = 0;

//         FAT32DriverRequest srcs[len - 2]; // create arr of sources
//         uint8_t isFile[len - 2]; 

//         int nSrc = 0;
//         for (int i = 1; i < len - 1; i++) {
//             isdir = is_directorypath_valid(shellparser.content[i], currentCluster);
//             isfile = is_filepath_valid(shellparser.content[i], currentCluster);
//             if (strcmp(shellparser.content[i], "-r") == 0) {
//                 hasR = 1;
//             }
//             else if (isfile) {
//                 srcs[nSrc] = path_to_file_request(shellparser.content[i], currentCluster);
//                 isFile[nSrc] = 1;
//                 nSrc++;
//             }
//             else if (isdir) {
//                 srcs[nSrc] = path_to_dir_request(shellparser.content[i], currentCluster);
//                 isFile[nSrc] = 0;
//                 hasDir = 1;
//                 nSrc++;
//             }
//             else { // aman
//                 print("\ncp: Invalid path\n");
//                 return;
//             }
//         }

//         if (hasR && len == 3) {
//             print("\ncp: Destination is empty\n");
//             return;
//         }

//         if (hasDir && !hasR) {
//             print("\ncp: Source is a directory\n");
//             return;
//         }

//         // semua source valid

//         // cek apakah dest ada
//         isdir = is_directorypath_valid(shellparser.content[len - 1], currentCluster);
//         isfile = is_filepath_valid(shellparser.content[len - 1], currentCluster);

//         if (nSrc > 1) {
//             if (isfile) { // aman
//                 print("\ncp: Destination is not a directory\n");
//             }
//             else if (isdir) {
//                 for (int i = 0; i < nSrc; i++) {
//                     FAT32DriverRequest src = srcs[i];
//                     if (isFile[i]) { // aman
//                         uint32_t destCluster = path_to_cluster(shellparser.content[len - 1], currentCluster);
//                         FAT32DriverRequest dest = {
//                             .parent_cluster_number = destCluster
//                         };
//                         memcpy(dest.name, src.name, 8);
//                         memcpy(dest.ext, src.ext, 3);
//                         copy1File(src, dest);
//                     } else {
//                         FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                         DirectoryEntry srcInfo = get_info(src);
//                         if (check_contain(dest.parent_cluster_number, srcInfo.cluster_number)){
//                             print("\ncp: Cannnot copy into itself\n");
//                         }
//                         else{
//                             copy1Folder(src, dest);
//                         }
//                     }
//                 }
//             }
//             else { // aman
//                 print("\ncp: Destination is not a directory\n");
//             }
//         } else {
//             if (isfile) { // aman
//                 print("\ncp: Cannot overwrite existing file\n");
//             }
//             else if (isdir) {
//                 if (hasDir && hasR) {
//                     FAT32DriverRequest src = srcs[0];
//                     FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                     DirectoryEntry srcInfo = get_info(src);
//                     DirectoryEntry destInfo = get_info(dest);
//                     if (srcInfo.cluster_number == destInfo.cluster_number || check_contain(dest.parent_cluster_number, srcInfo.cluster_number)){
//                         print("\ncp: Cannnot copy into itself\n");
//                     }
//                     else{
//                         copy1Folder(src, dest);
//                     }
//                 } else if (hasDir) { // aman
//                     print("\ncp: Source is a directory\n");
//                 } else { // aman
//                     FAT32DriverRequest src = srcs[0];
//                     uint32_t destCluster = path_to_cluster(shellparser.content[len - 1], currentCluster);
//                     FAT32DriverRequest dest = {
//                         .parent_cluster_number = destCluster
//                     };
//                     memcpy(dest.name, src.name, 8);
//                     memcpy(dest.ext, src.ext, 3);
//                     copy1File(src, dest);
//                 }
//             }
//             else {
//                 if (hasR && hasDir) {
//                     if(is_filename(shellparser.content[len - 1])) {
//                         print("\ncp: Invalid file name\n");
//                         return;
//                     }
//                     uint8_t status = copy_create_folders(shellparser.content[len - 1], currentCluster, 0);
//                     if(status != 0) return;

//                     FAT32DriverRequest src = srcs[0];
//                     FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                     DirectoryEntry srcInfo = get_info(src);
//                     DirectoryEntry destInfo = get_info(dest);
//                     if (srcInfo.cluster_number == destInfo.cluster_number || check_contain(dest.parent_cluster_number, srcInfo.cluster_number)){
//                         print("\ncp: Cannnot copy into itself\n");
//                     }
//                     else{
//                         copy1Folder(src, dest);
//                     }
//                 }
//                 else if (hasDir) { // aman
//                     print("\ncp: Source is a directory\n");
//                 }
//                 else { // aman
//                     FAT32DriverRequest src = srcs[0];
//                     FAT32DriverRequest dest = {0};
//                     if(!is_filename(shellparser.content[len - 1])) {
//                         uint8_t status = copy_create_folders(shellparser.content[len - 1], currentCluster, 0);
//                         if(status != 0) return;
//                         dest.parent_cluster_number = path_to_cluster(shellparser.content[len - 1], currentCluster);
//                         memcpy(dest.name, src.name, 8);
//                         memcpy(dest.ext, src.ext, 3);
//                     }
//                     else{
//                         uint8_t status = copy_create_folders(shellparser.content[len - 1], currentCluster, 1);
//                         if(status != 0) return;
//                         dest = path_to_file_request(shellparser.content[len - 1], currentCluster);
//                     }
//                     copy1File(src, dest);
//                 }
//             }
//         }
//     } else {
//         print("\ncp: Invalid command\n");
//     }
// }

// void mv(uint32_t currentCluster) {
//     // copy cp di atas, terus deletef setiap request untuk yg berhasil
//     int len = shellparser.word_count;
//     if (len >= 3) { // mv minimal len cmd 3
//         uint8_t isdir = 0;
//         uint8_t isfile = 0;
//         uint8_t hasR = 1; // karna ga perlu flag r
//         uint8_t hasDir = 0;

//         FAT32DriverRequest srcs[len - 2]; // create arr of sources
//         uint8_t isFile[len - 2]; 

//         int nSrc = 0;
//         for (int i = 1; i < len - 1; i++) {
//             isdir = is_directorypath_valid(shellparser.content[i], currentCluster);
//             isfile = is_filepath_valid(shellparser.content[i], currentCluster);
//             if (isfile) {
//                 srcs[nSrc] = path_to_file_request(shellparser.content[i], currentCluster);
//                 isFile[nSrc] = 1;
//                 nSrc++;
//             } else if (isdir) {
//                 srcs[nSrc] = path_to_dir_request(shellparser.content[i], currentCluster);
//                 isFile[nSrc] = 0;
//                 hasDir = 1;
//                 nSrc++;
//             } else { // aman
//                 print("\nmv: Invalid path\n");
//                 return;
//             }
//         }
        
//         // semua source valid

//         // cek apakah dest ada
//         isdir = is_directorypath_valid(shellparser.content[len - 1], currentCluster);
//         isfile = is_filepath_valid(shellparser.content[len - 1], currentCluster);

//         if (nSrc > 1) {
//             if (isfile) { // aman
//                 print("\nmv: Destination is not a directory\n");
//                 return;
//             } else if (isdir) {
//                 // FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                 for (int i = 0; i < nSrc; i++) {
//                     FAT32DriverRequest src = srcs[i];
//                     if (isFile[i]) { // aman
//                         uint32_t destCluster = path_to_cluster(shellparser.content[len - 1], currentCluster);
//                         FAT32DriverRequest dest = {
//                             .parent_cluster_number = destCluster
//                         };
//                         memcpy(dest.name, src.name, 8);
//                         memcpy(dest.ext, src.ext, 3);
//                         copy1File(src, dest);
//                     } else {
//                         FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                         DirectoryEntry srcInfo = get_info(src);
//                         DirectoryEntry destInfo = get_info(dest);
//                         if (srcInfo.cluster_number == destInfo.cluster_number || check_contain(dest.parent_cluster_number, srcInfo.cluster_number)){
//                             print("\ncp: Cannnot copy into itself\n");
//                         }
//                         else{
//                             copy1Folder(src, dest);
//                         }
//                     }
//                 }
//             } else { // aman
//                 print("\nmv: Destination is not a directory\n");
//                 return;
//             }
//         } else {
//             if (isfile) { // aman
//                 print("\nmv: Cannot overwrite existing file\n");
//                 return;
//             } else if (isdir) {
//                 if (hasDir && hasR) {
//                     FAT32DriverRequest src = srcs[0];
//                     FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                     DirectoryEntry srcInfo = get_info(src);
//                     DirectoryEntry destInfo = get_info(dest);
//                     if (srcInfo.cluster_number == destInfo.cluster_number || check_contain(dest.parent_cluster_number, srcInfo.cluster_number)){
//                         print("\ncp: Cannnot copy into itself\n");
//                     }
//                     else{
//                         copy1Folder(src, dest);
//                     }
//                 } else if (hasDir) { // aman
//                     print("\nmv: Source is a directory\n");
//                     return;
//                 } else { // aman
//                     //print("\nmasuksini");
//                     FAT32DriverRequest src = srcs[0];
//                     uint32_t destCluster = path_to_cluster(shellparser.content[len - 1], currentCluster);
//                     FAT32DriverRequest dest = {
//                         .parent_cluster_number = destCluster
//                     };
//                     memcpy(dest.name, src.name, 8);
//                     memcpy(dest.ext, src.ext, 3);
//                     copy1File(src, dest);
//                 }
//             } else {
//                 if (hasR && hasDir) {
//                     if(is_filename(shellparser.content[len - 1])) {
//                         print("\nmv: Invalid file name\n");
//                         return;
//                     }
//                     uint8_t status = copy_create_folders(shellparser.content[len - 1], currentCluster, 0);
//                     if(status != 0) return;

//                     FAT32DriverRequest src = srcs[0];
//                     FAT32DriverRequest dest = path_to_dir_request(shellparser.content[len - 1], currentCluster);
//                     DirectoryEntry srcInfo = get_info(src);
//                     DirectoryEntry destInfo = get_info(dest);
//                     if (srcInfo.cluster_number == destInfo.cluster_number || check_contain(dest.parent_cluster_number, srcInfo.cluster_number)){
//                         print("\ncp: Cannnot copy into itself\n");
//                     }
//                     else{
//                         copy1Folder(src, dest);
//                     }
//                 } else if (hasDir) { // aman
//                     print("\nmv: Source is a directory\n");
//                     return;
//                 } else { // aman
//                     if(is_filename(shellparser.content[len - 1])) {
//                         print("\nmv: Invalid file name\n");
//                         return;
//                     }
//                     uint8_t status = copy_create_folders(shellparser.content[len - 1], currentCluster, 0);
//                     if(status != 0) return;
//                     // write file baru
//                     FAT32DriverRequest src = srcs[0];
//                     // ini bisa tapi gabisa loncat folder, ex: [ada]/gaada.txt, gabisa kek [ada]/[gaada]/gaada.txt
//                     FAT32DriverRequest dest = path_to_file_request(shellparser.content[len - 1], currentCluster);
//                     copy1File(src, dest);
//                 }
//             }
//         }

//         for (int i = 0; i < nSrc; i++) {
//             deletef(srcs[i]);
//         }

//     } else {
//         print("\ncp: Invalid command\n");
//         return;
//     }
// }

// void copy1Folder(FAT32DriverRequest src, FAT32DriverRequest dest) {
//     dest.buffer_size = 0;

//     DirectoryEntry destination = get_info(dest);
//     FAT32DriverRequest selfReq = src;
//     selfReq.parent_cluster_number = destination.cluster_number;
//     uint8_t code = writef(selfReq);
//     if (code == 1){
//         print("\ncp: Folder already exist\n");
//         return;
//     }
//     if (code == 2){
//         print("\ncp: Destination is invalid\n");
//         return;
//     }
//     //dir(dest.parent_cluster_number);

//     DirectoryEntry self = get_info(selfReq);
//     DirectoryEntry source = get_info(src);

//     FAT32DirectoryReader read = readf_dir(src);
//     for(uint32_t i = 0; i < read.cluster_count; i++){
//         for(uint32_t j = 0; j < ENTRY_COUNT; j++){
//             // read.content[i].entry[j];
//             if(memcmp(&read.content[i].entry[j], &emptyEntry, 32) == 0){
//                 continue;
//             }

//             if(read.content[i].entry[j].directory){
//                 FAT32DriverRequest newDest = {0};

//                 memcpy(newDest.name, self.filename, 8);
//                 newDest.parent_cluster_number = destination.cluster_number;

//                 FAT32DriverRequest newSrc = {0};
//                 memcpy(newSrc.name, read.content[i].entry[j].filename, 8);
//                 newSrc.parent_cluster_number = source.cluster_number;
//                 newSrc.buffer_size = 0;
                
//                 copy1Folder(newSrc, newDest);
//             }
//             else{
//                 FAT32DriverRequest newDest = {
//                     .parent_cluster_number = self.cluster_number,
//                     .buffer_size = read.content[i].entry[j].size
//                 };
                
//                 memcpy(newDest.name, read.content[i].entry[j].filename, 8);
//                 memcpy(newDest.ext, read.content[i].entry[j].extension, 3);

//                 FAT32DriverRequest newSrc = {
//                     .parent_cluster_number = source.cluster_number,
//                     .buffer_size = read.content[i].entry[j].size
//                 };

//                 memcpy(newSrc.name, read.content[i].entry[j].filename, 8);
//                 memcpy(newSrc.ext, read.content[i].entry[j].extension, 3);
                
//                 copy1File(newSrc, newDest);
//             }

//         }
//     }
    
//     closef_dir(read);
// }

// void copy1File(FAT32DriverRequest src, FAT32DriverRequest dest) {
//     FAT32FileReader read = readf(src);
//     dest.buffer_size = read.size;
//     dest.buf = read.content;
//     closef(read);
    
//     uint8_t code = writef(dest);
//     if (code == 1){
//         print("\ncp: Filename already exist\n");
//         return;
//     }
//     if (code == 2){
//         print("\ncp: Destination is invalid\n");
//         return;
//     }    
// }

// void cat(uint32_t currentCluster) {
//     // prekondisi: path sudah valid, dan adalah path ke file
//     FAT32DriverRequest req = path_to_file_request(shellparser.content[1], currentCluster);
    
//     FAT32FileReader result = readf(req);
//     string_t string = str_new("\n");
//     for (uint32_t j = 0; j < result.size; j++) {
//         str_addc(&string, *(((char*)result.content) + j));
//     }
//     sout_t sout = sout_newstr(string);
//     sout_printall_ws(&sout);

//     str_delete(&string);
//     sout_clear(&sout);
//     closef(result);
// } 

// void execute_file(char* path, uint32_t currentCluster){
//     FAT32DriverRequest req = path_to_dir_request(path, currentCluster);
//     syscall(SYSCALL_TASK_START, (uint32_t) &req, 0, 0);
// }