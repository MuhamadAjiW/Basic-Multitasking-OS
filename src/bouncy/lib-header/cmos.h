
#ifndef _CMOS_H
#define _CMOS_H

#include "stdtype.h"

/**
 * Struct to store time information from CMOS
 * Should be self explanatory
 * 
*/
struct cmos_reader{
    uint8_t century;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

#endif
