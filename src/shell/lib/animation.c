#include "../lib-header/stdtype.h"
#include "../lib-header/stdio.h"
#include "../lib-header/stdlib.h"
#include "../lib-header/commands-util.h"
#include "../lib-header/animation.h"

void anim_load(anim_info* anim, char* path, uint32_t current_cluster){
    FAT32DriverRequest req = path_to_file_request(path, current_cluster);
    FAT32FileReader anim_file = readf(&req);

    uint32_t roamer = 0;
    uint32_t number_count = 0;
    uint32_t multiplier = 1;
    uint32_t number_value = 0;
    for(uint32_t i = 0; i < 3; i++){
        number_count = 0;
        while (*(((char*)anim_file.content) + roamer) != '\n'){
            number_count++;
            roamer++;
        }
        multiplier = 1;
        number_value = 0;
        for(uint32_t j = 1; j <= number_count; j++){
            number_value += ((*(((uint8_t*)anim_file.content) + roamer - j) - 48) * multiplier);
            multiplier *= 10;
        }

        switch (i){
            case 0:
                anim->width  = number_value;
                break;

            case 1:
                anim->height = number_value;
                break;

            case 2:
                anim->frame_count  = number_value;
                break;
        }
        roamer++;
    }
    uint32_t size = anim->width * anim->height * anim->frame_count;
    anim->map = malloc(size);

    for(uint32_t i = 0; i < size; i++){
        anim->map[i] = *(((char*)anim_file.content) + roamer);
        roamer++;
    }
    roamer++;

    anim->palette_len  = *(((uint8_t*)anim_file.content) + roamer);
    anim->palette = malloc(3 * anim->palette_len);
    roamer++;
    roamer++;

    for(uint32_t i = 0; i < anim->palette_len * 3; i++){
        anim->palette[i] = *(((uint8_t*)anim_file.content) + roamer);
        roamer++;
    }
    roamer++;

    closef(&anim_file);
}

void anim_delete(anim_info* anim){
    free(anim->map);
    free(anim->palette);
}