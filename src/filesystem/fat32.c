#include "../lib-header/fat32.h"
#include "../lib-header/stdtype.h"
#include "../lib-header/stdmem.h"
#include "../lib-header/disk.h"
#include "../lib-header/cmos.h"
#include "../lib-header/memory_manager.h"

static cmos_reader cmos = {0};
DirectoryEntry emptyEntry = {0};

uint8_t is_empty_storage(){
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    read_clusters((void*)reader, RESERVED_CLUSTER_NUMBER, 1);
    if(memcmp(reader, "fs_signature", 12) == 0){
        return 0;
    }
    return 1;
}

void initialize_filesystem_fat32(){
    if(is_empty_storage()){
        //initialize signature
        uint32_t signature[CLUSTER_SIZE/4] = {0};
        signature[0] = 'f' | ('s' << 8) | ('_' << 16) | ('s' << 24);
        signature[1] = 'i' | ('g' << 8) | ('n' << 16) | ('a' << 24);
        signature[2] = 't' | ('u' << 8) | ('r' << 16) | ('e' << 24);
        write_clusters((void*)signature, 0, 1);

        //initialize root
        DirectoryTable table = {0};
        DirectoryInfo info = {
            .filename = {'r', 'o', 'o', 't','\0', '\0', '\0', '\0'},
            .extension = {'\0', '\0', '\0'},
            .read_only = 0,
            .hidden = 0,
            .system = 0,
            .volume_id = 0,
            .directory = 1,
            .archive = 0,
            .resbit1 = 0,
            .resbit2 = 0,
            .reserved = 0,
            .cluster_number = ROOT_CLUSTER_NUMBER,
            .parent_base_cluster = ROOT_CLUSTER_NUMBER,
            .parent_actual_cluster = ROOT_CLUSTER_NUMBER,
            .entry_number = 0,
            .size = 32
        };
        table.info = info;

        void* writer = (void*) &table;
        write_clusters(writer, ROOT_CLUSTER_NUMBER, 1);

        //initialize FAT
        FAT32FileAllocationTable FAT_Table = {
            .sector_next = 0
        };
        FAT_Table.sector_next[RESERVED_CLUSTER_NUMBER] = END_OF_FILE;
        for (int i = 1; i < FAT_CLUSTER_LENGTH; i++){
            FAT_Table.sector_next[i] = i + 1;
        }
        FAT_Table.sector_next[FAT_CLUSTER_LENGTH] = END_OF_FILE;
        FAT_Table.sector_next[ROOT_CLUSTER_NUMBER] = END_OF_FILE;
        
        write_fat(&FAT_Table);

        struct FAT32DriverRequest request = {
            .buf                   = (uint8_t*)0,
            .name                  = "system",
            .ext                   = "\0\0\0",
            .parent_cluster_number = ROOT_CLUSTER_NUMBER,
            .buffer_size           = 0,
        };
        write(request);
        entryflags systemflags = {
            .read_only = 0,
            .hidden = 0,
            .system = 1,
            .volume_id = 0,
            .archive = 0,
            .resbit1 = 0,
            .resbit2 = 0
        };
        set_entry_flag(request, systemflags);
    }
}

void write_fat(FAT32FileAllocationTable* FAT_request){
    int cluster = 0;
    for(uint16_t i = 0; i < FAT_CLUSTER_LENGTH; i++){
        write_clusters(((void*)FAT_request + cluster), i + 1, 1);
        cluster += 512;
    }
}

void read_fat(FAT32FileAllocationTable* FAT_destination){
    int cluster = 0;
    for(uint16_t i = 0; i < FAT_CLUSTER_LENGTH; i++){
        read_clusters(((void*)FAT_destination + cluster), i + 1, 1);
        cluster += 512;
    }
}

