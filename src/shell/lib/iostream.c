
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "../lib-header/iostream.h"
#include "../lib-header/stdio.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/string.h"
#include "../lib-header/shell.h"

sout_t sout_newstr(string_t initial){
    sout_t retval;
    retval.string = str_newcopy(initial);

    if(initial.len > 0) retval.valid = 1;
    else retval.valid = 0;

    retval.loc = 0;

    return retval;
}

sout_t sout_newc(char* initial){
    string_t temp = str_new(initial);
    sout_t retval = sout_newstr(temp);
    str_delete(&temp);
    return retval;
}

void sout_clear(sout_t* sout){
    str_delete(&(sout->string));
    sout->valid = 0;
    sout->loc = 0;
}

void sout_printall(sout_t* sout){
    if(sout->valid){
        print(sout->string.content);
        sout->loc = sout->string.len;
        sout->valid = 0;    
    }
}

void sout_printall_ws(sout_t* sout){
    if(sout->valid){
        string_t print_str = sout_getln(sout);

        uint8_t cursor_y = cursor_get_y();

        uint8_t line_counter = 0;
        uint8_t fit = 1;

        char buf[2] = {0, 0};

        print(print_str.content);
        str_delete(&print_str);
        print_str = str_new("");
        while(sout->valid && buf[0] != ESC_CHAR){
            string_t addstr = sout_getln(sout);
            //print(addstr.content);

            str_concat(&print_str, addstr);
            str_delete(&addstr);
            line_counter++;

            if(cursor_y + line_counter >= 23 && sout->valid){
                fit = 0;
                print(print_str.content);
                
                while(sout->valid){
                    while (buf[0] == 0){
                        buf[0] = sys_keyboard_get_char();;
                    }
                    if(buf[0] == ESC_CHAR) break;
                    else sout_println(sout);
                    buf[0] = 0;
                }
            }
        }

        if(fit){
            print(print_str.content);
        }

        str_delete(&print_str);
    }
}

string_t sout_getln(sout_t* sout){
    string_t retval = {0};
    if(sout->valid){
        uint32_t counter = 0;
        uint32_t loc = sout->loc;

        uint8_t cursor_x = cursor_get_x();

        while (sout->string.content[loc + counter] != '\n' && sout->string.content[loc + counter] != 0 && cursor_x + counter < 63){
            counter++;
        }
        char* char_str = (char*) malloc (counter + 2);

        for(uint32_t i = 0; i <= counter; i++){
            char_str[i] = sout->string.content[loc + i];
        }
        if(sout->string.content[loc + counter] == 0) {
            sout->valid = 0;
            sout->loc = sout->string.len;
        }
        else{
            sout->loc += counter + 1;
            char_str[counter + 1] = 0;
        }

        retval = str_new(char_str);
        free(char_str);
    }
    return retval;
}

void sout_println(sout_t* sout){
    if(sout->valid){
        string_t str = sout_getln(sout);
        print(str.content);
        str_delete(&str);
    }
}

void sout_printc(sout_t* sout){
    if(sout->valid){
        print_char(sout->string.content[sout->loc]);
        sout->loc++;
        if(sout->loc >= sout->string.len) sout->valid = 0;
    }
}
