#include "../lib-header/string.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/syscall.h"
#include "../lib-header/parser.h"

// Kalo split jangan lupa clear, memory leak ntar
void parser_clear(parser_t* parser){
    if(parser->status){
        parser->status = 0;
        if(parser->content != 0){
            for(uint32_t i = 0; i < parser->word_count; i++){
                free(parser->content[i]);
            }

            free(parser->content);
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
        parser->content = (char**) malloc(sizeof(char*) * parser->word_count);

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
                intermediary = (char*) malloc(sizeof(char) * letter_counter + 1);
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