void create_entry(FAT32DriverRequest request, uint16_t cluster_number, FAT32FileAllocationTable* fat){
    cmos = cmos_get_data();
    DirectoryEntry add = {
        .filename = {0},
        .extension = {0},
        .read_only = 0,
        .hidden = 0,
        .system = 0,
        .volume_id = 0,
        .directory = 0,
        .archive = 0,
        .resbit1 = 0,
        .resbit2 = 0,
        .reserved = 0,
        .creation_time_low = 0,
        .creation_time_seconds = cmos.second,
        .creation_time_minutes = cmos.minute,
        .creation_time_hours = cmos.hour,
        .creation_time_day = cmos.day,
        .creation_time_month = cmos.month,
        .creation_time_year = cmos.year,
        .accessed_time_day = cmos.day,
        .accessed_time_month = cmos.month,
        .accessed_time_year = cmos.year,
        .high_bits = 0,
        .modification_time_seconds = cmos.second,
        .modification_time_minutes = cmos.minute,
        .modification_time_hours = cmos.hour,
        .modification_time_day = cmos.day,
        .modification_time_month = cmos.month,
        .modifcation_time_year = cmos.year,
        .cluster_number = cluster_number,
        .size = request.buffer_size
    };
    if(request.buffer_size == 0){
        add.size = 32;
        add.directory = 1;
    }
    else{
        memcpy(add.extension, request.ext, 3);
    }

    memcpy(add.filename, request.name, 8);

    uint32_t reader[CLUSTER_SIZE/4] = {0};
    read_clusters((void*)reader, request.parent_cluster_number, 1);
    DirectoryTable table = as_directory(reader);
    int i;

    uint32_t currentCluster = request.parent_cluster_number;
    uint8_t added = 0;
    while(!added){
        for(i = 0; i < ENTRY_COUNT; i ++){
            if(memcmp(&table.entry[i], &emptyEntry, 32) == 0){
                added = 1;
                break;
            }
        }
        if(i < ENTRY_COUNT){
            memcpy(&table.entry[i], &add, 32);
            void* writer = (void*) &table;
            write_clusters(writer, currentCluster, 1);
        }
        else{
            currentCluster = fat->sector_next[currentCluster];

            if(currentCluster == END_OF_FILE)
                currentCluster = expand_folder(request.parent_cluster_number, fat);

            read_clusters((void*)reader, currentCluster, 1);
            table = as_directory(reader);
        }
    }

    if(add.directory){
        init_directory_table(request, cluster_number, currentCluster, i);
    }
}

void update_size(FAT32DriverRequest request, char category, FAT32FileAllocationTable* fat){
    DirectoryTable table = {0};
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    uint32_t currentCluster = request.parent_cluster_number;
    uint32_t entryCluster;
    uint16_t entryNumber;

    cmos = cmos_get_data();
    while (currentCluster != ROOT_CLUSTER_NUMBER){
        do{
            read_clusters((void*)reader, currentCluster, 1);
            table = as_directory(reader);

            if(request.buffer_size == 0){
                if(category == '+') table.info.size += 32;
                else if(category == '-') table.info.size -= 32;
            }
            else{
                if(category == '+') table.info.size += request.buffer_size;
                else if(category == '-') table.info.size -= request.buffer_size;
            }

            write_clusters((void*) &table, currentCluster, 1);

            currentCluster = fat->sector_next[currentCluster];
        } while (currentCluster != END_OF_FILE);

        currentCluster = table.info.parent_base_cluster;
        entryNumber = table.info.entry_number;
        entryCluster = table.info.parent_actual_cluster;

        read_clusters((void*)reader, entryCluster, 1);
        table = as_directory(reader);
        update_file_size(&table.entry[entryNumber], request.buffer_size, category);
        update_file_time(&table.entry[entryNumber]);
        
        write_clusters((void*) &table, entryCluster, 1);
    }

    do{
        read_clusters((void*)reader, currentCluster, 1);
        table = as_directory(reader);

        if(request.buffer_size == 0){
            if(category == '+') table.info.size += 32;
            else if(category == '-') table.info.size -= 32;
        }
        else{
            if(category == '+') table.info.size += request.buffer_size;
            else if(category == '-') table.info.size -= request.buffer_size;
        }

        write_clusters((void*) &table, currentCluster, 1);

        currentCluster = fat->sector_next[currentCluster];
    } while (currentCluster != END_OF_FILE);
}

