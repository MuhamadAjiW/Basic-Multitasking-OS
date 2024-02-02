
#include "../lib-header/stdtype.h"
#include "../lib-header/time.h"
#include "../lib-header/syscall.h"

time get_time(){
    time retval;
    syscall(SYSCALL_GET_TIME, (uint32_t) &retval, 0, 0);
    return retval;
}

