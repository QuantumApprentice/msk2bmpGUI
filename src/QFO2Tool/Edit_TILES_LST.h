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

// void add_TMAP_tiles_to_lst(user_info* usr_nfo, char** new_tile_list, char* save_buff);
char* generate_new_tile_list(char* name, int tile_num);
void add_TMAP_tiles_to_lst_tt(user_info* usr_nfo, town_tile* new_tile_list, char* save_buff);
char* load_tiles_lst_game(char* game_path);
tile_name_arr* make_name_list_arr(char* new_tiles_list);
tile_name* make_name_list(char* tiles_list);

// bool check_tile_names_ll_tt(char* tiles_lst, town_tile* new_tiles, bool set_auto);