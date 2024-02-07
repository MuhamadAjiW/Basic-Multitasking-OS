#include "lib-header/stdtype.h"
#include "lib-header/syscall.h"
#include "lib-header/stdio.h"
#include "lib-header/stdlib.h"
#include "lib-header/string.h"
#include "lib-header/stdmem.h"
#include "lib-header/parser.h"
#include "lib-header/window_manager.h"
#include "lib-header/font.h"
#include "lib-header/image.h"

#include "lib-header/shell.h"
#include "lib-header/commands.h"
#include "lib-header/commands-util.h"

// const uint8_t window_size = 2;

parser_t sh_parser = {0};
shell_app sh = {
    .default_font_color = 0xf,
    .default_input_color = 0x54,
    .default_background_color = 0xe,
    .default_cursor_color = 0xd,

    .cursor_x = 0,
    .cursor_y = 0,
    .cursor_x_limit = 0,
    .cursor_y_limit = 0,
    .cursor_on = 0,
    .cursor_show = 0,
    .cursor_background_buffer = 0,
    .cursor_counter = 0,
    .cursor_blocked_1 = 0,
    .cursor_blocked_2 = 0,

    .winfo = {
        .main_buffer = (uint8_t*) 1,
        .xloc = 0,
        .yloc = 0,
        .xlen = SCREEN_WIDTH,
        .ylen = SCREEN_HEIGHT
    },

    .reader = {
        .buffer_addr = (char*) 1,
        .buffer_size = INPUT_BUFFER_SIZE,
        .max_idx = 0,
        .current_idx = 0
    },

    .grid = {0},

    .background = {0}
};

// App
void app_initialize(){
    sh.dir.cluster_number = ROOT_CLUSTER_NUMBER;
    sh.dir.path = (char*) malloc (sizeof(char) * INPUT_BUFFER_SIZE);
    memcpy(sh.dir.path, "/root", 6);

    font_load(&(sh.finfo), "system/stdfont.fnt", sh.dir.cluster_number);
    window_init(&(sh.winfo));
    grid_initialize();
    app_load_background("system/stdbg.imp");
    app_draw_background(&(sh.winfo), sh.background);
    window_register(&(sh.winfo));
    reader_initialize();
    cursor_initialize();

    shell_newline();    
}
void app_load_background(char* path){
    image_load(&(sh.background), path, sh.dir.cluster_number);
    image_change_palette(sh.background);
}
void app_draw_background(window_info* winfo, image_info imginfo){
    uint16_t min_width;
    uint16_t min_height;

    if(winfo->xlen < imginfo.width) min_width = winfo->xlen;
    else min_width = imginfo.width;
    if(winfo->ylen < imginfo.height) min_height = winfo->ylen;
    else min_height = imginfo.height;

    for(int r = 0; r < min_height; r++){
        for(int c = 0; c < min_width; c++){
            window_draw_pixel(winfo, r, c, imginfo.map[r * imginfo.width + c]);
        }
    }
}
void app_change_background(char* path){
    image_delete(&(sh.background));
    app_load_background(path);
    app_draw_background(&(sh.winfo), sh.background);
    print("\nBackground change successful\n");
}

// Grid
void grid_initialize(){
    sh.grid.xlen = sh.winfo.xlen / sh.finfo.width;
    sh.grid.ylen = sh.winfo.ylen / sh.finfo.height;
    uint32_t size = sh.grid.xlen * sh.grid.ylen;
    sh.grid.char_map = (char*) malloc (sizeof(char) * size);
    sh.grid.char_color_map = (char*) malloc (sizeof(char) * size);
}
void grid_write(){
    sh.cursor_blocked_2 = 1;
    app_draw_background(&(sh.winfo), sh.background);
    for (uint16_t r = 0; r < sh.grid.ylen; r++){
        for (uint16_t c = 0; c < sh.grid.xlen; c++){
            uint32_t grid_offset = r * sh.grid.xlen + c;
            if(sh.grid.char_map[grid_offset] != 0){
                font_write(&(sh.winfo), sh.finfo, r, c, sh.grid.char_map[grid_offset], sh.grid.char_color_map[grid_offset]);
            }
        }
    }
    cursor_show();
    sh.cursor_blocked_2 = 0;
}

