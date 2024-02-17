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
void sys_idle(__attribute__((unused)) struct TrapFrame cpu){
    return;
}
void sys_get_timer_tick(struct TrapFrame cpu){
    *(uint32_t*)cpu.registers.ebx = get_tick();
}
void sys_get_time(struct TrapFrame cpu){
    cmos_read_rtc();
    *(struct cmos_reader*)cpu.registers.ebx = cmos_get_data();
}
void sys_get_keyboard_last_key(struct TrapFrame cpu){
    keyboard_flush_buffer((char*) cpu.registers.ebx);
}
void sys_set_cursor_active(struct TrapFrame cpu){
    if(cpu.registers.ebx == 0) framebuffer_disable_cursor();
    else framebuffer_enable_cursor();
}
void sys_set_cursor_location(struct TrapFrame cpu){
    framebuffer_set_cursor((uint8_t) cpu.registers.ebx, (uint8_t) cpu.registers.ecx);
}

// Memory syscalls
void sys_malloc(struct TrapFrame cpu){
    *(void**) cpu.registers.edx = (void*) kmalloc(cpu.registers.ebx);
}
void sys_realloc(struct TrapFrame cpu){
    *(void**) cpu.registers.edx = (void*) krealloc((void*) cpu.registers.ebx, cpu.registers.ecx);
}
void sys_free(struct TrapFrame cpu){
    kfree((void*)cpu.registers.ebx);
}


// Windows manager syscall
void sys_windmgr_register(struct TrapFrame cpu){
    winmgr_register_winfo((struct window_info*) cpu.registers.ebx);
}
void sys_winmgr_update(struct TrapFrame cpu){
    winmgr_update_window((struct window_info*) cpu.registers.ebx);
}
void sys_winmgr_close(struct TrapFrame cpu){
    winmgr_close_window((uint16_t) cpu.registers.ebx);
}

// Tasking syscall
void sys_task_start(struct TrapFrame cpu){
    task_create(*(struct FAT32DriverRequest*) cpu.registers.ebx, STACKTYPE_USER, EFLAGS_BASE | EFLAGS_INTERRUPT | EFLAGS_PARITY);
}
void sys_task_stop(struct TrapFrame cpu){
    task_terminate(cpu.registers.ebx);
}
void sys_task_exit(__attribute__((unused))struct TrapFrame cpu){
    task_terminate_current();
}
void sys_task_info(struct TrapFrame cpu){
    task_generate_list((struct task_list*) cpu.registers.ebx);
}

// Filesystem syscall
void sys_read(struct TrapFrame cpu){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.registers.ebx;
    *((struct FAT32FileReader*) cpu.registers.ecx) = read(request);
}
void sys_read_directory(struct TrapFrame cpu){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.registers.ebx;
    *((struct FAT32DirectoryReader*) cpu.registers.ecx) = read_directory(request);
}
void sys_self_directory_info(struct TrapFrame cpu){
    *((struct FAT32DirectoryReader*) cpu.registers.ecx) = self_directory_info(cpu.registers.ebx);
}
void sys_write(struct TrapFrame cpu){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.registers.ebx;
    *((int8_t*) cpu.registers.ecx) =  write(request);
}
void sys_delete(struct TrapFrame cpu){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) cpu.registers.ebx;
    *((int8_t*) cpu.registers.ecx) =  delete(request);
}
void sys_close_file(struct TrapFrame cpu){
    close_file(*(struct FAT32FileReader*) cpu.registers.ebx);
}
void sys_close_directory(struct TrapFrame cpu){
    close_directory(*(struct FAT32DirectoryReader*) cpu.registers.ebx);
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

    register_syscall_response(SYSCALL_TASK_START, sys_task_start);
    register_syscall_response(SYSCALL_TASK_STOP, sys_task_stop);
    register_syscall_response(SYSCALL_TASK_EXIT, sys_task_exit);
    register_syscall_response(SYSCALL_TASK_INFO, sys_task_info);

    register_syscall_response(SYSCALL_READ_FILE, sys_read);
    register_syscall_response(SYSCALL_READ_DIR, sys_read_directory);
    register_syscall_response(SYSCALL_SELF_DIR_INFO, sys_self_directory_info);
    register_syscall_response(SYSCALL_WRITE_FILE, sys_write);
    register_syscall_response(SYSCALL_DELETE_FILE, sys_delete);
    register_syscall_response(SYSCALL_CLOSE_FILE, sys_close_file);
    register_syscall_response(SYSCALL_CLOSE_DIR, sys_close_directory);
}
