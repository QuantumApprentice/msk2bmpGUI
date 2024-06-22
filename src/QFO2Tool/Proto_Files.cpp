//Town-map tiles apparently need matching proto (.pro) files
//in order to be added to the mapper correctly.
//The mapper has a built-in function that can be used to add
//proto files, but configuration and setting up directory
//structure to match what the mapper needs is a PITA.

//Need to add entries to:
//data/proto/tiles/TILES.LST            //proto version
//data/Text/english/Game/pro_tile.msg   //alternates for other languages?


//Mapper produces interim text files to produce proto files:
/*      //example//
pid: 67111980 00003116      //proto ID number?  //line number in TILES.LST art file //check hex trick used to produce proto id from line number
name: abc                   //name (not required?)
message_num: 311600         //key/value pair in pro_tile.msg (Fallout 2/data/text/english/game/pro/proto_tile.msg)
fid: 67111979 test013       //fid == art id? has name of file
flags: 0                    //unkown?
flags_ext: 0                //unkown?
material: Glass             //material type == enum
*/

//tile proto files have: https://falloutmods.fandom.com/wiki/PRO_File_Format#Tiles
/*
0x0000  4bytes  ObjectType & ObjectID
0x0004  4bytes  TextID
0x0008  4bytes  FrmType & FrmID
0x000C  4bytes  Light radius
0x0010  4bytes  Light intensity
0x0014  4bytes  Flags
0x0018  4bytes  MaterialID
*/

#include <stdint.h>
#include <stdio.h>
#include <tinyfiledialogs.h>

#include "platform_io.h"
#include "Proto_Files.h"
#include "Load_Settings.h"
#include "B_Endian.h"
#include "Edit_TILES_LST.h"
#include "tiles_pattern.h"

enum material {
    Glass   = 0,
    Metal   = 1,
    Plastic = 2,
    Wood    = 3,
    Dirt    = 4,
    Stone   = 5,
    Cement  = 6,
    Leather = 7
};

void proto_tiles_lst_append(user_info* usr_info, town_tile* head);
bool backup_append_LST(char* path, char* string);

void pro_tile_msg_append(user_info* usr_nfo, proto_info* info, town_tile* tile)
{
    //append to pro_tile.msg if either a name
    //or a description has been provided
    if (strlen(info->name) > 1 || strlen(info->description) > 1) {

        //append to pro_tile.msg
        char msg_line[512+32];
        snprintf(msg_line, 512+32,
                "{%d}{}{%s}\r\n{%d}{}{%s}\r\n",
                tile->tile_id*100,   info->name,
                tile->tile_id*100+1, info->description);

        if (*usr_nfo->default_game_path == '\0') {
            return;
        }
        char path_buff[MAX_PATH];
        snprintf(path_buff, MAX_PATH, "%s/data/Text/english/Game/pro_tile.msg", usr_nfo->default_game_path);
        backup_append_LST(path_buff, msg_line);
    }
}

void export_tile_proto_start(user_info* usr_nfo, town_tile* head)
{
    proto_info info;
    //input name
    ImGui::Text(
        "In order to get new tiles to appear in the mapper\n"
        "(and thus in the game), each tile must have a proto(.pro)\n"
        "file made, and an entry for each tile appended to\n\n"
        "   Fallout 2/data/proto/tiles/TILES.LST\n"
        "   Fallout 2/data/art/tiles/TILES.LST.\n\n"
        "In addition, entries? can optionally be made in\n\n"
        "   Fallout 2/data/Text/english/Game/pro_tile.msg\n\n"
        "to give the tile a name and description in the\n"
        "Fallout 2 mapper (Mapper2.exe).\n\n"
        "Please provide the path to fallout2.exe in your\n"
        "modded Fallout 2 folder.\n"
        );
    static char FObuf[MAX_PATH] = "";
    strncpy(FObuf, usr_nfo->default_game_path, MAX_PATH);
    ImGui::InputText("###fallout2.exe", FObuf, MAX_PATH);
    ImGui::Text(
        "These are Optional,\n"
        "and will be applied to all tiles in this set.\n"
    );
    static char buf1[32]  = ""; ImGui::InputText("Name",                 buf1, 32);
    static char buf2[512] = ""; ImGui::InputTextMultiline("Description", buf2, 512);

    if (ImGui::Button("Add to Fallout 2...")) {
        if (head == nullptr) {
        //TODO: place a warning here, this needs town_tile*head to work
            return;
        }

        //copy any game_path changes to user_info for saving to config
        strncpy(usr_nfo->default_game_path, FObuf, MAX_PATH);
        info.name        = buf1;
        info.description = buf2;
        info.material_id = get_material_id();

        add_TMAP_tiles_to_lst_tt(usr_nfo, head, nullptr);
        TMAP_tiles_make_row(usr_nfo, head);

        //tiles can reference different line numbers in pro_tile.msg
        //have all subsequent tiles point to first new tile entry
        info.pro_tile = head->tile_id * 100;
        town_tile* node = head;
        while (node != nullptr)
        {
            export_tile_proto(usr_nfo, node, &info);
            node = node->next;
        }

        if (usr_nfo->default_game_path[0] == '\0') {
            return;
        }

        //add tile protos to data/proto/tiles/TILES.LST
        proto_tiles_lst_append(usr_nfo, head);

        //TODO: need to add option for different languages
        //TODO: maybe need to create the subfolders
        //TODO: also need to give options for items already on the list
        //add name/description to data/Text/english/Game/pro_tile.msg
        pro_tile_msg_append(usr_nfo, &info, head);
    }

    if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
    }
}