uint8_t write(FAT32DriverRequest request){
    if(request.parent_cluster_number < 2){
        return 2;
    }
    else if (!is_directory(request.parent_cluster_number)){
        return 2;
    }
    else if (name_exists(request)){
        return 1;
    }


    FAT32FileAllocationTable fat = {0};
    read_fat(&fat);

    update_size(request, '+', &fat);

    uint32_t size = request.buffer_size;
    int index = 0;
    int cachedindex = ROOT_CLUSTER_NUMBER;
    
    bool large = 0;
    bool running = 1;
    bool created = 0;
    uint16_t startCluster = 0;
    uint16_t i;

    while (running){
        for (i = cachedindex + 1; i < CLUSTER_COUNT; i++){ //ini untuk nulis entry di fat, jadi gak perlu handle untuk extendable folder
            if(fat.sector_next[i] == 0){
                break;
            }
        }

        if (!created){
            startCluster = i;
            created = 1;
        }
        
        if(size != 0){
            void* writer = request.buf + index * 2048;

            if(size < CLUSTER_SIZE){
                uint8_t reader[CLUSTER_SIZE] = {0};
                uint8_t* mover = (uint8_t*) writer;
                for(uint32_t i = 0; i < size; i++){
                    reader[i] = mover[i];
                }
                write_clusters((void*) reader, i, 1);
            }
            else{
                write_clusters(writer, i, 1);
            }

            index++;
        }

        if(large){
            fat.sector_next[cachedindex] = i;
        }

        if(size <= CLUSTER_SIZE){
            fat.sector_next[i] = END_OF_FILE;
            running = 0;
        }
        else{
            size -= CLUSTER_SIZE;
            large = 1;
            cachedindex = i;
        }
    };
    create_entry(request, startCluster, &fat);
    
    write_fat(&fat);
    return 0;
}


void init_directory_table(
    FAT32DriverRequest request,
    uint16_t cluster_number,
    uint16_t parent_actual_cluster,
    uint16_t entry_number
){
    DirectoryTable table = {0};

    memcpy(&table.info.filename, &request.name, 8);
    table.info.directory = 1;
    table.info.cluster_number = cluster_number;
    table.info.parent_base_cluster = request.parent_cluster_number;
    table.info.parent_actual_cluster = parent_actual_cluster;
    table.info.entry_number = entry_number;
    table.info.size = 32;
    
    void* writer = (void*) &table;
    write_clusters(writer, cluster_number, 1);
}

void deleteRecurse(FAT32DriverRequest request, FAT32FileAllocationTable* fat){
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    uint32_t empty_cluster[CLUSTER_SIZE/4] = {0};
    DirectoryEntry self = get_self_entry(request);
    uint32_t marker = 0;
    uint32_t currentCluster = 0;
    uint8_t deleting = 0;
    void* writer;

    if(self.directory == 1){
        deleteFolder(self.cluster_number, fat);
    }

    currentCluster = self.cluster_number;
    marker = fat->sector_next[currentCluster];
    deleting = 1;
    
    while (deleting){
        fat->sector_next[currentCluster] = 0;
        write_clusters((void*) empty_cluster, currentCluster, 1);
        if(marker == END_OF_FILE){
            deleting = 0;
        }
        else{
            currentCluster = marker;
            marker = fat->sector_next[currentCluster] = 0;
        }
    }

    DirectoryTable parent_table;
    uint32_t roaming_cluster = request.parent_cluster_number;
    uint8_t found = 0;

    while (!found && roaming_cluster != END_OF_FILE){
        read_clusters((void*)reader, roaming_cluster, 1);
        parent_table = as_directory(reader);
        for(int i = 0; i < ENTRY_COUNT; i++){
            if(memcmp(&parent_table.entry[i].filename, &request.name, 8) == 0){
                if (request.buffer_size == 0 ||
                    (request.buffer_size != 0 && memcmp(&parent_table.entry[i].extension, &request.ext, 3) == 0)){
                    parent_table.entry[i] = emptyEntry;
                    found = 1;
                    break;
                }
            }
        }
        if (!found){
            roaming_cluster = fat->sector_next[roaming_cluster];
        }
    }
    writer = (void*) &parent_table;
    write_clusters(writer, roaming_cluster, 1);    
}

