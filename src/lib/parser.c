#include "../lib-header/stdmem.h"
#include "../lib-header/string.h"
#include "../lib-header/memory_manager.h"
#include "../lib-header/syscall.h"
#include "../lib-header/parser.h"

// Kalo split jangan lupa clear, memory leak ntar
void parser_clear(parser_t* parser){
    if(parser->status){
        parser->status = 0;
        if(parser->content != 0){

            // In case these parts are extracted
            for(uint32_t i = 0; i < parser->word_count; i++){
                kfree(parser->content[i]);
            }

            kfree(parser->content);
        }
        parser->word_count = 0;
    }
}

void parser_parse(parser_t* parser, char* string, char splitter){
    if(!parser->status){
        parser->status = 1;

        int index = 0;
        int letter_counter = 0;
        char* intermediary;
        parser->word_count = 0;
        while (string[index] != 0){
            if(string[index] == splitter){
                index++;
                continue;
            }
            else{
                while (string[index] != splitter && string[index] != 0 && string[index] != '\n'){
                    index++;
                }

                parser->word_count++;
            }
        }
        parser->content = (char**) kmalloc(sizeof(char*) * parser->word_count);

        index = 0;
        letter_counter = 0;
        parser->word_count = 0;
        while (string[index] != 0){
            if(string[index] == splitter){
                index++;
                continue;
            }
            else{
                letter_counter = 0;

                while (string[index] != splitter && string[index] != 0 && string[index] != '\n'){
                    letter_counter++;
                    index++;
                }
                intermediary = (char*) kmalloc(sizeof(char) * letter_counter + 1);
                parser->content[parser->word_count] = intermediary;

                for(int i = 0; i < letter_counter; i++){
                    parser->content[parser->word_count][i] = string[index - letter_counter + i];
                }

                parser->content[parser->word_count][letter_counter] = 0;
                parser->word_count++;
            }
        }
    }
}


// TODO: These both are dangerous to be made syscall
// can be improved somewhat, not sure though
char** parser_args_extract(parser_t* parser, uint32_t target_pid){
    uint32_t word_count = parser->word_count - 1;
    char** args = (char**) kmalloc(sizeof(char*) * word_count);

    allocator* allocated_word;
    uint32_t parser_idx;
    uint32_t word_len;
    for (uint32_t i = 0; i < word_count; i++){
        parser_idx = i + 1;
        allocated_word = (allocator*) (parser->content[parser_idx] - sizeof(allocator));

        word_len = allocated_word->size;
        args[i] = (char*) kmalloc(sizeof(char) * word_len);
        memcpy(args[i], parser->content[parser_idx], word_len);

        allocated_word = (allocator*)(args[i] - sizeof(allocator));
        allocated_word->pid = target_pid;
    }

    return args;
}

// Commented since the garbage collector will clean the memory after a task is done anyway
// But worth keeping, might need it
// void parser_args_clear(uint32_t argc, char** argv){
//     if(argv != 0){

//         // In case these parts are extracted
//         for(uint32_t i = 0; i < argc; i++){
//             kfree(argv[i]);
//         }

//         kfree(argv);
//     }
// }