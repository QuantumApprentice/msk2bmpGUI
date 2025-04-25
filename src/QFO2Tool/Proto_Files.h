#pragma once
#include "town_map_tiles.h"
#include "Edit_TILES_LST.h"

struct proto_info {
    char* name;
    char* description;
    int material_id;
    int pro_tile;
};

struct tile_proto {
    uint32_t ObjectID;
    uint32_t TextID;
    uint32_t FrmID;
    uint32_t Light_Radius;
    uint32_t Light_Intensity;
    uint32_t Flags;
    uint32_t MaterialID;
};


int get_material_id();
void export_PRO_tiles_POPUP(user_info* usr_nfo, tt_arr_handle* handle, export_state* state, bool auto_export);
bool export_single_tile_PRO(char* game_path, tt_arr* tile, proto_info* info);
