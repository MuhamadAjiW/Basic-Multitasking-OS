#include "../lib-header/cmos.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/portio.h"

/**
 * Static struct to store time information from CMOS
 * Reading should always be done to this variable
 * 
*/
static cmos_reader cmos_data = {
    .century = 0,
    .second = 0,
    .minute = 0,
    .hour = 0,
    .day = 0,
    .month = 0,
    .year = 0
};

void cmos_initialize(){
    out(CMOS_ADDR, 0x0b);                   // check if register is busy
    uint8_t former = in(CMOS_DATA);         // get old register values
    out(CMOS_DATA, former | 0x02 | 0x04);   // tick 24h flag
}

bool cmos_check_update(){
    out(CMOS_ADDR, 0x0A);                   //check if register is busy
    return (in(CMOS_DATA) & 0x80);          //retrieve update flag from register
}
 
uint8_t cmos_get_reg(int reg){
    out(CMOS_ADDR, reg);                    //check if register is busy
    return in(CMOS_DATA);                   //register data
}
 
void cmos_read_rtc(){
    cmos_initialize();

    while (cmos_check_update()){}; //nunggu clear
    cmos_data.second = cmos_get_reg(0x00);
    cmos_data.minute = cmos_get_reg(0x02);
    cmos_data.hour = (cmos_get_reg(0x04) + TIMEZONE) % 24;
    cmos_data.day = cmos_get_reg(0x07);
    cmos_data.month = cmos_get_reg(0x08);
    cmos_data.year = cmos_get_reg(0x09);
    cmos_data.century = cmos_get_reg(0x32);
}

cmos_reader cmos_get_data(){
    cmos_read_rtc();
    return cmos_data;
}
