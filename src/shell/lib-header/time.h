
#ifndef _TIME_H
#define _TIME_H

#include "stdtype.h"

/**
 * Struct to store time information
 * Should be self explanatory
 * 
*/
typedef struct time{
    uint8_t century;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} time;

/**
 * Get current time information
 * 
 * @return          current time
*/
time get_time();

#endif