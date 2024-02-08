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
    img->map = malloc(size);

    for(uint32_t i = 0; i < size; i++){
        img->map[i] = *(((char*)img_file.content) + roamer);
        roamer++;
    }
    roamer++;

    img->palette_len  = *(((uint8_t*)img_file.content) + roamer);
    img->palette = malloc(3 * img->palette_len);
    roamer++;
    roamer++;

    for(uint32_t i = 0; i < img->palette_len * 3; i++){
        img->palette[i] = *(((uint8_t*)img_file.content) + roamer);
        roamer++;
    }
    roamer++;

    closef(&img_file);
}

void image_delete(image_info* img){
    free(img->map);
    free(img->palette);
}

void image_change_palette(image_info img){
    //TODO: This is very temporary, make a proper config later on (possibly based on json)
    
    char imgoffset[4] = {0};
    int_to_string(img.palette_len, imgoffset);
    struct FAT32DriverRequest palette_config = {
        .buf                   = (uint8_t*) imgoffset,
        .name                  = "colors",
        .ext                   = "cnf",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER + 1,
        .buffer_size           = sizeof(imgoffset),
    };
    if (is_filepath_valid("system/colors.cnf", ROOT_CLUSTER_NUMBER)){
        deletef(&palette_config);
    }
    writef(&palette_config);
    
    syscall(SYSCALL_GRAPHICS_PALETTE_UPDATE, (uint32_t) img.palette, img.palette_len, 0);
}