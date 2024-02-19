
#ifndef _TIME_H
#define _TIME_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Struct to store time information
 * Should be self explanatory
 * 
*/
struct time{
    uint8_t century;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

/**
 * Get current time information
 * 
 * @return          current time
*/
struct time get_time();

#endif