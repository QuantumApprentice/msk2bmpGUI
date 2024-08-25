#pragma once
#include "town_map_tiles.h"

//  out_pattern is array of
//  4x int struct w/pragma pack applied
#pragma pack(push, 1)
struct pattern {
    uint32_t tile_id;
    uint32_t unkown_b;
    uint32_t unkown_c;
    uint32_t unkown_d;
};
#pragma pack(pop)

void TMAP_tiles_pattern_arr(user_info* usr_info, tt_arr_handle* handle, char* file_buff);
void export_pattern_file(user_info* usr_info, tt_arr_handle* handle);