// Cursor
void cursor_initialize(){
    sh.cursor_background_buffer = (uint8_t*) malloc (sh.finfo.height);
    cursor_on();
}
void cursor_hide(){
    if(sh.cursor_on){
        sh.cursor_blocked_1 = 1;
        sh.cursor_show = 0;
        sh.cursor_counter = 0;

        for(uint8_t j = 0; j < sh.finfo.height - 1; j++){
            window_draw_pixel(&(sh.winfo), (sh.finfo.height * sh.cursor_y) + j, (sh.finfo.width * sh.cursor_x), sh.cursor_background_buffer[j]);
        }

        window_update(&(sh.winfo));
        sh.cursor_blocked_1 = 0;
    }
}
void cursor_show(){
    if(sh.cursor_on){
        sh.cursor_blocked_1 = 1;
        sh.cursor_show = 1;
        sh.cursor_counter = 0;
        
        for(uint8_t j = 0; j < sh.finfo.height - 1; j++){
            sh.cursor_background_buffer[j] = sh.winfo.main_buffer[(320 * ((sh.finfo.height* sh.cursor_y) + j)) + sh.finfo.width * sh.cursor_x];
            window_draw_pixel(&(sh.winfo), (sh.finfo.height * sh.cursor_y) + j, (sh.finfo.width * sh.cursor_x), sh.default_cursor_color);
        }

        window_update(&(sh.winfo));
        sh.cursor_blocked_1 = 0;
    }
}
uint16_t cursor_get_y(){
    return sh.cursor_y;
}
uint16_t cursor_get_x(){
    return sh.cursor_x;
}

void cursor_on(){
    sh.cursor_on = 1;
}

void cursor_off(){
    if(sh.cursor_on){
        if(sh.cursor_show){
            cursor_hide();
        }
        sh.cursor_on = 0;
    }
}

void cursor_limit(uint8_t x, uint8_t y){
    sh.cursor_x_limit = x;
    sh.cursor_y_limit = y;
}

void cursor_set(uint8_t x, uint8_t y){
    if(sh.cursor_on){
        sh.cursor_blocked_2 = 1;
        if(sh.cursor_show){
            cursor_hide();
        }

        sh.cursor_x = x;
        sh.cursor_y = y;

        sh.cursor_show = 1;
        cursor_show();
        sh.cursor_blocked_2 = 0;
    }
}

uint16_t cursor_find_edge(uint16_t y){
    uint16_t counter = 0;
    uint32_t baseline = y * sh.grid.xlen;
    while (sh.grid.char_map[baseline + counter] != 0 && sh.grid.char_map[baseline + counter] != '\n' && counter < sh.grid.xlen){
        counter++;
    }
    if(counter == sh.grid.xlen) counter--;
    
    return counter;
}

int32_t cursor_move(int8_t direction){
    int32_t dist = 0;
    uint32_t cursor_overall_position = sh.cursor_y * sh.grid.xlen + sh.cursor_x;
    if (sh.cursor_on){
        switch (direction){
            case 1:
                if((sh.cursor_x == sh.grid.xlen - 1 && sh.grid.char_map[cursor_overall_position] != 0)){
                    if (sh.cursor_y != sh.grid.ylen - 1){
                        dist = 1;
                        cursor_set(0, sh.cursor_y + 1);
                    }
                }
                else{
                    if(sh.grid.char_map[cursor_overall_position] == '\n'){
                        if(sh.cursor_y != sh.grid.ylen - 1){
                            dist = 1;
                            cursor_set(0, sh.cursor_y + 1);
                        }
                    }
                    else if(sh.grid.char_map[cursor_overall_position] != 0){
                        dist = 1;
                        cursor_set(sh.cursor_x + 1, sh.cursor_y);
                    }
                }
                break;
            case -1:
                if(!(sh.cursor_y == sh.cursor_y_limit && sh.cursor_x == sh.cursor_x_limit)){
                    if(sh.cursor_x != 0 ){
                        dist = -1;
                        cursor_set(sh.cursor_x - 1, sh.cursor_y);
                    }
                    else{
                        if(sh.cursor_y != 0){
                            dist = -1;
                            uint8_t edge = cursor_find_edge(sh.cursor_y - 1);
                            if(sh.grid.char_map[(sh.cursor_y - 1) * sh.grid.xlen + edge - 1] == '\n') cursor_set(edge - 1, sh.cursor_y - 1);
                            else cursor_set(edge, sh.cursor_y - 1);
                        }
                    }
                }
                break;
            
            default:
                break;
        }
    }

    return dist;
}