void deleteFolder(uint16_t cluster_number, FAT32FileAllocationTable* fat){
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    DirectoryTable table;
    FAT32DriverRequest request;

    uint32_t currentCluster = cluster_number;
    do{
        read_clusters((void*)reader, currentCluster, 1);
        table = as_directory(reader);
        for(int i = 0; i < ENTRY_COUNT; i++){
            if (memcmp(&table.entry[i], &emptyEntry, 32) != 0){
                memcpy(&request.name, &table.entry[i].filename, 8);
                memcpy(&request.ext, &table.entry[i].extension, 3);
                request.parent_cluster_number = cluster_number;

                deleteRecurse(request, fat);
            }
        }
        currentCluster = fat->sector_next[currentCluster];
    } while (currentCluster != END_OF_FILE);
}

uint8_t delete(FAT32DriverRequest request){
    FAT32FileAllocationTable fat;
    read_fat(&fat);

    update_size(request, '-', &fat);

    uint32_t empty_cluster[CLUSTER_SIZE/4] = {0};
    DirectoryEntry self = get_self_entry(request);
    uint32_t marker = 0;
    uint32_t currentCluster = 0;
    uint8_t deleting = 0;
    void* writer;

    if(self.directory == 1){
        deleteFolder(self.cluster_number, &fat);
    }

    currentCluster = self.cluster_number;
    marker = fat.sector_next[currentCluster];
    deleting = 1;
    
    while (deleting){
        fat.sector_next[currentCluster] = 0;
        writer = (void*) empty_cluster;
        write_clusters(writer, currentCluster, 1);
        if(marker == END_OF_FILE){
            deleting = 0;
        }
        else{
            currentCluster = marker;
            marker = fat.sector_next[currentCluster];
        }
    }
    write_fat(&fat);

    uint32_t reader[CLUSTER_SIZE/4] = {0};
    DirectoryTable parent_table;
    uint32_t roaming_cluster = request.parent_cluster_number;
    uint8_t found = 0;

    while (!found && roaming_cluster != END_OF_FILE){
        read_clusters((void*)reader, roaming_cluster, 1);
        parent_table = as_directory(reader);
        for(int i = 0; i < ENTRY_COUNT; i++){
            if(memcmp(&parent_table.entry[i].filename, &request.name, 8) == 0){
                if (request.buffer_size == 0 ||
                    (request.buffer_size != 0 && memcmp(&parent_table.entry[i].extension, &request.ext, 3) == 0)){
                    parent_table.entry[i] = emptyEntry;
                    found = 1;
                    break;
                }
            }
        }
        if (!found){
            roaming_cluster = fat.sector_next[roaming_cluster];
        }
    }
    
    if(roaming_cluster == END_OF_FILE){
        return 0;
    }
    
    writer = (void*) &parent_table;
    write_clusters(writer, roaming_cluster, 1);    

    return 1;
}

