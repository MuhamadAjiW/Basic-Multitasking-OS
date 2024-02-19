#ifndef _IOSTREAM_H
#define _IOSTREAM_H

#include <stdint.h>
#include <stdbool.h>

#include "string.h"

//TODO: Document
typedef struct sout_t{
    string_t string;
    unsigned int loc;
    uint8_t valid;
} sout_t;

sout_t sout_newstr(string_t initial);
sout_t sout_newc(char* initial);
void sout_clear(sout_t* sout);
void sout_printall(sout_t* sout);
void sout_printall_ws(sout_t* sout);
string_t sout_getln(sout_t* sout);
void sout_println(sout_t* sout);
void sout_printc(sout_t* sout);

#endif