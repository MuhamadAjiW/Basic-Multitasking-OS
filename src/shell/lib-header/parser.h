
#ifndef _PARSER_H
#define _PARSER_H

#include <stdint.h>
#include <stdbool.h>

//TODO: Document
typedef struct parser_t{
    char** content;
    uint32_t word_count;
    uint8_t status;
} parser_t;

void parser_clear(parser_t* parser);
void parser_parse(parser_t* parser, char* string, char splitter);

#endif