void proto_tiles_lst_append(user_info* usr_info, town_tile* head)
{
    //TODO: refactor append_tiles_lst() to work here
    int size = 0;
    town_tile* node = head;
    while (node != nullptr)
    {
        size += 14;
        node = node->next;
    }

    char* new_proto_lst = (char*)malloc(size);
    char* ptr = new_proto_lst;
    int index = 0;
    node      = head;
    while (node != nullptr)
    {
        snprintf(ptr, 15, "%08d.pro\r\n", node->tile_id);
        ptr += 14;
        node = node->next;
    }

    //this assumes usr_info->default_game_path has been set
    //append to data/proto/tiles/TILES.LST
    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", usr_info->default_game_path);

    //TODO: this needs to check if ####.pro is already on the list
    backup_append_LST(save_path, new_proto_lst);
    free(new_proto_lst);
}

//backs up file at "path",
//appends "names" to text file at "path"
bool backup_append_LST(char* path, char* string)
{
    if (io_file_exists(path) == false) {
        return false;
    }

    int file_size = io_file_size(path);
    int string_len = strlen(string);
    char* buff_tiles_lst = (char*)malloc(file_size + string_len);
    FILE* tiles_lst = fopen(path, "rb");
    if (tiles_lst == nullptr) {
        //TODO: popup warning?
        //unable to open file for some reason
        return false;
    }

    fread(buff_tiles_lst, file_size, 1, tiles_lst);
    fclose(tiles_lst);

    char* ptr = buff_tiles_lst + file_size;
    memcpy(ptr, string, string_len);

    io_backup_file(path);
    tiles_lst = fopen(path, "wb");
    fwrite(buff_tiles_lst, file_size+string_len, 1, tiles_lst);
    fclose(tiles_lst);

    free(buff_tiles_lst);
    return true;
}

//export individual tile proto to save_path
void export_tile_proto(user_info* usr_info, town_tile* tile, proto_info* info)
{
    if (tile == nullptr) {
        //TODO: place a warning here, this needs town_tile*head to work
        return;
    }

    char path_buff[MAX_PATH];

    tile_proto proto;
    proto.ObjectID        = tile->tile_id | 0x4000000;
    proto.TextID          = info->pro_tile;                //used as a key/value pair in pro_tile.msg
    //FrmID is the line number (starting from 0) in art/tiles/TILES.LST
    proto.FrmID           = (tile->tile_id -1) | 0x4000000; // -1 for off by 1 error
    //TODO: test if these 3 have effect on tiles
    proto.Light_Radius    = 0;
    proto.Light_Intensity = 0;
    proto.Flags           = 0xFFFFFFFF;     //this is what the mapper uses on tiles, not sure why yet
    //end TODO
    proto.MaterialID      = info->material_id;

    B_Endian::swap_32(proto.ObjectID);
    B_Endian::swap_32(proto.TextID);
    B_Endian::swap_32(proto.FrmID);
    B_Endian::swap_32(proto.MaterialID);

    snprintf(path_buff, MAX_PATH, "%s/data/proto/tiles/%08d.pro", usr_info->default_game_path, tile->tile_id);
    FILE* tile_pro = fopen(path_buff, "wb");
    fwrite(&proto, sizeof(tile_proto), 1, tile_pro);
    fclose(tile_pro);

}

//dropdown menu picking type of material
//to set the proto as
//     0: Glass
//     1: Metal
//     2: Plastic
//     3: Wood
//     4: Dirt
//     5: Stone
//     6: Cement
//     7: Leather
int get_material_id()
{
    //input material type
    static int material_id = 0;
    const char* names[] = { "Glass", "Metal", "Plastic", "Wood", "Dirt", "Stone", "Cement", "Leather" };
    // Simple selection popup (if you want to show the current selection inside the Button itself,
    // you may want to build a string using the "###" operator to preserve a constant ID with a variable label)

    if (ImGui::Button("Select Material Type...")) {
        ImGui::OpenPopup("material_select");
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(material_id == 0 ? "Glass" : names[material_id]);
    if (ImGui::BeginPopup("material_select"))
    {
        ImGui::SeparatorText("Material");
        for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
            if (ImGui::Selectable(names[i])) {
                material_id = i;
            }
        }

        ImGui::EndPopup();
    }
    return material_id;
}