// TODO: Use threading instead
void cursor_blinking(){
    if(sh.cursor_on && !sh.cursor_blocked_1 && !sh.cursor_blocked_2){
        if(sh.cursor_counter > 30000){
            if(sh.cursor_show){
                cursor_hide();
            }
            else{
                cursor_show();
            }
            sh.cursor_counter = 0;
        }
        else{
            sh.cursor_counter++;
        }
    }
}


// Reader
void reader_initialize(){
    sh.reader.buffer_size = INPUT_BUFFER_SIZE;
    sh.reader.buffer_addr = (char*) malloc (sizeof(char) * INPUT_BUFFER_SIZE);
    sh.reader.buffer_addr[0] = 0;
    sh.reader.current_idx = 0;
    sh.reader.max_idx = 0;
}

void reader_clear(){
    sh.reader.buffer_size = INPUT_BUFFER_SIZE;
    free(sh.reader.buffer_addr);
    sh.reader.buffer_addr = (char*) malloc (sizeof(char) * INPUT_BUFFER_SIZE);
    sh.reader.buffer_addr[0] = 0;
    sh.reader.current_idx = 0;
    sh.reader.max_idx = 0;
}

void reader_insert(char c){
    if(sh.reader.max_idx < sh.reader.buffer_size - 1){
        for(uint32_t i = sh.reader.max_idx; i > sh.reader.current_idx; i--){
            sh.reader.buffer_addr[i] = sh.reader.buffer_addr[i - 1];
        }

        sh.reader.buffer_addr[sh.reader.current_idx] = c;
        sh.reader.max_idx++;
        sh.reader.current_idx++;
    }
    else{
        sh.reader.buffer_size += INPUT_BUFFER_SIZE;
        sh.reader.buffer_addr = (char*) realloc (sh.reader.buffer_addr, sizeof(char) * sh.reader.buffer_size);
        reader_insert(c);
    }
}

void reader_move(int8_t direction){
    sh.reader.current_idx += direction;
}


// Shell functionalities
void shell_clear(){
    uint32_t limit = sh.grid.xlen * sh.grid.ylen;
    for(uint32_t i = 0; i < limit; i++){
        sh.grid.char_map[i] = 0;
        sh.grid.char_color_map[i] = 0;
    }
    app_draw_background(&(sh.winfo), sh.background);

    if(sh.cursor_on){
        cursor_set(0, 0);
    }

    window_update(&(sh.winfo));
}

void shell_backspace(){
    uint8_t success = 0;

    if(!(sh.cursor_y == sh.cursor_y_limit && sh.cursor_x == sh.cursor_x_limit)){
        uint16_t end = sh.grid.xlen * sh.grid.ylen;
        uint16_t loc = sh.cursor_y * sh.grid.xlen + sh.cursor_x;
        for(int i = loc - 1; i < end; i++){
            sh.grid.char_map[i] = sh.grid.char_map[i + 1];
            sh.grid.char_color_map[i] = sh.grid.char_color_map[i + 1];
        }
        sh.grid.char_map[end - 1] = 0;
        sh.grid.char_color_map[end - 1] = 0;
        success = 1;
    }
    else{
        success = 0;
    }

    if(success){
        cursor_move(-1);
        grid_write();

        for(uint32_t i = sh.reader.current_idx-1; i < sh.reader.max_idx-1; i++){
            sh.reader.buffer_addr[i] = sh.reader.buffer_addr[i+1];
        }
        if(sh.reader.current_idx > 0){
            sh.reader.current_idx--;
        }
        if(sh.reader.max_idx > 0){
            sh.reader.max_idx--;
        }
    }
}

