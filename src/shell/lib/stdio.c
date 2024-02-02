#include "../lib-header/syscall.h"
#include "../lib-header/shell.h"
#include "../lib-header/stdio.h"

extern shell_app sh;

// Screen I/O
char sys_keyboard_get_char(){
    char c;
    syscall(SYSCALL_GET_KEYBOARD_LAST_KEY, (uint32_t) &c, 0, 0);
    return c;
}

void sys_cursor_set_active(uint8_t active){
    syscall(SYSCALL_SET_CURSOR_ACTIVE, (uint32_t) active, 0, 0);
}

void sys_cursor_set_location(uint8_t r, uint8_t c){
    syscall(SYSCALL_SET_CURSOR_LOCATION, (uint32_t) r, (uint32_t) c, 0);
}

void scroll(){
    if(sh.cursor_y_limit > 0){
        sh.cursor_y_limit = sh.cursor_y_limit - 1;
    }
    else{
        sh.cursor_x_limit = 0;
    }

    for(uint16_t i = 0; i < sh.grid.ylen - 1; i++){
        for(uint16_t j = 0; j < sh.grid.xlen; j++){
            sh.grid.char_map[i * sh.grid.xlen + j] = sh.grid.char_map[(i + 1) * sh.grid.xlen + j];
            sh.grid.char_color_map[i * sh.grid.xlen + j] = sh.grid.char_color_map[(i + 1) * sh.grid.xlen + j];
        }
    }
    for(uint16_t i = 0; i < sh.grid.xlen; i++){
        sh.grid.char_map[(sh.grid.ylen - 1) * sh.grid.xlen + i] = 0;
        sh.grid.char_color_map[(sh.grid.ylen - 1) * sh.grid.xlen + i] = 0;
    }
}

void print(char* str){
    print_color(str, sh.default_font_color, sh.default_background_color);
}

void print_color(char* str, uint8_t fg_color, uint8_t bg_color){
    //kurang sangkil, masih bisa diimprove
    int i = 0;
    while (str[i] != 0){
        if(str[i] != '\n'){
            uint16_t end = sh.grid.xlen * sh.grid.ylen;
            uint16_t loc = sh.cursor_y * sh.grid.xlen + sh.cursor_x;
            for(int i = end-1; i >= loc; i--){
                sh.grid.char_map[0 + i] = sh.grid.char_map[0 + i - 1];
                sh.grid.char_color_map[0 + i] = sh.grid.char_color_map[0 + i - 1];
            }
            sh.grid.char_map[loc] = str[i];
            sh.grid.char_color_map[loc] = (bg_color << 4) | (fg_color & 0xf);
                
            if(sh.cursor_x == sh.grid.xlen - 1 && sh.cursor_y == sh.grid.ylen - 1){
                scroll();
                sh.cursor_x = 0;
                sh.cursor_y = sh.grid.ylen - 1;
                loc = sh.cursor_y * sh.grid.xlen + sh.cursor_x;
            }
            
            if((sh.cursor_x == sh.grid.xlen - 1 && sh.grid.char_map[loc] != 0)){
                if(sh.grid.char_map[loc] == '\n'){
                    sh.cursor_x = 0;
                    sh.cursor_y = sh.cursor_y + 2;
                }
                else if (sh.cursor_y != 24){
                    sh.cursor_x = 0;
                    sh.cursor_y = sh.cursor_y + 1;
                }
            }
            else{
                if(sh.grid.char_map[loc] == '\n'){
                    sh.cursor_x = 0;
                    sh.cursor_y = sh.cursor_y + 1;
                }
                else if(sh.grid.char_map[loc] != 0){
                    sh.cursor_x = sh.cursor_x + 1;
                }
            }
        }
        else{
            if(sh.cursor_y == sh.grid.ylen - 1){
                scroll();
                sh.cursor_x = 0;
                sh.cursor_y = sh.grid.ylen - 1;
            }
            else{
                sh.cursor_x = 0;
                sh.cursor_y = sh.cursor_y + 1;
            }
        }
        i++;
    }

    cursor_set(sh.cursor_x, sh.cursor_y);
    grid_write();
}

void print_char(char c){
    print_char_color(c, sh.default_font_color, sh.default_background_color);
}


void print_char_color(char c, uint8_t fg_color, uint8_t bg_color){
    if(c != '\n'){
        uint32_t end = sh.grid.xlen * sh.grid.ylen;
        uint32_t loc = sh.cursor_y * sh.grid.xlen + sh.cursor_x;
            
        for(uint32_t i = end - 1; i >= loc; i--){
            sh.grid.char_map[0 + i] = sh.grid.char_map[0 + i - 1];
            sh.grid.char_color_map[0 + i] = sh.grid.char_color_map[0 + i - 1];

            if(i == 0) break;
        }

        sh.grid.char_map[loc] = c;
        sh.grid.char_color_map[loc] = (bg_color << 4) | (fg_color & 0xf);

        if(sh.cursor_x == sh.grid.xlen - 1 && sh.cursor_x == sh.grid.ylen - 1){
            scroll();
            cursor_set(0, sh.grid.ylen - 1);
        }

        cursor_move(1);
        grid_write();
    }
    else{
        if(sh.cursor_y == sh.grid.ylen - 1){
            scroll();
            cursor_set(0, sh.grid.ylen - 1);
        }
        else{
            cursor_set(0, sh.cursor_y + 1);
        }
    }
}

// File I/O
FAT32FileReader readf(FAT32DriverRequest request){
    FAT32FileReader retval;
    syscall(SYSCALL_READ_FILE, (uint32_t) &request, (uint32_t) &retval, 0);
    return retval;
}

FAT32DirectoryReader readf_dir(FAT32DriverRequest request){
    FAT32DirectoryReader retval;
    syscall(SYSCALL_READ_DIR, (uint32_t) &request, (uint32_t) &retval, 0);
    return retval;
}

void closef(FAT32FileReader request){
    syscall(SYSCALL_CLOSE_FILE, (uint32_t) &request, 0, 0);
}

void closef_dir(FAT32DirectoryReader request){
    syscall(SYSCALL_CLOSE_DIR, (uint32_t) &request, 0, 0);
}

uint8_t writef(FAT32DriverRequest request){
    uint8_t status;
    syscall(SYSCALL_WRITE_FILE, (uint32_t) &request, (uint32_t) &status, 0);
    return status;
}

uint8_t deletef(FAT32DriverRequest request){
    uint8_t status;
    syscall(SYSCALL_DELETE_FILE, (uint32_t) &request, (uint32_t) &status, 0);
    return status;
}
