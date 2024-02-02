#include "../lib-header/syscall.h"

char getc(){
    char c;
    syscall(SYSCALL_GET_KEYBOARD_LAST_KEY, (uint32_t) &c, 0, 0);
    return c;
}