#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib-header/syscall.h"
#include "../lib-header/interrupt.h"
#include "../lib-header/keyboard.h"
#include "../lib-header/framebuffer.h"
#include "../lib-header/pit.h"
#include "../lib-header/cmos.h"
#include "../lib-header/window_manager.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/process.h"

extern InterruptHandler syscall_handlers[];

void register_syscall_response(uint8_t no, InterruptHandler response){
    syscall_handlers[no] = response;
}

/*syscall functions*/
// Basic functionality
void sys_idle(__attribute__((unused)) struct InterruptFrame* iframe){
    return;
}
void sys_get_timer_tick(struct InterruptFrame* iframe){
    *(uint32_t*)iframe->cpu.general.ebx = get_tick();
}
void sys_get_time(struct InterruptFrame* iframe){
    cmos_read_rtc();
    *(struct cmos_reader*)iframe->cpu.general.ebx = cmos_get_data();
}
void sys_get_keyboard_last_key(struct InterruptFrame* iframe){
    keyboard_flush_buffer((char*) iframe->cpu.general.ebx);
}
void sys_set_cursor_active(struct InterruptFrame* iframe){
    if(iframe->cpu.general.ebx == 0) framebuffer_disable_cursor();
    else framebuffer_enable_cursor();
}
void sys_set_cursor_location(struct InterruptFrame* iframe){
    framebuffer_set_cursor((uint8_t) iframe->cpu.general.ebx, (uint8_t) iframe->cpu.general.ecx);
}

// Memory syscalls
void sys_malloc(struct InterruptFrame* iframe){
    *(void**) iframe->cpu.general.edx = (void*) kmalloc(iframe->cpu.general.ebx);
}
void sys_realloc(struct InterruptFrame* iframe){
    *(void**) iframe->cpu.general.edx = (void*) krealloc((void*) iframe->cpu.general.ebx, iframe->cpu.general.ecx);
}
void sys_free(struct InterruptFrame* iframe){
    kfree((void*)iframe->cpu.general.ebx);
}


// Windows manager syscall
void sys_windmgr_register(struct InterruptFrame* iframe){
    winmgr_register_winfo((struct window_info*) iframe->cpu.general.ebx);
}
void sys_winmgr_update(struct InterruptFrame* iframe){
    winmgr_update_window((struct window_info*) iframe->cpu.general.ebx);
}
void sys_winmgr_close(struct InterruptFrame* iframe){
    winmgr_close_window((uint16_t) iframe->cpu.general.ebx);
}

// Tasking syscall
void sys_process_start(struct InterruptFrame* iframe){
    process_create_user_proc(*(struct FAT32DriverRequest*) iframe->cpu.general.ebx);
}
void sys_process_stop(struct InterruptFrame* iframe){
    process_terminate(iframe->cpu.general.ebx);
}
void sys_process_exit(__attribute__((unused))struct InterruptFrame* iframe){
    process_terminate_current();
}
void sys_process_info(struct InterruptFrame* iframe){
    process_generate_list((struct process_list*) iframe->cpu.general.ebx);
}

// Filesystem syscall
void sys_read(struct InterruptFrame* iframe){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) iframe->cpu.general.ebx;
    *((struct FAT32FileReader*) iframe->cpu.general.ecx) = read(request);
}
void sys_read_directory(struct InterruptFrame* iframe){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) iframe->cpu.general.ebx;
    *((struct FAT32DirectoryReader*) iframe->cpu.general.ecx) = read_directory(request);
}
void sys_self_directory_info(struct InterruptFrame* iframe){
    *((struct FAT32DirectoryReader*) iframe->cpu.general.ecx) = self_directory_info(iframe->cpu.general.ebx);
}
void sys_write(struct InterruptFrame* iframe){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) iframe->cpu.general.ebx;
    *((int8_t*) iframe->cpu.general.ecx) =  write(request);
}
void sys_delete(struct InterruptFrame* iframe){
    struct FAT32DriverRequest request = *(struct FAT32DriverRequest*) iframe->cpu.general.ebx;
    *((int8_t*) iframe->cpu.general.ecx) =  delete(request);
}
void sys_close_file(struct InterruptFrame* iframe){
    close_file(*(struct FAT32FileReader*) iframe->cpu.general.ebx);
}
void sys_close_directory(struct InterruptFrame* iframe){
    close_directory(*(struct FAT32DirectoryReader*) iframe->cpu.general.ebx);
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

    register_syscall_response(SYSCALL_PROCESS_START, sys_process_start);
    register_syscall_response(SYSCALL_PROCESS_STOP, sys_process_stop);
    register_syscall_response(SYSCALL_PROCESS_EXIT, sys_process_exit);
    register_syscall_response(SYSCALL_PROCESS_INFO, sys_process_info);

    register_syscall_response(SYSCALL_READ_FILE, sys_read);
    register_syscall_response(SYSCALL_READ_DIR, sys_read_directory);
    register_syscall_response(SYSCALL_SELF_DIR_INFO, sys_self_directory_info);
    register_syscall_response(SYSCALL_WRITE_FILE, sys_write);
    register_syscall_response(SYSCALL_DELETE_FILE, sys_delete);
    register_syscall_response(SYSCALL_CLOSE_FILE, sys_close_file);
    register_syscall_response(SYSCALL_CLOSE_DIR, sys_close_directory);
}
