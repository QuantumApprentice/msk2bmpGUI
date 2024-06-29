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
void proto_tiles_lst_append_arr(user_info* usr_info, tt_arr_handle* head);
bool backup_append_LST(char* path, char* string);

//TODO: refactor append_tiles_lst() to work here
//_tt stands for town_tile*
char* make_proto_list_tt(town_tile* head, uint8_t* match_buff)
{

    //get total_size of tile_ids where no match was found
    //used for allocating buffer
    uint8_t shift_ctr  = 0;
    int     tile_num   = 0;
    int     total_size = 0;
    town_tile* node = head;
    while (node != nullptr) {
        if (!(match_buff[tile_num/8]) & (1 << shift_ctr)) {
            total_size += 14;
        }
            shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
        tile_num++;
        node = node->next;
    }

    //if there are no nodes (or none with viable names)
    if (total_size < 1) {
        tinyfd_notifyPopup("TILES.LST not updated...",
                        "All new proto ids were already\n"
                        "found on /data/proto/tiles/TILES.LST.\n"
                        "No new proto ids were added.\n",
                        "info");
        return nullptr;
    }

    //add non-matches to list of ids
    char* cropped_list = (char*)malloc(total_size+1);
    char* c    = cropped_list;
    shift_ctr  = 0;
    tile_num   = 0;
    node       = head;
    while (node != nullptr)
    {
        if (!(match_buff[tile_num/8]) & (1 << shift_ctr)) {
            snprintf(c, 15, "%08d.pro\r\n", node->tile_id);
            c += 14;
        }
        //increment all the counters
        node = node->next;
        tile_num++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
    }
    c[0] = '\0';

    return cropped_list;
}

//TODO: refactor append_tiles_lst() to work here
//arr stands for tt_arr*
char* make_proto_list_arr(tt_arr_handle* head, uint8_t* match_buff)
{

    //get total_size of tile_ids where no match was found
    //used for allocating buffer
    int     match_ctr = 0;
    uint8_t shift_ctr  = 0;
    int     total_size = 0;
    tt_arr* tiles = head->tile;
    for (int i = 0; i < head->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == 1) {
            continue;
        }

        int match = match_ctr/8;
        int shift = 1 << shift_ctr;

        // if (!(match_buff[match_ctr/8]) & (1 << shift_ctr)) {
        if (!(match_buff[match] & shift)) {
            total_size += strlen(node->name_ptr)+2;    //+2 for /r/n
        }
        match_ctr++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
    }

    //if there are no nodes (or none with viable names)
    if (total_size < 1) {
        tinyfd_notifyPopup("TILES.LST not updated...",
                        "All new proto ids were already\n"
                        "found on /data/proto/tiles/TILES.LST.\n"
                        "No new proto ids were added.\n",
                        "info");
        return nullptr;
    }

    //add non-matches to list of ids
    char* cropped_list = (char*)malloc(total_size+1);
    char* c   = cropped_list;
    shift_ctr = 0;
    match_ctr = 0;
    for (int i = 0; i < head->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == 1) {
            continue;
        }

        if (!(match_buff[match_ctr/8] & 1 << shift_ctr)) {
            snprintf(c, 15, "%08d.pro\r\n", node->tile_id);
            c += strlen(c);
        }
        //increment all the counters
        match_ctr++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
    }
    c[0] = '\0';

    return cropped_list;
}

//compare names on tiles_lst to names on new_tiles
//but convert new_tiles to town_tile* linked list first
char* check_proto_names_arr(char* tiles_lst, tt_arr_handle* new_protos)
{
    int num_tiles = 0;
    tt_arr* tiles = new_protos->tile;
    for (int i = 0; i < new_protos->size; i++) {
        tt_arr* node = &tiles[i];
        if (node->tile_id == 1) {
            continue;
        }
        num_tiles++;
    }
    uint8_t* matches = (uint8_t*)calloc(1+num_tiles/8, 1);

    //if tiles_lst doesn't exist
    //create new list from new_protos and return it
    if (tiles_lst == nullptr) {
        char* cropped_list = make_proto_list_arr(new_protos, matches);
        free(matches);
        return cropped_list;
    }

    //identify duplicate entries in the proto/TILES.LST file
    // and mark them as duplicates in (matches)
    int match_ctr = 0;
    uint8_t shift_ctr = 0;
    char* strt = tiles_lst;  //keeps track of position on TILES.LST
    int tiles_lst_len = strlen(tiles_lst);
    for (int i = 0; i < new_protos->size; i++)
    {
        tt_arr* node = &tiles[i];
        //skip blank nodes
        if (node->tile_id == 1) {
            continue;
        }

        for (int char_ctr = 0; char_ctr < tiles_lst_len; char_ctr++)
        {
            if (tiles_lst[char_ctr] != '\n' && tiles_lst[char_ctr] != '\0') {
                continue;
            }
            //check if strt == node.tile_id
            int num = atoi(strt);
            if (num != node->tile_id) {
                strt = &tiles_lst[char_ctr+1];
                continue;
            }
            //identify this node as having a duplicate match
            matches[match_ctr/8] |= 1 << shift_ctr;
            break;
        }
        assert(shift_ctr == match_ctr &7);
        //increment all the counters
        match_ctr++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
        strt = tiles_lst;
    }

    //generate new list from remaining nodes in linked_lst
    char* cropped_list = make_proto_list_arr(new_protos, matches);

    free(matches);
    return cropped_list;
}

