
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib-header/stdlib.h"
#include "../lib-header/syscall.h"
#include "../lib-header/cmos.h"

void* malloc(uint32_t size){
    void* location;
    syscall(SYSCALL_MALLOC, size, 0, (uint32_t) &location);
    return location;
}

void* realloc(void* ptr, uint32_t size){
    void* location;
    syscall(SYSCALL_REALLOC, (uint32_t) ptr, size, (uint32_t) &location);
    return location;
}

void free(void* memory){
    syscall(SYSCALL_FREE, (uint32_t) memory, 0, 0);
}

struct cmos_reader get_time(){
    struct cmos_reader cmos;
    syscall(SYSCALL_GET_TIME, (uint32_t) &cmos, 0, 0);
    return cmos;
}