DirectoryEntry get_self_entry(FAT32DriverRequest request){
    DirectoryEntry info;
    DirectoryTable table;

    uint32_t reader[CLUSTER_SIZE/4] = {0};
    FAT32FileAllocationTable fat = {0};
    read_fat(&fat);

    uint32_t currentCluster = request.parent_cluster_number;
    read_clusters((void*)reader, currentCluster, 1);
    table = as_directory(reader);

    bool found = 0;
    uint8_t i;
    while (!found && currentCluster != END_OF_FILE)
    {
        for(i = 0; i < ENTRY_COUNT; i++){
            if(memcmp(&table.entry[i].filename, &request.name, 8) == 0){
                if (request.buffer_size == 0 ||
                    (request.buffer_size != 0 && memcmp(&table.entry[i].extension, &request.ext, 3) == 0)){

                    info = table.entry[i];
                    found = 1;
                    break;
                }
            }
        }
        if (!found){
            //untuk extendable folder
            currentCluster = fat.sector_next[currentCluster];
            read_clusters((void*)reader, currentCluster, 1);
            table = as_directory(reader);
        }
    }
    if (currentCluster == END_OF_FILE && !found)
        return emptyEntry;

    return info;
}

DirectoryTable as_directory(uint32_t* reader){
    DirectoryTable table;
    memcpy(&table, reader, CLUSTER_SIZE);
    return table;
}

void read_clusters(void* target, uint16_t cluster, uint16_t sector_count){
    read_blocks(target, cluster_to_lba(cluster), sector_count);
    read_blocks(target + 512, cluster_to_lba(cluster) + 1, sector_count);
    read_blocks(target + 1024, cluster_to_lba(cluster) + 2, sector_count);
    read_blocks(target + 1536, cluster_to_lba(cluster) + 3, sector_count);
};

void write_clusters(void* entry, uint16_t cluster, uint16_t sector_count){
    write_blocks(entry, cluster_to_lba(cluster), sector_count);
    write_blocks(entry + 512, cluster_to_lba(cluster) + 1, sector_count);
    write_blocks(entry + 1024, cluster_to_lba(cluster) + 2, sector_count);
    write_blocks(entry + 1536, cluster_to_lba(cluster) + 3, sector_count);
};

FAT32FileReader read(FAT32DriverRequest request){
    FAT32FileReader retval = {0};
    if(request.parent_cluster_number < 2){
        retval.content = (void*) 2;
        return retval;
    }
    else if (!is_directory(request.parent_cluster_number)){
        retval.content = (void*) 2;
        return retval;
    }
    else if (!name_exists(request) || (
        request.name[0] == 'r' &&
        request.name[1] == 'o' &&
        request.name[2] == 'o' &&
        request.name[3] == 't' &&
        request.name[4] == ' ' &&
        request.name[5] == ' ' &&
        request.name[6] == ' ' &&
        request.name[7] == ' ')
    ){
        retval.content = (void*) 1;
        return retval;
    }

    ClusterBuffer* output;
    DirectoryEntry self = get_self_entry(request);
    
    if (self.directory == 1)
    {
        retval.content = (void*) 3;
        return retval;
    }

    else{
        retval.size = self.size;
        
        FAT32FileAllocationTable fat = {0};
        read_fat(&fat);
        
        uint32_t currentCluster = self.cluster_number;
        uint32_t cluster_count = 0;
        do
        {
            cluster_count++;
            currentCluster = fat.sector_next[currentCluster];
        } while (currentCluster != END_OF_FILE);
        output = (ClusterBuffer*) kmalloc (sizeof(ClusterBuffer) * cluster_count);        

        currentCluster = self.cluster_number;
        uint32_t counter = 0;
        do
        {
            read_clusters((void*) &output[counter], currentCluster, 1);

            if(*((char*)&output[counter]) == 0){
                //here
                counter++;
                counter--;
            }

            currentCluster = fat.sector_next[currentCluster];
            counter++;
            
        } while (currentCluster != END_OF_FILE);
        
        retval.cluster_count = counter;
        retval.content = output;

        return retval;
    }
}

