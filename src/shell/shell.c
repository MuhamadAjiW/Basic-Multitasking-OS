#include "lib-header/stdtype.h"
#include "lib-header/syscall.h"
#include "lib-header/fat32.h"

int main(void) {
    while (TRUE){
        syscall(SYSCALL_NULL, 0, 0, 0);
    }

    return 0;
}
