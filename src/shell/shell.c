#include "lib-header/stdtype.h"
#include "lib-header/syscall.h"
#include "lib-header/stdio.h"
#include "lib-header/iostream.h"
#include "lib-header/stdlib.h"
#include "lib-header/string.h"
#include "lib-header/stdmem.h"
#include "lib-header/parser.h"
#include "lib-header/window_manager.h"
#include "lib-header/font.h"
#include "lib-header/image.h"
#include "lib-header/animation.h"

#include "lib-header/shell.h"
#include "lib-header/commands.h"
#include "lib-header/commands-util.h"

// const uint8_t window_size = 2;

parser_t sh_parser = {0};
shell_app sh = {
    .default_font_color = 0x1,
    .default_input_color = 0x2,
    .default_background_color = 0x0,
    .default_cursor_color = 0x4,

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
        .main_buffer = (uint32_t*) 1,
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

    font_load(&(sh.finfo), "system/stdfont.fnt", ROOT_CLUSTER_NUMBER);
    sh.winfo.ylen -= sh.finfo.height;
    window_init(&(sh.winfo));
    window_register(&(sh.winfo));
    grid_initialize();
    cursor_initialize();

    app_load_background("system/bg/stdbg1.imp");
    app_draw_background(&(sh.winfo), sh.background);
    window_update(&(sh.winfo));

    reader_initialize();

    // app_play_animation("system/stdanim.anm");

    shell_newline();
}
void app_load_background(char* path){
    image_load(&(sh.background), path, sh.dir.cluster_number);
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
void app_play_animation(char* path){
    cursor_off();
    anim_info anim;
    anim_load(&anim, path, sh.dir.cluster_number);

    FAT32DriverRequest req = path_to_file_request("system/colors.cnf", ROOT_CLUSTER_NUMBER);
    FAT32FileReader config = readf(&req);

    uint32_t roamer = 0;
    uint32_t multiplier = 1;
    uint32_t offset = 0;

    while (*(((char*)config.content) + roamer) != 0){
        roamer++;
    }
    for(uint32_t j = 1; j <= roamer; j++){
        offset += ((*(((uint8_t*)config.content) + roamer - j) - 48) * multiplier);
        multiplier *= 10;
    }
    
    if(offset + anim.palette_len > 250){
        print("\nplayanim: Too many colors\n");
        anim_delete(&anim);
        cursor_on();
    }
    else{
        syscall(SYSCALL_GRAPHICS_PALETTE_UPDATE, (uint32_t) anim.palette, anim.palette_len, offset);

        for(uint32_t i = 0; i < anim.frame_count; i++){
            uint32_t frameoffset = i * anim.width * anim.height;
            uint32_t limit = sh.winfo.xlen * sh.winfo.ylen;
            for(uint32_t j = 0; j < limit; j++){
                sh.winfo.main_buffer[j] = anim.map[frameoffset + j] - offset;
            }
            window_update(&(sh.winfo));
            delay(4);
        }

        anim_delete(&anim);
        cursor_on();
    }
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

    window_update(&(sh.winfo));

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
            window_update(&(sh.winfo));
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
    string_t string = str_new("\n");
    str_add(&string, sh.dir.path);
    str_add(&string, "> ");

    print(string.content);
    cursor_limit(sh.cursor_x, sh.cursor_y);
    
    str_delete(&string);
}

void shell_evaluate(){
    sh.reader.buffer_addr[sh.reader.max_idx] = 0;

    parser_parse(&sh_parser, sh.reader.buffer_addr, ' ');

    string_t message = str_new("");
    
    if (sh_parser.word_count > 0){
        if(strcmp(sh_parser.content[0], "clear") == 0){
            if(sh_parser.word_count == 1) shell_clear();
            else str_add(&message, "\nclear: Invalid argument\n");
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
                str_add(&message, "\ncd: No such directory: ");
                str_add(&message, sh_parser.content[1]);
                str_addc(&message, '\n');
            }
        }
        else if(strcmp(sh_parser.content[0], "mkdir") == 0){
            if(sh_parser.word_count != 2){
                str_add(&message, "\nmkdir: Invalid command\n");
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
                str_add(&message, "\nls: ");
                str_add(&message, sh_parser.content[1]);
                str_add(&message, ": No such file or directory\n");
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
                str_add(&message, "\ncat: ");
                str_add(&message, sh_parser.content[1]);
                str_add(&message, ": Is a directory\n");
            } else {
                str_add(&message, "\ncat: ");
                str_add(&message, sh_parser.content[1]);
                str_add(&message, ": No such file\n");
            }
        }
        else if(strcmp(sh_parser.content[0], "changebg") == 0){
            if(sh_parser.word_count == 1){
                str_add(&message, "\nchangebg: No image provided\n");
            }
            else if (sh_parser.word_count > 2){
                str_add(&message, "\nchangebg: Invalid argument\n");
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
                    
                    if(!valid) str_add(&message, "\nchangebg: Invalid file\n");
                    else app_change_background(sh_parser.content[1]);
                }
                else{
                    str_add(&message, "\nchangebg: No such file: ");
                    str_add(&message, sh_parser.content[1]);
                    str_addc(&message, '\n');
                }
            }
        }
        else if(strcmp(sh_parser.content[0], "playanim") == 0){
            if(sh_parser.word_count == 1){
                str_add(&message, "\nplayanim: No animation provided\n");
            }
            else if (sh_parser.word_count > 2){
                str_add(&message, "\nplayanim: Invalid argument\n");
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
                            valid = sh_parser.content[1][len-1] == 'm' &&
                                    sh_parser.content[1][len-2] == 'n' && 
                                    sh_parser.content[1][len-3] == 'a';
                        }
                    }
                    
                    if(!valid) str_add(&message, "\nplayanim: Invalid file\n");
                    else{
                        print("\nLoading animation...");
                        app_play_animation(sh_parser.content[1]);
                        print("\n");                
                    }
                }
                else{
                    str_add(&message, "\nplayanim: No such file: ");
                    str_add(&message, sh_parser.content[1]);
                    str_addc(&message, '\n');
                }
            }
        }

        // TODO: Review
        else if(strcmp(sh_parser.content[0], "ps") == 0){
            if(sh_parser.word_count == 1) ps();
            else str_add(&message, "\nps: Invalid argument\n");
        }
        else if(strcmp(sh_parser.content[0], "kill") == 0){
            if(sh_parser.word_count == 2){
                if(int_parse_string_valid(sh_parser.content[1])){
                    uint32_t pid = int_parse_string(sh_parser.content[1]);
                    if(pid == 0){
                        str_add(&message, "\nkill: Terminating the kernel is not allowed\n");
                    } else{
                        kill(pid);
                    }
                } else{
                    str_add(&message, "\nkill: Invalid argument\n");
                }
            } else{
                str_add(&message, "\nkill: Invalid argument\n");
            }
        }
        else if (is_filepath_valid(sh_parser.content[0], sh.dir.cluster_number)){
            FAT32DriverRequest req = path_to_file_request(sh_parser.content[0], sh.dir.cluster_number);
            if( req.ext[0] == 'p' &&	
                req.ext[1] == 'r' &&	
                req.ext[2] == 'g'	
            ){
                exec(&req, sh_parser);
            } else{
                str_add(&message, "\nShell: Non-executable file\n");
            }
        }
        else{
            str_add(&message, "\nNo Command found: ");
            str_add(&message, sh.reader.buffer_addr);
            str_addc(&message, '\n');
        }
    }

    sout_t outstream = sout_newstr(message);
    sout_printall(&outstream);

    str_delete(&message);
    sout_clear(&outstream);
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
                    window_update(&(sh.winfo));
                }
                break;

            case RARROW_CHAR:
                if(cursor_move(1)){
                    reader_move(1);
                    window_update(&(sh.winfo));
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
                    print_char_color(buf[0], sh.default_input_color);
                    // 14
                }
                break;
        }
        cursor_blinking();
    }

    return 0;
}