FAT32DirectoryReader read_directory(FAT32DriverRequest request){
    FAT32DirectoryReader retval = {0};
    if(request.parent_cluster_number < 2){
        retval.content = (void*) 2;
        return retval;
    }
    else if (!is_directory(request.parent_cluster_number)){
        retval.content = (void*) 2;
        return retval;
    }
    else if (!name_exists(request) || (
        request.name[0] == 'r' &&
        request.name[1] == 'o' &&
        request.name[2] == 'o' &&
        request.name[3] == 't' &&
        request.name[4] == ' ' &&
        request.name[5] == ' ' &&
        request.name[6] == ' ' &&
        request.name[7] == ' ')
    ){
        retval.content = (void*) 1;
        return retval;
    }

    DirectoryTable* output;
    DirectoryEntry self = get_self_entry(request);
    
    if (self.directory != 1)
    {
        retval.content = (void*) 3;
        return retval;
    }
    
    else{
        uint32_t currentCluster = self.cluster_number;

        FAT32FileAllocationTable fat = {0};
        read_fat(&fat);

        uint32_t cluster_count = 0;
        do
        {
            cluster_count++;
            currentCluster = fat.sector_next[currentCluster];
        } while (currentCluster != END_OF_FILE);
        output = (DirectoryTable*) kmalloc (sizeof(DirectoryTable) * cluster_count);        

        currentCluster = self.cluster_number;
        uint32_t counter = 0;
        do
        {
            read_clusters((void*) &output[counter], currentCluster, 1);
            currentCluster = fat.sector_next[currentCluster];
            counter++;
            
        } while (currentCluster != END_OF_FILE);
        
        retval.cluster_count = counter;
        retval.content = output;

        return retval;
    }
}

FAT32DirectoryReader self_directory_info(uint32_t cluster_number){
    FAT32DirectoryReader retval = {0};

    DirectoryTable* output;
    
    FAT32FileAllocationTable fat = {0};
    read_fat(&fat);

    uint32_t currentCluster = cluster_number;

    uint32_t cluster_count = 0;
    do
    {
        cluster_count++;
        currentCluster = fat.sector_next[currentCluster];
    } while (currentCluster != END_OF_FILE);
    output = (DirectoryTable*) kmalloc (sizeof(DirectoryTable) * cluster_count);        

    currentCluster = cluster_number;
    uint32_t counter = 0;
    do
    {
        read_clusters((void*) &output[counter], currentCluster, 1);
        currentCluster = fat.sector_next[currentCluster];;
        counter++;
        
    } while (currentCluster != END_OF_FILE);
    
    retval.cluster_count = counter;
    retval.content = output;

    return retval;
}

void close_file(FAT32FileReader pointer){
    kfree(pointer.content);
}

void close_directory(FAT32DirectoryReader pointer){
    kfree(pointer.content);
}

uint8_t load(FAT32DriverRequest request){
    void* output = (void*) request.buf;
    DirectoryEntry self = get_self_entry(request);
    if (memcmp(&self, &emptyEntry, sizeof(DirectoryEntry)) == 0){
        return 1;
    }
    if(request.buffer_size < self.size){
        return 2;
    }
    else{
        bool reading = 1;
        uint16_t index = 0;

        FAT32FileAllocationTable fat = {0};
        read_fat(&fat);
        
        uint32_t currentCluster = self.cluster_number;
        uint32_t marker = fat.sector_next[currentCluster];
        while (reading){
            read_clusters(output + index * 2048, currentCluster, 1); 

            if(marker == END_OF_FILE){
                reading = 0;
            }
            else{
                currentCluster = marker;
                marker = fat.sector_next[currentCluster];
                index++;
            }
        }
    }

    return 0;
}


int cluster_to_lba(int clusters){
    return 4 * clusters;
}

