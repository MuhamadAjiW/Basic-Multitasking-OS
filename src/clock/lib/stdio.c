#include "../lib-header/syscall.h"
#include "../lib-header/stdio.h"

// File I/O
FAT32FileReader readf(FAT32DriverRequest request){
    FAT32FileReader retval;
    syscall(SYSCALL_READ_FILE, (uint32_t) &request, (uint32_t) &retval, 0);
    return retval;
}

FAT32DirectoryReader readf_dir(FAT32DriverRequest request){
    FAT32DirectoryReader retval;
    syscall(SYSCALL_READ_DIR, (uint32_t) &request, (uint32_t) &retval, 0);
    return retval;
}

void closef(FAT32FileReader request){
    syscall(SYSCALL_CLOSE_FILE, (uint32_t) &request, 0, 0);
}

void closef_dir(FAT32DirectoryReader request){
    syscall(SYSCALL_CLOSE_DIR, (uint32_t) &request, 0, 0);
}

uint8_t writef(FAT32DriverRequest request){
    uint8_t status;
    syscall(SYSCALL_WRITE_FILE, (uint32_t) &request, (uint32_t) &status, 0);
    return status;
}

uint8_t deletef(FAT32DriverRequest request){
    uint8_t status;
    syscall(SYSCALL_DELETE_FILE, (uint32_t) &request, (uint32_t) &status, 0);
    return status;
}
