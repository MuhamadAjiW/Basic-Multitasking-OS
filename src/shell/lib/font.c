#include "../lib-header/stdio.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/commands-util.h"
#include "../lib-header/font.h"
#include "../lib-header/window_manager.h"

void font_load(font_info* finfo, char* path_from_root){
    for(int i = 0; i < 128; i++){
        finfo->font[i] = 0;
    }
    
    FAT32DriverRequest req = path_to_file_request(path_from_root, ROOT_CLUSTER_NUMBER);
    FAT32FileReader font_file = readf(req);

    finfo->width = *(((uint8_t*)font_file.content) + 0) - 48;
    finfo->height = *(((uint8_t*)font_file.content) + 1) - 48;

    uint32_t roamer = 0;
    while (*(((char*)font_file.content) + roamer) != '{'){
        roamer++;
    }
    roamer++;
    while(*(((char*)font_file.content) + roamer) == '\n' || *(((char*)font_file.content) + roamer) == '\r'){
        roamer++;
    }

    uint8_t line = 0;
    while (*(((char*)font_file.content) + roamer) != '}'){
        uint8_t number = 0;
        uint8_t numberofpixels = 0;

        while (*(((char*)font_file.content) + roamer) != ',' && *(((char*)font_file.content) + roamer) != '\n'){
            number++;
            roamer++;
        }
        if(*(((char*)font_file.content) + roamer) != '\n'){
            while(*(((char*)font_file.content) + roamer) == '\n' || *(((char*)font_file.content) + roamer) == '\r'){
                roamer++;
            }
        }
        if(*(((char*)font_file.content) + roamer) == ','){
            uint8_t multiplier = 1;
            for(int i = 1; i <= number; i++){
                numberofpixels += ((*(((uint8_t*)font_file.content) + roamer - i) - 48) * multiplier);
                multiplier *= 10;
            }
            if(numberofpixels != 0) {
                finfo->font[(uint8_t) line] = (uint32_t) malloc (numberofpixels);
                ((uint8_t*)finfo->font[line])[0] = numberofpixels;
            }
            
            roamer++;
        };

        for(int i = 1; i < numberofpixels; i++){
            number = ((*(((uint8_t*)font_file.content) + roamer) - 48) << 4) + (*(((uint8_t*)font_file.content) + roamer + 1) - 48); 
            
            ((uint8_t*)finfo->font[line])[i] = number;
            roamer += 2;
            if (*(((char*)font_file.content) + roamer) == ',') roamer++;
        }
        while(*(((char*)font_file.content) + roamer) == '\n' || *(((char*)font_file.content) + roamer) == '\r'){
            roamer++;
        }

        line++;
    }

    closef(font_file);
}

void font_clear(font_info* finfo){
    finfo->width = 0;
    finfo->height = 0;
    for(int i = 0; i < 128; i++){
        if(finfo->font[i] != 0){
            free((void*) finfo->font[i]);
            finfo->font[i] = 0;
        }
    }
}

void font_write(window_info* winfo, font_info finfo, uint8_t row, uint8_t col, char c, uint8_t color){
    uint8_t index = (uint8_t) c;

    if(finfo.font[index] != 0){
        uint16_t block_start_row = row * finfo.height;
        uint16_t block_start_col = col * finfo.width;

        uint8_t* addr = (uint8_t*) finfo.font[index];

        for(int i = 1; i < addr[0]; i++){
            window_draw_pixel(winfo, block_start_row + (addr[i] & 0xf), block_start_col + (addr[i] >> 4),color);
        }
    }

}