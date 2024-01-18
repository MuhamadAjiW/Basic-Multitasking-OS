#include "../lib-header/keyboard.h"
#include "../lib-header/portio.h"
#include "../lib-header/framebuffer.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/gdt.h"
#include "../lib-header/idt.h"

KeyboardDriverState keyboard_state = {
    .keyboard_input_on = 0,
    .buffer_index = 0,
    .keyboard_buffer = {0},
    .read_extended_mode = 0
};

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void keyboard_isr() {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    if (!keyboard_state.keyboard_input_on)
        keyboard_state.buffer_index = 0;
    else {
        char mapped_char = keyboard_scancode_1_to_ascii_map[scancode];
        // TODO : Implement scancode processing
        keyboard_state.keyboard_buffer[keyboard_state.buffer_index] = mapped_char;
        keyboard_state.buffer_index++;
        framebuffer_keyboard();
    }
    pic_ack(IRQ_KEYBOARD);
}

// Activate keyboard ISR / start listen keyboard & save to buffer
void keyboard_state_activate(void){
    keyboard_state.keyboard_input_on = 1;
}

// Deactivate keyboard ISR / stop listening keyboard interrupt
void keyboard_state_deactivate(void){
    keyboard_state.keyboard_input_on = 0;
}

// Get keyboard buffer values - @param buf Pointer to char buffer, recommended size at least KEYBOARD_BUFFER_SIZE
void get_keyboard_buffer(char *buf){
    for (uint8_t i = 0; i < 128; i++){
        buf[i] = keyboard_state.keyboard_buffer[i];
        keyboard_state.keyboard_buffer[i] = 0;
    }
    keyboard_state.buffer_index = 0;
}

// Check whether keyboard ISR is active or not - @return Equal with keyboard_input_on value
bool is_keyboard_blocking(void){
    return keyboard_state.keyboard_input_on;
}
