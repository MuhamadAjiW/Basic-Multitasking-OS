#include "../lib-header/keyboard.h"
#include "../lib-header/portio.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/gdt.h"
#include "../lib-header/idt.h"

#include "../lib-header/framebuffer.h"

KeyboardDriverState keyboard_state = {0};

const char scantable[64] = {
    0,0,'1','2','3','4','5','6','7','8',
    '9','0','-','=',0,0,'q','w','e','r',
    't','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';',
    '\'',0,0,0,'z','x','c','v','b','n',
    'm',',','.','/',0,0,0,' ',0,0,
    0,0,0,0
};
const char scantableCaps[64] = {
    0,0,'!','@','#','$','%','^','&','*',
    '(',')','_','+',0,0,'Q','W','E','R',
    'T','Y','U','I','O','P','{','}','\n',0,
    'A','S','D','F','G','H','J','K','L',':',
    '"',0,0,0,'Z','X','C','V','B','N',
    'M','<','>','?',0,0,0,' ',0,0,
    0,0,0,0
};

void keyboard_isr() {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    if (keyboard_state.on){
        keyboard_process_input(scancode);
    }

    pic_ack(IRQ_KEYBOARD);
}

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
    keyboard_state.on = 1;
}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void){
    keyboard_state.on = 0;
}

// Get keyboard buffer values - @param buf Pointer to char buffer
void keyboard_flush_buffer(char *buf){
    *buf = keyboard_state.buffer;
    keyboard_state.buffer = NULL_CHAR;
}

// Check whether keyboard ISR is active or not - @return Equal with keyboard_input_on value
bool keyboard_is_blocking(void){
    return keyboard_state.on;
}

void keyboard_process_input(uint8_t input){
        switch (input){
            // Caps mechanism
            case 58: keyboard_state.caps = keyboard_state.caps? 0 : 1; break;
            
            // Shift mechanism
            case 42: keyboard_state.shift = 1; break;
            case 170: keyboard_state.shift = 0; break;
            case 54: keyboard_state.shift = 1; break;
            case 182: keyboard_state.shift = 0; break;

            // Allowed other keys
            case BACKSPACE_CHAR:
            case TAB_CHAR:
            case LARROW_CHAR:
            case RARROW_CHAR:
            case UARROW_CHAR:
            case DARROW_CHAR:
            case ESC_CHAR:
                keyboard_state.buffer = input;
                break;

            default:
                if(input < 64){
                    if(scantable[input] != 0 && scantableCaps[input] != 0){
                        if (keyboard_state.caps){
                            if(keyboard_state.shift){
                                if(scantableCaps[input] >= 'A' && scantableCaps[input] <= 'Z'){
                                    keyboard_state.buffer = scantable[input];
                                }
                                else{
                                    keyboard_state.buffer = scantableCaps[input];
                                }
                            }
                            else{
                                if(scantableCaps[input] >= 'A' && scantableCaps[input] <= 'Z'){
                                    keyboard_state.buffer = scantableCaps[input];
                                }
                                else{
                                    keyboard_state.buffer = scantable[input];
                                }
                            }
                        }
                        else {
                            if (keyboard_state.shift){
                                keyboard_state.buffer = scantableCaps[input];
                            }
                            else{
                                keyboard_state.buffer = scantable[input];
                            }
                        }
                    }
                }
                break;
        }
}