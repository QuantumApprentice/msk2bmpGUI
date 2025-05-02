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

void export_TMAP_tiles_pattern(user_info* usr_info, tt_arr_handle* handle, char* file_buff);
void export_PAT_file_POPUP(user_info* usr_info, tt_arr_handle* handle, export_state* state, bool auto_export);
void assign_tile_id(tt_arr_handle* handle, const char* tiles_lst);