//compare names on tiles_lst to names on new_tiles
//but convert new_tiles to town_tile* linked list first
char* check_proto_names_ll_tt(char* tiles_lst, town_tile* new_protos)
{
    int num_tiles = 0;
    int tiles_lst_len = strlen(tiles_lst);
    town_tile* node = new_protos;
    while (node != nullptr)
    {
        num_tiles++;
        node = node->next;
    }

    //TODO: can I make this more precise?
    uint8_t* matches = (uint8_t*)calloc(1+num_tiles/8, 1);
    uint8_t shift_ctr = 0;
    int     node_ctr  = 0;

    char* strt = tiles_lst;  //keeps track of position on TILES.LST
    node = new_protos;       //reset node for next step
    while (node != nullptr) {
        for (int char_ctr = 0; char_ctr < tiles_lst_len; char_ctr++)
        {
            if (tiles_lst[char_ctr] != '\n' && tiles_lst[char_ctr] != '\0') {
                continue;
            }
            //check first char of strt == first char of node.name_ptr
            if (atoi(strt) != node->tile_id) {
                strt = &tiles_lst[char_ctr+1];
                continue;
            }
            //identify this node as having a duplicate match
            matches[node_ctr/8] |= 1 << shift_ctr;
            //increment all the counters
            shift_ctr++;
            if (shift_ctr >= 8) {
                shift_ctr = 0;
            }
            node_ctr++;
            break;
        }
        strt = tiles_lst;
        if (node == nullptr) {
            break;
        }
        node = node->next;
    }

    //generate new list from remaining nodes in linked_lst
    char* cropped_list = make_proto_list_tt(new_protos, matches);

    return cropped_list;
}



