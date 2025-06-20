#pragma once
#include <stdint.h>
#include "platform_io.h"
#include "Load_Settings.h"
#include "town_map_tiles.h"

struct DAT_file {
    char     name[MAX_PATH];
    uint8_t* data;
    int32_t  size;
};

struct DIR_entry {
    char*   path_ptr;
    int32_t path_size;
    uint8_t packed;
    int32_t unpack_size;
    int32_t packed_size;
    int32_t offset;
    uint8_t* file_ptr;
};

//generic buffer struct?
struct Buffer {
    int32_t  file_size;
    uint8_t* file_data;
};

//TODO: clean this up
bool tt_file_DAT_extract(user_info* usr_nfo, STATE_export* state);
// DAT_file load_dat_file(char* file_name, char* game_path);
// bool extract_from_DAT(char* file_name, char* dat_name, user_info* usr_nfo, DAT_file* dat_file, Buffer* buff);