void shell_newline(){
    print("\n");
    print(sh.dir.path);
    print("> ");
    cursor_limit(sh.cursor_x, sh.cursor_y);
}

void shell_evaluate(){
    sh.reader.buffer_addr[sh.reader.max_idx] = 0;

    parser_parse(&sh_parser, sh.reader.buffer_addr, ' ');

    if (sh_parser.word_count > 0){
        if(strcmp(sh_parser.content[0], "clear") == 0){
            if(sh_parser.word_count == 1) shell_clear();
            else print("\nclear: Invalid argument");
        }
        else if(strcmp(sh_parser.content[0], "dir") == 0){
            dir(sh.dir.cluster_number);
        }
        else if(strcmp(sh_parser.content[0], "cd") == 0){
            // current_dir = cd(sh_parser.content[1], current_dir);
            if (sh_parser.word_count == 1){
                sh.dir.cluster_number = ROOT_CLUSTER_NUMBER;
                memcpy(sh.dir.path, "/root", 6);
            }
            else if (is_directorypath_valid(sh_parser.content[1], sh.dir.cluster_number)){
                cd(sh_parser.content[1], &sh.dir);
            } else {
                print("\ncd: no such directory: ");
                print(sh_parser.content[1]);
                print("\n");
            }
        }
        else if(strcmp(sh_parser.content[0], "mkdir") == 0){
            if(sh_parser.word_count != 2){
                print("\nmkdir: Invalid command\n");
            }
            else{
                mkdir(sh_parser.content[1], sh.dir.cluster_number);
            }
        }
        else if (strcmp(sh_parser.content[0], "whereis") == 0) {
            whereis(sh.dir.cluster_number, sh_parser.content[1], sh.dir.path);
        }
        else if(strcmp(sh_parser.content[0], "ls") == 0){
            if (sh_parser.word_count == 1){
                ls(sh.dir.cluster_number);
            }
            else if (is_directorypath_valid(sh_parser.content[1], sh.dir.cluster_number)){
                ls(path_to_cluster(sh_parser.content[1], sh.dir.cluster_number));
            } else {
                print("\nls: ");
                print(sh_parser.content[1]);
                print(": No such file or directory\n");
            }  
        }
        else if (strcmp(sh_parser.content[0], "rm") == 0) {
            rm(sh.dir.cluster_number);
        }
        else if (strcmp(sh_parser.content[0], "cp") == 0) {
            cp(sh.dir.cluster_number);
        }
        else if (strcmp(sh_parser.content[0], "mv") == 0) {
            mv(sh.dir.cluster_number);
        }
        else if(strcmp(sh_parser.content[0], "cat") == 0){
            if (sh_parser.word_count == 1){
                // masuk state mini program
            } else if (is_filepath_valid(sh_parser.content[1], sh.dir.cluster_number)){
                cat(sh.dir.cluster_number);
            } else if (is_directorypath_valid(sh_parser.content[1], sh.dir.cluster_number)){
                print("\ncat: ");
                print(sh_parser.content[1]);
                print(": Is a directory\n");
            } else {
                print("\ncat: ");
                print(sh_parser.content[1]);
                print(": No such file\n");
            }
        }
        else if(strcmp(sh_parser.content[0], "changebg") == 0){
            if(sh_parser.word_count == 1){
                print("\nchangebg: no image provided\n");
            }
            else{
                if(is_filepath_valid(sh_parser.content[1], sh.dir.cluster_number)){
                    int len = strlen(sh_parser.content[1]);
                    uint8_t valid = 1;
                    if(len < 3) valid = 0;
                    else{
                        for (int i = 0; i < 3; i++) {
                            if (sh_parser.content[1][len - 1 - i] == '.') {
                                valid = 0;
                                break;
                            }
                        }
                        if(valid){
                            valid = sh_parser.content[1][len-1] == 'p' &&
                                    sh_parser.content[1][len-2] == 'm' && 
                                    sh_parser.content[1][len-3] == 'i';
                        }
                    }
                    
                    if(!valid) print("\nchangebg: invalid file\n");
                    else app_change_background(sh_parser.content[1]);
                }
                else{
                    print("\nchangebg: no such file: ");
                    print(sh_parser.content[1]);
                    print("\n");
                }
            }
        }

        // TODO: Review
        else if(strcmp(sh_parser.content[0], "exec") == 0){
            int execution_num = 1;
            switch (sh_parser.word_count){
            case 3:
                if(!int_parse_string_valid(sh_parser.content[2])){
                    print("\nexec: ");
                    print(sh_parser.content[3]);
                    print(": Invalid execution amount\n");
                    break;
                }
                
                execution_num = int_parse_string(sh_parser.content[2]);
                if(execution_num < 1){
                    print("\nexec: ");
                    print(sh_parser.content[3]);
                    print(": Invalid execution amount\n");
                    break;
                }
                /* fall through */

            case 2:
                if(is_filepath_valid(sh_parser.content[1], sh.dir.cluster_number)){
                    FAT32DriverRequest req = path_to_file_request(sh_parser.content[1], sh.dir.cluster_number);
                    if( req.ext[0] == 'p' &&
                        req.ext[1] == 'r' &&
                        req.ext[2] == 'g'
                    ){
                        for (int i = 0; i < execution_num; i++){
                            exec(&req);
                        }
                    }
                    else{
                        print("\nexec: Invalid file type");
                    }
                }
                else{
                    print("\nexec: ");
                    print(sh_parser.content[1]);
                    print(": No such file\n");
                }
                break;
            
            default:
                print("\nexec: Invalid argument");
                break;
            }
        }
        else if(strcmp(sh_parser.content[0], "ps") == 0){
            if(sh_parser.word_count == 1) ps();
            else print("\nps: Invalid argument");
        }
        else if(strcmp(sh_parser.content[0], "kill") == 0){
            if(sh_parser.word_count == 2){
                if(int_parse_string_valid(sh_parser.content[1])){
                    uint32_t pid = int_parse_string(sh_parser.content[1]);
                    if(pid == 0){
                        print("\nkill: Terminating the kernel is not allowed");
                    } else{
                        kill(pid);
                    }
                } else{
                    print("\nkill: Invalid argument");
                }
            } else{
                print("\nkill: Invalid argument");
            }
        }
        else{
            print("\nNo Command found: ");
            print(sh.reader.buffer_addr);
            print("\n");
        }
    }

    parser_clear(&sh_parser);
}

int main(void) {
    syscall(SYSCALL_NULL, 0, 0, 0);

    app_initialize();

    char buf[2] = {0};
    while (TRUE){
        buf[0] = sys_keyboard_get_char();

        switch (buf[0]){
            case TAB_CHAR:
                reader_insert(' ');
                reader_insert(' ');
                reader_insert(' ');
                reader_insert(' ');
                print("    ");
                break;

            case LARROW_CHAR:
                if(cursor_move(-1)){
                    reader_move(-1);
                }
                break;

            case RARROW_CHAR:
                if(cursor_move(1)){
                    reader_move(1);
                }
                break;
            
            case BACKSPACE_CHAR:
                shell_backspace();
                break;

            case '\n':
                shell_evaluate();
                reader_clear();
                shell_newline();
                break;
            
            default:
                if(buf[0] >= 32 && buf[0] <= 126){
                    reader_insert(buf[0]);
                    print_char_color(buf[0], 14);
                }
                break;
        }
        cursor_blinking();
    }

    return 0;
}


