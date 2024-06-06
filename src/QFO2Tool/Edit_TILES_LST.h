#pragma once

#include "Load_Settings.h"
#include "town_map_tiles.h"

void add_TMAP_tiles_to_lst(user_info* usr_nfo, char** new_tile_list, char* save_buff);
char* generate_new_tile_list(char* name, int tile_num);
void add_TMAP_tiles_to_lst_tt(user_info* usr_nfo, town_tile* new_tile_list, char* save_buff);
// bool check_tile_names_ll_tt(char* tiles_lst, town_tile* new_tiles, bool set_auto);