uint32_t expand_folder(int cluster_number, FAT32FileAllocationTable* fat){
    //nyari yang kosong di fat
    uint32_t i = 0;
    for (i = ROOT_CLUSTER_NUMBER + 1; i < CLUSTER_COUNT; i++){
        if(fat->sector_next[i] == 0){
            break;
        }
    }
    
    //nyari ujung cluster
    uint32_t traverse2 = cluster_number;
    uint32_t traverse = cluster_number;
    while (traverse != END_OF_FILE){
        traverse2 = traverse;
        traverse = fat->sector_next[traverse];
    }
    //ekspansi cluster
    fat->sector_next[traverse2] = i;
    fat->sector_next[i] = END_OF_FILE;

    write_fat(fat);
    
    //inisiasi
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    read_clusters((void*)reader, cluster_number, 1);

    DirectoryTable table = {0};
    table.info = as_directory(reader).info;

    write_clusters((void*) &table, i, 1);

    return i;
}

void update_file_time(DirectoryEntry* entry){
    //cmosnya gak dipanggil di sini siapa tau perlu buat waktu lain, inget aja buat manggil cmosnya
    entry->modification_time_seconds = cmos.second;
    entry->modification_time_minutes = cmos.minute;
    entry->modification_time_hours = cmos.hour;
    entry->modification_time_day = cmos.day;
    entry->modification_time_month = cmos.month;
    entry->modifcation_time_year = cmos.year;
}

void update_file_size(DirectoryEntry* entry, uint32_t size, char category){
    if(size == 0){
        if(category == '+'){
            entry->size += 32;

        }
        else if (category == '-'){
            entry->size -= 32;
        }
    }
    else{
        if(category == '+'){
            entry->size += size;

        }
        else if(category == '-'){
            entry->size -= size;

        }
    }
}

uint8_t is_directory(uint32_t cluster){
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    read_clusters((void*)reader, cluster, 1);
    DirectoryTable table = as_directory(reader);

    if(table.info.directory != 1){
        return 0;
    }
    for(int i = 0; i < 10; i++){
        if(table.entry[i].reserved != 0){
            return 0;
        }
    }

    return 1;
}

uint8_t name_exists(FAT32DriverRequest request){
    FAT32FileAllocationTable fat = {0};
    read_fat(&fat);
    uint32_t currentCluster = request.parent_cluster_number;
    uint32_t reader[CLUSTER_SIZE/4] = {0};
    DirectoryTable table = {0};

    do{
        read_clusters((void*)reader, currentCluster, 1);
        table = as_directory(reader);
        for(int i = 0; i < ENTRY_COUNT; i++){
            if((memcmp(&table.entry[i].filename, request.name, 8) == 0) && 
                (memcmp(&table.entry[i].extension, request.ext, 3) == 0)
            ){
                return 1;
            }
        }

        currentCluster = fat.sector_next[currentCluster];

     } while (currentCluster != END_OF_FILE);

    return 0;
}

void set_entry_flag(FAT32DriverRequest request, entryflags flag){
    DirectoryEntry entry = get_self_entry(request);

    entry.read_only = flag.read_only;
    entry.hidden = flag.hidden;
    entry.system = flag.system;
    entry.volume_id = flag.volume_id;
    //Directory gabisa di set-unset for obvious reasons
    entry.archive = flag.archive;
    entry.resbit1 = flag.resbit1;
    entry.resbit2 = flag.resbit2;

    uint32_t reader[CLUSTER_SIZE/4] = {0};
    DirectoryTable table = {0};

    read_clusters((void*) reader, entry.cluster_number, 1);
    table = as_directory(reader);
    uint16_t entrynum = table.info.entry_number;
    uint16_t actualcluster = table.info.parent_actual_cluster;
    
    read_clusters((void*) reader, actualcluster, 1);
    table = as_directory(reader);
    table.entry[entrynum] = entry;
    write_clusters((void*) &table, actualcluster, 1);
}