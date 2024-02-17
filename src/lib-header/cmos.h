
#ifndef _CMOS_H
#define _CMOS_H

#include "stdtype.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

#define TIMEZONE 7

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

/**
 * Set 24 hour flag in CMOS via porting
 * 
*/
void cmos_initialize();


/**
 * Check busy port in CMOS
 * 
*/
bool cmos_check_update();


/**
 * Get values of CMOS registers
 * 
 * @param reg requested register index
 * 
 * @return value of requested register
*/
uint8_t cmos_get_reg(int reg);


/**
 * Update static rtc values
 * 
*/
void cmos_read_rtc();


/**
 * Return values of stored static rtc values
 * 
 * @return static rtc values
*/
struct cmos_reader cmos_get_data();

#endif