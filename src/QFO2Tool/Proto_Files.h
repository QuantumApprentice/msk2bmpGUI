#include "town_map_tiles.h"


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
void export_tile_proto_start(user_info* usr_nfo, town_tile* head);
void export_tile_proto(user_info* save_path, town_tile* tile, proto_info* info);