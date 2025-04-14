//Town-map tiles apparently need matching proto (.pro) files
//in order to be added to the mapper correctly.
//The mapper has a built-in function that can be used to add
//proto files, but configuration and setting up directory
//structure to match what the mapper needs is a PITA.

//Need to add entries to:
//data/proto/tiles/TILES.LST            //proto version
//data/Text/english/Game/pro_tile.msg   //alternates for other languages?

//https://www.nma-fallout.com/threads/using-the-bis-mapper-to-edit-create-a-new-proto.220548/#post-4471019
//https://falloutmods.fandom.com/wiki/Making_prototypes
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

// https://www.nma-fallout.com/threads/i-made-a-pro-file-editor.222442/
// https://falloutmods.fandom.com/wiki/PRO_File_Format#Tiles
// tile proto file contents:
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

#include "ImGui_Warning.h"

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

char* add_TMAP_PRO_tiles_to_LST(user_info* usr_info, tt_arr_handle* head);
bool backup_append_LST(char* path, char* string);

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
char* check_PRO_LST_names(char* tiles_lst, tt_arr_handle* new_protos)
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
    int tiles_lst_len = strlen(tiles_lst);
    for (int i = 0; i < new_protos->size; i++)
    {
        tt_arr* node = &tiles[i];
        //skip blank nodes
        if (node->tile_id == 1) {
            continue;
        }

        char* strt = tiles_lst;  //keeps track of position on TILES.LST
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
        assert(shift_ctr == (match_ctr & 7));
        //increment all the counters
        match_ctr++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
    }

    //generate new list from remaining nodes in linked_lst
    char* cropped_list = make_proto_list_arr(new_protos, matches);

    free(matches);
    return cropped_list;
}

char* input_name()
{
    static char name_buff[23] = "";
    ImGui::InputText(
        "Name\n"
        "(max 23 characters)",
        name_buff, 23);

    return name_buff;
}
char* input_desc()
{
    static char desc_buff[71] = "";
    ImGui::InputTextMultiline(
        "Description\n"
        "(max 71 characters)\n"
        "(no line-breaks)",
        desc_buff, 71);

    return desc_buff;
}

void pro_tile_msg_append_arr(user_info* usr_nfo, proto_info* info, tt_arr* tile)
{
    //append to pro_tile.msg if either a name
    //or a description has been provided

    proto_info pr_info;
    if (info == NULL) {
        info = &pr_info;
        pr_info.name        = input_name();
        pr_info.description = input_desc();
    }


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

bool create_LST_popup(char* lst_path)
{
    ImGui::Text(
        "Unable to find TILES.LST in"
        "\n\n%s\n\n"

        "Would you like to make a new one?\n"
        "This new TILES.LST will be blank\n"
        "(except for the new tiles made here),\n"
        "and will create all the subfolders\n"
        "necessary for the game engine to load\n"
        "this new TILES.LST file.\n\n"

        "--IMPORTANT--\n"
        "The Fallout game engine reads tiles in\n"
        "from TILES.LST based on the line number.\n"
        "The new TILES.LST file will override the\n"
        "game list. Only do this if you want to create\n"
        "the whole tile system from scratch,\n"
        "or to preview the results before manually merging.\n\n"

        , lst_path
    );
    if (ImGui::Button("Create new LST file?")) {
        return true;
    }

    ImGui::BeginDisabled();
    if (ImGui::Button("Extract LST from master.dat")) {
        //TODO: implement this
    }
    ImGui::SetItemTooltip("Unimplemented");
    ImGui::EndDisabled();

    if (ImGui::Button("Select different Fallout 2 folder?")) {
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
    }

    return false;
}

char* export_protos(user_info* usr_nfo, tt_arr_handle* handle)
{
    proto_info pr_info;

    if (handle == nullptr) {
    //TODO: place a warning here, this needs tile_arr*head to work
    //TODO: maybe implement this?
        return NULL;
    }
    if (usr_nfo->default_game_path[0] == '\0') {
        return NULL;
    }
    if (!usr_nfo->game_files.FRM_TILES_LST) {
        return NULL;
    }


    pr_info.name        = input_name();
    pr_info.description = input_desc();
    pr_info.material_id = get_material_id();


    //tiles can reference different line numbers in pro_tile.msg
    //have all tiles from this batch point to first new tile entry
    tt_arr* tiles = handle->tile;
    pr_info.pro_tile = tiles->tile_id * 100;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == 1) {
            continue;
        }
        export_single_tile_PRO(usr_nfo->default_game_path, node, &pr_info);
    }


    //add tile protos to data/proto/tiles/TILES.LST
    // add_TMAP_PRO_tiles_to_LST(usr_nfo, handle);

    //TODO: need to add option for different languages?
    //TODO: maybe need to create the subfolders
    //TODO: also need to give options for descriptions already on pro_tile.msg
    //add name/description to data/Text/english/Game/pro_tile.msg
    // pro_tile_msg_append_arr(usr_nfo, &pr_info, tiles);
}

