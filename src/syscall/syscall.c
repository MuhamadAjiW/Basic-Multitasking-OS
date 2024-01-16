#include "../lib-header/syscall.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/interrupt.h"

extern InterruptHandler syscall_handlers[];

void register_syscall_response(uint8_t no, InterruptHandler response){
    syscall_handlers[no] = response;
}

/*syscall functions*/
// use as template
void idle(
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    return;
}

void enable_system_calls(){
    register_syscall_response(SYSCALL_NULL, idle);
}

