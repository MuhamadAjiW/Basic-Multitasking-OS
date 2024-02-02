#include "../lib-header/syscall.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/interrupt.h"

#include "../lib-header/keyboard.h"
#include "../lib-header/framebuffer.h"
#include "../lib-header/pit.h"
#include "../lib-header/cmos.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/task.h"

extern InterruptHandler syscall_handlers[];

void register_syscall_response(uint8_t no, InterruptHandler response){
    syscall_handlers[no] = response;
}

/*syscall functions*/
// Basic functionality
void sys_idle(__attribute__((unused)) TrapFrame cpu){
    return;
}
void sys_get_timer_tick(TrapFrame cpu){
    *(uint32_t*)cpu.registers.ebx = get_tick();
}
void sys_get_time(TrapFrame cpu){
    cmos_read_rtc();
    *(cmos_reader*)cpu.registers.ebx = cmos_get_data();
}
void sys_get_keyboard_last_key(TrapFrame cpu){
    keyboard_flush_buffer((char*) cpu.registers.ebx);
}
void sys_set_cursor_active(TrapFrame cpu){
    if(cpu.registers.ebx == 0) framebuffer_disable_cursor();
    else framebuffer_enable_cursor();
}
void sys_set_cursor_location(TrapFrame cpu){
    framebuffer_set_cursor((uint8_t) cpu.registers.ebx, (uint8_t) cpu.registers.ecx);
}

// Memory syscalls
void sys_malloc(TrapFrame cpu){
    *(void**) cpu.registers.edx = (void*) kmalloc(cpu.registers.ebx);
}
void sys_realloc(TrapFrame cpu){
    *(void**) cpu.registers.edx = (void*) krealloc((void*) cpu.registers.ebx, cpu.registers.ecx);
}
void sys_free(TrapFrame cpu){
    kfree((void*)cpu.registers.ebx);
}


// Windows manager syscall
void sys_windmgr_register(TrapFrame cpu){
    winmgr_register_winfo((window_info*) cpu.registers.ebx);
}
void sys_winmgr_update(TrapFrame cpu){
    winmgr_update_window((window_info*) cpu.registers.ebx);
}
void sys_winmgr_close(TrapFrame cpu){
    winmgr_close_window((uint16_t) cpu.registers.ebx);
}

// Tasking syscall
void sys_task_exit(__attribute__((unused)) TrapFrame cpu){
    task_terminate_current();
}

void enable_system_calls(){
    register_syscall_response(SYSCALL_NULL, sys_idle);
    register_syscall_response(SYSCALL_GET_TICK, sys_get_timer_tick);
    register_syscall_response(SYSCALL_GET_TIME, sys_get_time);
    register_syscall_response(SYSCALL_GET_KEYBOARD_LAST_KEY, sys_get_keyboard_last_key);
    register_syscall_response(SYSCALL_SET_CURSOR_ACTIVE, sys_set_cursor_location);
    register_syscall_response(SYSCALL_SET_CURSOR_LOCATION, sys_set_cursor_location);

    register_syscall_response(SYSCALL_MALLOC, sys_malloc);
    register_syscall_response(SYSCALL_REALLOC, sys_realloc);
    register_syscall_response(SYSCALL_FREE, sys_free);

    register_syscall_response(SYSCALL_WINMGR_REG, sys_windmgr_register);
    register_syscall_response(SYSCALL_WINMGR_UPDATE, sys_winmgr_update);
    register_syscall_response(SYSCALL_WINMGR_CLOSE, sys_winmgr_close);

    register_syscall_response(SYSCALL_TASK_EXIT, sys_task_exit);
}
