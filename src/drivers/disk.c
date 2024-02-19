
#include <stdint.h>
#include <stdbool.h>

#include "../lib-header/portio.h"

void read_blocks(uint32_t target_address, uint32_t lba, uint8_t sector_count){
    while(in(0x1F7) & 0x80){} 
	out(0x1F6, 0xE0 | (0xF0 << 4) |((lba >>24) & 0xF));
	out(0x1F2, sector_count);
	out(0x1F3, (uint8_t) lba);
	out(0x1F4, (uint8_t)(lba >> 8));
	out(0x1F5, (uint8_t)(lba >> 16));
	
	out(0x1F7, 0x20);

	uint16_t *target = (uint16_t*) target_address;
	
	for (uint16_t j = 0; j < sector_count; j++){
	    while(in(0x1F7) & 0x80){}
	    while(!(in(0x1F7) & 0x40)){}
		for(uint16_t i = 0; i < 256; i++){
			target[i] = in2(0x1F0);
		}
	}

	return;
}

void write_blocks(uint32_t* entry, uint32_t lba, uint8_t sector_count){
	while(in(0x1F7) & 0x80){}
	out(0x1F6, 0xE0 | ((lba >>24) & 0xF));
	out(0x1F2, sector_count);
	out(0x1F3, (uint8_t) lba);
	out(0x1F4, (uint8_t)(lba >> 8));
	out(0x1F5, (uint8_t)(lba >> 16)); 
	out(0x1F7, 0x30);

	for (int j = 0; j < sector_count; j++){
	    while(in(0x1F7) & 0x80){}
	    while(!(in(0x1F7) & 0x40)){}
		for(int i = 0; i < 128; i++){
			out4(0x1F0, entry[i]);
		}
	}

	return;
}