void export_proto_arr_POPUP(user_info* usr_nfo, tt_arr_handle* handle)
{
    static char* lst_path = NULL;
    // static char LST_path[MAX_PATH];
    bool export_proto = false;
    //input name
    ImGui::Text(
        "In order to get new tiles to appear in the mapper\n"
        "(and thus in the game), each tile must have a proto(.pro)\n"
        "file made, and an entry for each tile appended to\n\n"
        "   Fallout 2/data/art/tiles/TILES.LST\n"
        "   Fallout 2/data/proto/tiles/TILES.LST\n\n"
        "In addition, entries can optionally be made in\n\n"
        "   Fallout 2/data/Text/english/Game/pro_tile.msg\n\n"
        "to give the tile a name and description in the\n"
        "Fallout 2 mapper (Mapper2.exe).\n\n"
        "For this to work, please provide the path to\n"
        "fallout2.exe in your modded Fallout 2 folder,\n"
        "and have these files extracted to their\n"
        "appropriate locations.\n"
        "(I plan on adding a feature to extract these)\n"
        "(automatically, but currently can't do this.)\n"
        );
    static char FObuf[MAX_PATH] = "";
    if (FObuf[0] == '\0' && usr_nfo->default_game_path[0] != '\0') {
        strncpy(FObuf, usr_nfo->default_game_path, MAX_PATH);
    }
    ImGui::InputText("###fallout2.exe", FObuf, MAX_PATH);
    ImGui::Text(
        "\nThese are Optional,\n"
        "and will be applied to all tiles in this set.\n"
    );

    get_material_id();
    input_name();
    input_desc();

    if (ImGui::Button("Add to Fallout 2")) {
        export_proto = true;
    }
    if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
    }


    //Begin Popups/////////////////////////////////
    bool game_path      = false;
    bool load_LST       = false;
    bool make_LST       = false;
    bool append_LST     = false;
    bool append_PRO_LST = false;
    bool append_PRO_MSG = false;
    if (ImGui::BeginPopupModal("Create LST File")) {
        make_LST = create_LST_popup(lst_path);
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Append to LST")) {
        ImGui::Text(
            "%s\n\n"
            "Append new tiles to TILES.LST?\n"
            "(A backup will be made.)\n\n"
            "--IMPORTANT--\n"
            "The Fallout game engine reads tiles in\n"
            "from TILES.LST based on the line number.\n"
            "Be careful not to change the order of\n"
            "tiles once they are on the list.\n\n"
            , lst_path
        );
        if (ImGui::Button("Append to TILES.LST")) {
            append_LST = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Create new TILES.LST")) {
            make_LST = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("TILES.LST Unmodified")) {
        ImGui::Text(
            "%s\n\n"
            "TILES.LST not updated...\n"
            "All new tile-names were already\n"
            "found on TILES.LST.\n"
            "No new tile-names were added.\n"
            , lst_path
        );
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    //End Popups/////////////////////////////////

    // 1) export tile FRM files (already done outside this function)
    // 2) append tile names to /art/tiles/TILES.LST (or create new one)
    //  a) load /art/ TILES.LST
    //  b) if no /art/ TILES.LST then ==> select new folder || create new LST || cancel
    //  c) check if names already on list
    //  d) if not then append
    // 3) export tile PRO files using info from /art/ TILES.LST
    //  a) load /art/ TILES.LST
    //  b) if no /art/ TILES.LST then ==> select new folder || create new LST || cancel
    //  c) check if names already on list (and get line #)
    //  d) if not then append             (and get line #)
    //  e) finally, export protos with line info from /art/ TILES.LST
    // 4) append tile proto info to /pro/tiles/TILES.LST (or create new one)
    //  a) load /proto/ TILES.LST
    //  b) if no /proto/ TILES.LST then ==> select new folder || create new LST || cancel
    //  c) can only append?

    if (game_path) {
        //copy any game_path changes to user_info for saving to config
        if (fallout2exe_exists(FObuf) == false) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] export_proto_arr_POPUP()\n\n"
                "Fallout executable not found.\n"
            );
            printf("Error: export_proto_arr_POPUP() Fallout executable not found:\n %s: %d\n", FObuf, __LINE__);
            return;
        }
        strncpy(usr_nfo->default_game_path, FObuf, MAX_PATH);
    }


    if (load_LST) {
        lst_path = load_FRM_tiles_LST(usr_nfo, handle);
    }
    if (make_LST) {
        usr_nfo->game_files.FRM_TILES_LST = save_NEW_tiles_LST(handle, lst_path);
    }
    if (append_LST) {
        lst_path = add_TMAP_tiles_to_lst_arr(usr_nfo, handle, lst_path);
    }


    if (export_proto) {
        lst_path = export_protos(usr_nfo, handle);
    }
    if (append_PRO_LST) {
        //add tile protos to data/proto/tiles/TILES.LST
        // lst_path = add_TMAP_PRO_tiles_to_LST(usr_nfo, handle);
        add_TMAP_PRO_tiles_to_LST(usr_nfo, handle);
    }
    if (append_PRO_MSG) {
        //TODO: need to add option for different languages?
        //TODO: maybe need to create the subfolders
        //TODO: also need to give options for descriptions already on pro_tile.msg
        //add name/description to data/Text/english/Game/pro_tile.msg
        pro_tile_msg_append_arr(usr_nfo, NULL, handle->tile);
    }
}

