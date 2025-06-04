#pragma once
#include <stdint.h>
#include "platform_io.h"

struct DAT_file {
    char     name[MAX_PATH];
    uint8_t* data;
    int32_t  size;
};

struct DIR_entry {
    char*   path_ptr;
    int32_t path_size;
    char    path_pack;
    uint8_t type;
    int32_t unpack_size;
    int32_t packed_size;
    int32_t offset;
};

//generic buffer struct?
struct Buffer {
    int      file_size;
    uint8_t* file_data;
};