#include "../lib-header/stdtype.h"
#include "../lib-header/syscall.h"
#include "../lib-header/stdio.h"
#include "../lib-header/string.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/commands-util.h"
#include "../lib-header/image.h"

void image_load(image_info* img, char* path, uint32_t current_cluster){
    FAT32DriverRequest req = path_to_file_request(path, current_cluster);
    FAT32FileReader img_file = readf(&req);

    uint32_t roamer = 0;
    uint32_t number_count = 0;
    uint32_t multiplier = 1;
    uint32_t number_value = 0;
    for(uint32_t i = 0; i < 2; i++){
        number_count = 0;
        while (*(((char*)img_file.content) + roamer) != '\n'){
            number_count++;
            roamer++;
        }
        multiplier = 1;
        number_value = 0;
        for(uint32_t j = 1; j <= number_count; j++){
            number_value += ((*(((uint8_t*)img_file.content) + roamer - j) - 48) * multiplier);
            multiplier *= 10;
        }
        if(i == 0) img->width  = number_value;
        else img->height = number_value;
        roamer++;
    }
    uint32_t size = img->width * img->height;
    img->map = (uint32_t*) malloc (sizeof(uint32_t) * size);

    volatile uint32_t* image = (uint32_t*)((uint32_t)img_file.content + roamer);
    for(uint32_t i = 0; i < size; i++){
        img->map[i] = *(image + i);
    }

    closef(&img_file);
}

void image_delete(image_info* img){
    free(img->map);
}