char* append_PRO_tiles_LST(char* old_PRO_LST, tt_arr_handle* head, bool set_auto)
{

    //append new protos to list in memory
    char* new_PRO_LST = check_PRO_LST_names(old_PRO_LST, head);
    if (new_PRO_LST == nullptr) {
        return old_PRO_LST;
    }


    //TODO: use an io_ function to append text instead
    //append new list_of_tiles to the end of original list
    //in a new buffer large enough to fit both
    int old_LST_size    = strlen(old_PRO_LST);
    int new_LST_size    = strlen(new_PRO_LST);
    char* final_PRO_LST = (char*)malloc(old_LST_size + new_LST_size +1);   //+1 for null char
    strncpy(final_PRO_LST, old_PRO_LST, old_LST_size);
    final_PRO_LST[old_LST_size] = '\0';    //needed for strncat() to work
    strncat(final_PRO_LST, new_PRO_LST, new_LST_size);

    free(old_PRO_LST);
    return final_PRO_LST;
}

//##### I feel like this is a stupid way to write this
//##### but it's better than it was,
//##### and I don't know a better way yet
//this assumes usr_info->default_game_path has been set
//append to data/proto/tiles/TILES.LST
char* add_TMAP_PRO_tiles_to_LST(user_info* usr_nfo, tt_arr_handle* head)
{
    static char LST_path[MAX_PATH];
    snprintf(LST_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", usr_nfo->default_game_path);
    char* old_PRO_LST = io_load_txt_file(LST_path);

    if (old_PRO_LST == nullptr) {
        //TODO: may want to handle other failures
        //      which would cause io_load_text_file()
        //      to return NULL/nullptr

        ImGui::OpenPopup("Create LST File");
        printf("Unable to load /proto/tiles/TILES.LST...\nCreating new one...\n");
        return LST_path;
    }

    char* new_PRO_LST = append_PRO_tiles_LST(old_PRO_LST, head, false);
    if (new_PRO_LST != usr_nfo->game_files.PRO_TILES_LST) {
        free(usr_nfo->game_files.PRO_TILES_LST);
    }

    //backup and save new list
    io_backup_file(LST_path, nullptr);
    io_save_txt_file(LST_path, new_PRO_LST);

    usr_nfo->game_files.PRO_TILES_LST = new_PRO_LST;

    return LST_path;
}

//backs up file at "path",
//appends "names" to text file at "path"
bool backup_append_LST(char* path, char* LST_file)
{
    if (io_file_exists(path) == false) {
        return false;
    }
    if (LST_file == nullptr) {
        return false;
    }

    int file_size        = io_file_size(path);
    int string_len       = strlen(LST_file);
    char* buff_tiles_lst = (char*)malloc(file_size + string_len);
    FILE* tiles_lst      = fopen(path, "rb");
    if (tiles_lst == nullptr) {
        //TODO: popup warning?
        //unable to open file for some reason
        return false;
    }

    fread(buff_tiles_lst, file_size, 1, tiles_lst);
    fclose(tiles_lst);

    char* ptr = buff_tiles_lst + file_size;
    memcpy(ptr, LST_file, string_len);

    io_backup_file(path, nullptr);
    tiles_lst = fopen(path, "wb");
    fwrite(buff_tiles_lst, file_size+string_len, 1, tiles_lst);
    fclose(tiles_lst);

    free(buff_tiles_lst);
    return true;
}

void export_single_tile_PRO(char* game_path, tt_arr* tile, proto_info* info)
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

    B_Endian::flip_proto_endian(&proto);

    // B_Endian::swap_32(proto.ObjectID);
    // B_Endian::swap_32(proto.TextID);
    // B_Endian::swap_32(proto.FrmID);
    // B_Endian::swap_32(proto.MaterialID);

    //TODO: create folder paths if they don't exist
    snprintf(path_buff, MAX_PATH, "%s/data/proto/tiles/%08d.pro", game_path, tile->tile_id);

    char* ptr = strrchr(path_buff, PLATFORM_SLASH);
    char back = ptr[0];
    ptr[0] = '\0';
    if (!io_isdir(path_buff)) {
        io_make_dir(path_buff);
    }
    ptr[0] = back;

    FILE* tile_pro = fopen(path_buff, "wb");
    if (tile_pro == nullptr) {
        printf("Unable to open proto file: %s\n", path_buff);
    }
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
    static int material_id = 0;
    const char* names[] = { "Glass", "Metal", "Plastic", "Wood", "Dirt", "Stone", "Cement", "Leather" };
    ImGui::Combo("Material Type", &material_id, names, IM_ARRAYSIZE(names));

    return material_id;
}