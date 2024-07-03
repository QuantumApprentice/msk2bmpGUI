#pragma once

#include "Load_Settings.h"
#include "town_map_tiles.h"

struct tile_name {
    char* name_ptr;
    int length;
    tile_name* next;
};

struct tile_name_arr {
    char*    name_ptr;
    uint32_t length;
    uint32_t next;           //points to array index of next viable name
};

void add_TMAP_tiles_to_lst_arr(user_info* usr_nfo, tt_arr_handle* handle,    char* save_buff);
char* load_tiles_lst_game(char* game_path);
tile_name_arr* make_name_list_arr(char* new_tiles_list);
