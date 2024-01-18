#include "../lib-header/syscall.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/interrupt.h"

#include "../lib-header/pit.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/memory_manager.h"

extern InterruptHandler syscall_handlers[];

void register_syscall_response(uint8_t no, InterruptHandler response){
    syscall_handlers[no] = response;
}

/*syscall functions*/
// Basic functionality
void sys_idle(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    return;
}
void sys_get_timer_tick(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    *(uint32_t*)cpu.ebx = get_tick();
}

// Memory syscalls
void sys_malloc(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    *(void**) cpu.edx = (void*) kmalloc(cpu.ebx);
}
void sys_realloc(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    *(void**) cpu.edx = (void*) krealloc((void*) cpu.ebx, cpu.ecx);
}
void sys_free(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    kfree((void*)cpu.ebx);
}


// Windows manager syscall
void sys_windmgr_register(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    winmgr_register_winfo((window_info*) cpu.ebx);
}
void sys_winmgr_update(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    winmgr_update_window((window_info*) cpu.ebx);
}
void sys_winmgr_close(
    __attribute__((unused)) CPUSegments seg,
    __attribute__((unused)) CPURegister cpu,
    __attribute__((unused)) uint32_t int_number,
    __attribute__((unused)) InterruptStack info
){
    winmgr_close_window((uint16_t) cpu.ebx);
}



void enable_system_calls(){
    register_syscall_response(SYSCALL_NULL, sys_idle);
    register_syscall_response(SYSCALL_GET_TICK, sys_get_timer_tick);

    register_syscall_response(SYSCALL_MALLOC, sys_malloc);
    register_syscall_response(SYSCALL_REALLOC, sys_realloc);
    register_syscall_response(SYSCALL_FREE, sys_free);

    register_syscall_response(SYSCALL_WINMGR_REG, sys_windmgr_register);
    register_syscall_response(SYSCALL_WINMGR_UPDATE, sys_winmgr_update);
    register_syscall_response(SYSCALL_WINMGR_CLOSE, sys_winmgr_close);
}
