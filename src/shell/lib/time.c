
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib-header/time.h"
#include "../lib-header/syscall.h"

struct time get_time(){
    struct time retval;
    syscall(SYSCALL_GET_TIME, (uint32_t) &retval, 0, 0);
    return retval;
}

