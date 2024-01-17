#include "../lib-header/syscall.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/interrupt.h"

// TODO: Delete
#include "../lib-header/framebuffer.h"

extern InterruptHandler syscall_handlers[];

void register_syscall_response(uint8_t no, InterruptHandler response){
    syscall_handlers[no] = response;
}

/*syscall functions*/
// use as template
void idle(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    framebuffer_write(0, 1, 'X', 0, 0xf);
}

void enable_system_calls(){
    register_syscall_response(SYSCALL_NULL, idle);
}