void pro_tile_msg_append_arr(user_info* usr_nfo, proto_info* info, tt_arr* tile)
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
        "In addition, entries can optionally be made in\n\n"
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
    static char buf1[23] = ""; ImGui::InputText(
        "Name\n(max 23 characters)",                          buf1, 23);
    static char buf2[71] = ""; ImGui::InputTextMultiline(
        "Description\n(max 71 characters)\n(no line-breaks)", buf2, 71);

    if (ImGui::Button("Add to Fallout 2...")) {
        if (head == nullptr) {
        //TODO: place a warning here, this needs town_tile*head to work
            return;
        }

        //copy any game_path changes to user_info for saving to config
        char game_path[MAX_PATH];
        snprintf(game_path, MAX_PATH, "%s/fallout2.exe", FObuf);
        if (io_file_exists(game_path)) {
            strncpy(usr_nfo->default_game_path, FObuf, MAX_PATH);
        } else {
            //TODO: popup warning - can't find fallout2.exe
        }
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

void export_tile_proto_arr_start(user_info* usr_nfo, tt_arr_handle* handle)
{
    proto_info info;
    //input name
    ImGui::Text(
        "In order to get new tiles to appear in the mapper\n"
        "(and thus in the game), each tile must have a proto(.pro)\n"
        "file made, and an entry for each tile appended to\n\n"
        "   Fallout 2/data/proto/tiles/TILES.LST\n"
        "   Fallout 2/data/art/tiles/TILES.LST.\n\n"
        "In addition, entries can optionally be made in\n\n"
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
    static char buf1[23] = ""; ImGui::InputText(
        "Name\n(max 23 characters)",                          buf1, 23);
    static char buf2[71] = ""; ImGui::InputTextMultiline(
        "Description\n(max 71 characters)\n(no line-breaks)", buf2, 71);

    if (ImGui::Button("Add to Fallout 2...")) {
        if (handle == nullptr) {
        //TODO: place a warning here, this needs tile_arr*head to work
            return;
        }

        //copy any game_path changes to user_info for saving to config
        char game_path[MAX_PATH];
        snprintf(game_path, MAX_PATH, "%s/fallout2.exe", FObuf);
        if (io_file_exists(game_path)) {
            strncpy(usr_nfo->default_game_path, FObuf, MAX_PATH);
        } else {
            //TODO: popup warning - can't find fallout2.exe
        }
        info.name        = buf1;
        info.description = buf2;
        info.material_id = get_material_id();

        tt_arr* tiles = handle->tile;
        add_TMAP_tiles_to_lst_arr(usr_nfo, handle, nullptr);
        TMAP_tiles_pattern_arr(usr_nfo, handle);

        //tiles can reference different line numbers in pro_tile.msg
        //have all subsequent tiles point to first new tile entry
        info.pro_tile = tiles->tile_id * 100;
        for (int i = 0; i < handle->size; i++)
        {
            tt_arr* node = &tiles[i];
            if (node->tile_id == 1) {
                continue;
            }
            export_tile_proto_arr(usr_nfo, node, &info);
        }

        if (usr_nfo->default_game_path[0] == '\0') {
            return;
        }

        //add tile protos to data/proto/tiles/TILES.LST
        proto_tiles_lst_append_arr(usr_nfo, handle);

        //TODO: need to add option for different languages
        //TODO: maybe need to create the subfolders
        //TODO: also need to give options for descriptions already on pro_tile.msg
        //add name/description to data/Text/english/Game/pro_tile.msg
        pro_tile_msg_append_arr(usr_nfo, &info, tiles);
    }

    if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
    }
}

void proto_tiles_lst_append_arr(user_info* usr_info, tt_arr_handle* head)
{
    //this assumes usr_info->default_game_path has been set
    //append to data/proto/tiles/TILES.LST
    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", usr_info->default_game_path);
    //check if new protos are on old list
    char* old_proto_list = io_load_text_file(save_path);
    if (old_proto_list == nullptr) {
        //TODO: need either a popup or some menu
        //      telling the user they're missing this file
        //      and asking if we should create a new one
        //      or load the original from master.dat file
        printf("Unable to load /proto/tiles/TILES.LST...\nCreating new one...\n");
    }
    char* new_proto_list = check_proto_names_arr(old_proto_list, head);
    if (new_proto_list == nullptr) {
        free(old_proto_list);
        return;
    }
    //backup and save new list
    backup_append_LST(save_path, new_proto_list);
    free(old_proto_list);
    free(new_proto_list);
}

void proto_tiles_lst_append(user_info* usr_info, town_tile* head)
{
    //this assumes usr_info->default_game_path has been set
    //append to data/proto/tiles/TILES.LST
    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", usr_info->default_game_path);
    //check if new protos are on old list
    char* old_proto_list = io_load_text_file(save_path);
    char* new_proto_list = check_proto_names_ll_tt(old_proto_list, head);
    if (new_proto_list == nullptr) {
        free(old_proto_list);
        return;
    }
    //backup and save new list
    backup_append_LST(save_path, new_proto_list);
    free(old_proto_list);
    free(new_proto_list);
}

//backs up file at "path",
//appends "names" to text file at "path"
bool backup_append_LST(char* path, char* string)
{
    if (io_file_exists(path) == false) {
        return false;
    }
    if (string == nullptr) {
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

    io_backup_file(path, nullptr);
    tiles_lst = fopen(path, "wb");
    fwrite(buff_tiles_lst, file_size+string_len, 1, tiles_lst);
    fclose(tiles_lst);

    free(buff_tiles_lst);
    return true;
}

void export_tile_proto_arr(user_info* usr_info, tt_arr* tile, proto_info* info)
{
    if (tile == nullptr) {
        //TODO: place a warning here, this needs town_tile*head to work
        return;
    }

    char path_buff[MAX_PATH];

    tile_proto proto;
    //protoIDs are 1 indexed?
    proto.ObjectID        = tile->tile_id+1 | 0x4000000;
    //used as a key/value pair in pro_tile.msg
    proto.TextID          = info->pro_tile;
    //FrmID is the line number (starting from 0) in art/tiles/TILES.LST
    proto.FrmID           = (tile->tile_id) | 0x4000000;
    //TODO: test if these 3 have effect on tiles
    proto.Light_Radius    = 8;
    proto.Light_Intensity = 8;
    proto.Flags           = 0xFFFFFFFF;     //this is what the mapper uses on tiles, not sure why yet
    //end TODO
    proto.MaterialID      = info->material_id;

    B_Endian::swap_32(proto.ObjectID);
    B_Endian::swap_32(proto.TextID);
    B_Endian::swap_32(proto.FrmID);
    B_Endian::swap_32(proto.MaterialID);

    snprintf(path_buff, MAX_PATH, "%s/data/proto/tiles/%08d.pro", usr_info->default_game_path, tile->tile_id);
    FILE* tile_pro = fopen(path_buff, "wb");
    if (tile_pro == nullptr) {
        printf("Unable to open proto file: %s\n", path_buff);
    }
    fwrite(&proto, sizeof(tile_proto), 1, tile_pro);
    fclose(tile_pro);
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
    proto.Light_Radius    = 8;
    proto.Light_Intensity = 8;
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