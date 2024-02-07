#ifndef _STRING_H
#define _STRING_H

#include "stdtype.h"

/**
 * Convert integer to a null terminated string
 * 
 * @param x integer
 * @param str buffer to store null terminated string
 * 
*/
void int_to_string(int x, char str[]);

/**
 * Count the length of a null terminated string
 * @warning DO NOT USE ON NON-NULL-TERMINATED STRINGS
 * 
 * @param str null terminated string
 * 
 * @return length in integer
*/
int strlen(char str[]);

/**
 * Compare null terminated strings
 * @warning DO NOT USE ON NON-NULL-TERMINATED STRINGS
 * 
 * @param str1 null terminated string
 * @param str2 null terminated string
 * 
 * @return comparison result, 1 means str1 > str2, -1 means str1 < str2, 0 means str1 == str2
*/
int strcmp(char str1[], char str2[]);

/**
 * Copy null terminated strings
 * @warning DO NOT USE ON NON-NULL-TERMINATED STRINGS
 * 
 * @param dest Buffer to store copied string
 * @param src Null terminated string
 * 
*/
void strcpy(char dest[], char src[]);


//TODO: Document
typedef struct string_t{
    char* content;
    uint32_t len;
} string_t;

uint8_t int_parse_string_valid(char str[]);
int int_parse_string(char str[]);

string_t str_new(char* initial);
string_t str_newcopy(string_t source);
string_t str_splice_rear(string_t mainstring, uint32_t loc);
string_t str_splice_front(string_t mainstring, uint32_t loc);
void str_delete(string_t* string);
void str_concat(string_t* mainstring, string_t substring);
void str_consdot(string_t* mainstring, string_t substring);
void str_insertc(string_t* mainstring, char c, uint32_t loc);
char str_remove(string_t* mainstring, uint32_t loc);
void str_add(string_t* mainstring, char* substring);
void str_addc(string_t* mainstring, char c);

#endif