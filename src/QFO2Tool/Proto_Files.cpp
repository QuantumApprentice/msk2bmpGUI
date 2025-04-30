//Town-map tiles apparently need matching proto (.pro) files
//in order to be added to the mapper correctly.
//The mapper has a built-in function that can be used to add
//proto files, but configuration and setting up directory
//structure to match what the mapper needs is a PITA.

//Need to add entries to:
//data/proto/tiles/TILES.LST            //proto version
//data/text/english/game/pro_tile.msg   //alternates for other languages?

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

#include "platform_io.h"
#include "Proto_Files.h"
#include "Load_Settings.h"
#include "B_Endian.h"
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

bool append_TMAP_PRO_tiles_LST(user_info* usr_nfo, tt_arr_handle* head, export_state* state);
bool backup_append_LST(char* path, char* string);

//TODO: refactor append_tiles_lst() to work here
//arr stands for tt_arr*
char* make_PRO_tiles_LST(tt_arr_handle* head, uint8_t* match_buff_src)
{
    uint8_t* match_buff = match_buff_src;
    if (match_buff_src == NULL) {
        //create a blank buffer
        match_buff = (uint8_t*)calloc(1+head->size/8, 1);
    }

    //get total_size of tile_ids where no match was found
    //used for allocating buffer
    int     match_ctr  = 0;
    uint8_t shift_ctr  = 0;
    int     total_size = 0;
    tt_arr* tiles = head->tile;
    for (int i = 0; i < head->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == -1) {
            continue;
        }

        int match = match_ctr/8;
        int shift = 1 << shift_ctr;

        // if (!(match_buff[match_ctr/8]) & (1 << shift_ctr)) {
        if (!(match_buff[match] & shift)) {
            total_size += 14;    //total length of name +2 for /r/n
        }
        match_ctr++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
    }

    //if there are no nodes (or none with viable names)
    if (total_size < 1) {
        ImGui::OpenPopup("TILES.LST Unmodified");
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
        if (node->tile_id == -1) {
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
        if (node->tile_id == -1) {
            continue;
        }
        num_tiles++;
    }
    uint8_t* matches = (uint8_t*)calloc(1+num_tiles/8, 1);

    //if tiles_lst doesn't exist
    //create new list from new_protos and return it
    if (tiles_lst == nullptr) {
        char* cropped_list = make_PRO_tiles_LST(new_protos, matches);
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
        if (node->tile_id == -1) {
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
    char* cropped_list = make_PRO_tiles_LST(new_protos, matches);

    free(matches);
    return cropped_list;
}

//ImGui menu input
//returns stored name
char* input_name()
{
    //TODO: move name_buff into export_state struct and pass into here
    static char name_buff[23] = "";
    ImGui::InputText(
        "Name\n"
        "(max 23 characters)",
        name_buff, 23);

    return name_buff;
}
//ImGui menu input
//returns stored description
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

char* make_PRO_tile_MSG(proto_info* info, int tile_id)
{
    char* msg_line = (char*)malloc(512+32);
    if (msg_line == NULL) {
        //TODO: log to file
        set_popup_warning(
            "[ERROR] make_PRO_tile_MSG()\n"
            "Unable to allocate enough memory for msg_line\n"
        );
        printf("Error: make_PRO_tile_MSG() allocate memory for msg_line failed: %d\n", __LINE__);
    }

    snprintf(msg_line, 512+32,
            "{%d}{}{%s}\r\n{%d}{}{%s}\r\n",
            tile_id*100,   info->name,
            tile_id*100+1, info->description);

    return msg_line;
}

bool append_PRO_tile_MSG(user_info* usr_nfo, proto_info* info, tt_arr_handle* handle)
{
    char* FRM_tiles_LST = usr_nfo->game_files.FRM_TILES_LST;
    if (FRM_tiles_LST == NULL) {
        //need /art/tiles/TILES.LST in order to
        //  get line numbers for proto tile_id
        return false;
    }
    char* game_path = usr_nfo->default_game_path;
    if (game_path[0] == '\0') {
        return false;
    }

    tt_arr* tile;
    for (int i = 0; i < handle->size; i++)
    {
        tile = &handle->tile[i];
        if (tile->tile_id == -1) {
            continue;
        }
        break;
    }

    proto_info pr_info;
    if (info == NULL) {
        info                = &pr_info;
        pr_info.name        = input_name();
        pr_info.description = input_desc();
    }

    if (info->name[0] == '\0' && info->description[0] == '\0') {
        //append to pro_tile.msg if either a name
        //or a description has been provided
        return false;
    }

    assign_tile_id(handle, FRM_tiles_LST);
    char* new_PRO_tile_MSG = make_PRO_tile_MSG(info, tile->tile_id);

    const char* language[] = {
        "english",
        "french",
        "russian",
        "etc"
    };

    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/text/%s/game/pro_tile.msg", game_path, language[0]);
    char* actual_path = io_path_check(save_path);
    if (actual_path) {
        strncpy(save_path, actual_path, MAX_PATH);
    }

    bool success = io_backup_file(save_path, nullptr);
    success      = io_save_txt_file(save_path, new_PRO_tile_MSG);
    if (!success) {
        free(new_PRO_tile_MSG);
        return false;
    }

    // assign new_PRO_tile_MSG;
    if (usr_nfo->game_files.PRO_TILE_MSG) {
        free(usr_nfo->game_files.PRO_TILE_MSG);
    }
    usr_nfo->game_files.PRO_TILE_MSG = new_PRO_tile_MSG;

    return true;
}

bool missing_files_popup(export_state* state)
{

    const char* art = "";
    const char* pro = "";
    const char* msg = "";
    if (state->loaded_FRM_LST == false) {
        art = "/data/art/tiles/TILES.LST\n";
    }
    if (state->loaded_PRO_LST == false) {
        pro = "/data/proto/tiles/TILES.LST\n";
    }
    if (state->loaded_PRO_MSG == false) {
        msg = "/data/text/%s/game/pro_tile.msg\n";
    }


    // char* lst_path = state->LST_path;
    ImGui::Text(
        "Unable to find these files:\n"
        "\n"
        "%s%s%s"
        "\n"

        "Would you like to make new ones?\n"
        "These new proto files will be blank\n"
        "(except for the new tiles made here),\n"
        "and will create all the subfolders\n"
        "necessary for the game engine to load\n"
        "these new files.\n\n"

        "--IMPORTANT--\n"
        "The Fallout game engine reads proto IDs/FRM names\n"
        "in from *.LST files based on the line number.\n"
        "The new *.LST files will override the old ones.\n"
        "Only do this if you want to create\n"
        "the whole tile system from scratch,\n"
        "or to preview the results before manually merging.\n\n"

        , art,pro,msg//,language[0]
    );
    if (ImGui::Button("Create new files?")) {
        if (state->loaded_FRM_LST == false) {
            state->make_FRM_LST = true;
        } else {
            state->append_FRM_LST = true;
        }

        if (state->loaded_PRO_LST == false) {
            state->make_PRO_LST = true;
        } else {
            state->append_PRO_LST = true;
        }

        if (state->loaded_PRO_MSG == false) {
            state->make_PRO_MSG = true;
        } else {
            state->append_PRO_MSG = true;
        }

        state->export_proto   = true;
        ImGui::CloseCurrentPopup();
        return true;
    }

    ImGui::BeginDisabled();
    if (ImGui::Button("Extract files from master.dat")) {
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

char* save_NEW_PRO_tile_MSG(tt_arr_handle* handle, user_info* usr_nfo, export_state* state)
{
    char* FRM_tiles_LST = usr_nfo->game_files.FRM_TILES_LST;
    if (FRM_tiles_LST == NULL) {
        //need /art/tiles/TILES.LST in order to
        //  get line numbers for proto tile_id
        return NULL;
    }
    char* game_path = usr_nfo->default_game_path;
    if (game_path[0] == '\0') {
        return NULL;
    }
    tt_arr* tile;
    for (int i = 0; i < handle->size; i++)
    {
        tile = &handle->tile[i];
        if (tile->tile_id == -1) {
            continue;
        }
        break;
    }

    proto_info info;
    info.name        = input_name();
    info.description = input_desc();

    // if (strlen(info->name) > 1 || strlen(info->description) > 1) {
    if (info.name[0] == '\0' && info.description[0] == '\0') {
        return NULL;
    }

    assign_tile_id(handle, FRM_tiles_LST);

    char* new_PRO_tile_MSG = make_PRO_tile_MSG(&info, tile->tile_id);

    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/text/english/game/pro_tile.msg", game_path);
    char* actual_path = io_path_check(save_path);
    if (actual_path) {
        strncpy(save_path, actual_path, MAX_PATH);
    }

    bool success = io_create_path_from_file(save_path);
    if (!success) {
        set_false(state);
        set_popup_warning(
            "Error: save_NEW_FRM_tiles_LST()\n"
            "Unable to create folders\n"
        );
        return NULL;
    }

    success = io_save_txt_file(save_path, new_PRO_tile_MSG);
    if (!success) {
        return NULL;
    }

    return new_PRO_tile_MSG;
}

char* save_NEW_PRO_tiles_LST(tt_arr_handle* handle, user_info* usr_nfo, export_state* state)
{
    char* game_path     = usr_nfo->default_game_path;
    char* FRM_tiles_LST = usr_nfo->game_files.FRM_TILES_LST;
    if (FRM_tiles_LST == NULL) {
        //need /art/tiles/TILES.LST in order to
        //  get line numbers for proto tile_id
        return NULL;
    }
    if (game_path[0] == '\0') {
        return NULL;
    }
    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", game_path);
    char* actual_path = io_path_check(save_path);
    if (actual_path) {
        strncpy(save_path, actual_path, MAX_PATH);
    }

    bool success = io_create_path_from_file(save_path);
    if (!success) {
        set_false(state);
        set_popup_warning(
            "Error: save_NEW_FRM_tiles_LST()\n"
            "Unable to create folders\n"
        );
        printf("Error: save_NEW_PRO_tiles_LST() L%d\n"
                "io_create_path_from_file() failed\n", __LINE__);
        return NULL;
    }

    assign_tile_id(handle, FRM_tiles_LST);

    //NULL match_buff so all tiles are added to new_tile_list
    char* new_tile_list = make_PRO_tiles_LST(handle, NULL);
    success = io_save_txt_file(save_path, new_tile_list);
    if (!success) {
        return NULL;
    }

    return new_tile_list;
}



//TODO: get rid of the bool return?
//      I'm not using it for anything right now
//TODO: delete, not used anymore
bool create_PRO_tiles_LST_popup(export_state* state)
{
    char* lst_path = state->LST_path;
    ImGui::Text(
        "Unable to find TILES.LST in"
        "\n\n%s\n\n"

        "Would you like to make a new one?\n"
        "This new proto TILES.LST will be blank\n"
        "(except for the new tiles made here),\n"
        "and will create all the subfolders\n"
        "necessary for the game engine to load\n"
        "this new proto TILES.LST file.\n\n"

        "--IMPORTANT--\n"
        "The Fallout game engine reads proto IDs in\n"
        "from /proto/TILES.LST based on the line number.\n"
        "The new proto TILES.LST file will override the\n"
        "game list. Only do this if you want to create\n"
        "the whole tile system from scratch,\n"
        "or to preview the results before manually merging.\n\n"

        , lst_path
    );
    if (ImGui::Button("Create new LST file?")) {
        state->make_PRO_LST   = true;
        state->append_PRO_MSG = true;
        state->export_proto   = true;
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

//TODO: get rid of the bool return?
//      I'm not using it for anything right now
//TODO: delete, not used anymore
bool create_FRM_tiles_LST_popup(export_state* state)
{
    char* lst_path = state->LST_path;
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
        state->make_FRM_LST   = true;
        state->append_PRO_MSG = true;
        state->export_proto   = true;
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

void export_protos(user_info* usr_nfo, tt_arr_handle* handle)
{
    if (handle == nullptr) {
    //TODO: place a warning here, this needs tile_arr*head to work
    //TODO: maybe implement this?
        return;
    }
    if (usr_nfo->default_game_path[0] == '\0') {
        return;
    }
    if (!usr_nfo->game_files.FRM_TILES_LST) {
        return;
    }

    proto_info info;
    info.name        = input_name();
    info.description = input_desc();
    info.material_id = get_material_id();

    //tiles can reference different line numbers in pro_tile.msg
    //have all tiles from this batch point to first new tile entry
    tt_arr* tiles = handle->tile;
    info.pro_tile = tiles->tile_id * 100;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == -1) {
            continue;
        }
        bool success = export_single_tile_PRO(usr_nfo->default_game_path, node, &info);
        if (!success) {
            set_popup_warning(
                "[ERROR] export_protos()\n\n"
                "Something broke while exporting *.pro files.\n"
            );
            break;
        }
    }
}

bool load_PRO_tiles_MSG(user_info* usr_nfo, export_state* state)
{
    char* LST_path = state->LST_path;
    const char* language[] = {
        "english",
        "french",
        "russian"
    };

    snprintf(LST_path, MAX_PATH, "%s/data/text/%s/game/pro_tile.msg", usr_nfo->default_game_path, language[0]);
    char* actual_path = io_path_check(LST_path);
    if (actual_path) {
        strncpy(LST_path, actual_path, MAX_PATH);
    }

    char* old_PRO_MSG = io_load_txt_file(LST_path);
    if (old_PRO_MSG == nullptr) {
        //TODO: may want to handle other failures
        //      which would cause io_load_text_file()
        //      to return NULL/nullptr

        ImGui::OpenPopup("Missing Files");

        state->auto_export    = false;
        state->export_proto   = false;
        state->chk_game_path  = false;

        state->make_FRM_LST   = false;
        state->make_PRO_LST   = false;
        state->make_PRO_MSG   = false;

        state->load_files     = false;
        // state->loaded_FRM_LST   = false;
        // state->loaded_PRO_LST   = false;
        // state->loaded_PRO_MSG   = false;

        state->append_FRM_LST = false;
        state->append_PRO_LST = false;
        state->append_PRO_MSG = false;

        printf("Unable to load /proto/tiles/TILES.LST...\nCreating new one...\n");
        return false;
    }

    if (usr_nfo->game_files.PRO_TILE_MSG) {
        free(usr_nfo->game_files.PRO_TILE_MSG);
    }
    usr_nfo->game_files.PRO_TILE_MSG = old_PRO_MSG;
    return true;
}

bool load_PRO_tiles_LST(user_info* usr_nfo, export_state* state)
{
    char* LST_path = state->LST_path;
    snprintf(LST_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", usr_nfo->default_game_path);
    char* actual_path = io_path_check(LST_path);
    if (actual_path) {
        strncpy(LST_path, actual_path, MAX_PATH);
    }

    char* old_PRO_LST = io_load_txt_file(LST_path);
    if (old_PRO_LST == nullptr) {
        //TODO: may want to handle other failures
        //      which would cause io_load_text_file()
        //      to return NULL/nullptr

        ImGui::OpenPopup("Missing Files");

        state->auto_export    = false;
        state->export_proto   = false;
        state->chk_game_path  = false;

        state->make_FRM_LST   = false;
        state->make_PRO_LST   = false;
        state->make_PRO_MSG   = false;

        state->load_files     = false;
        // state->loaded_FRM_LST   = false;
        // state->loaded_PRO_LST   = false;
        // state->loaded_PRO_MSG   = false;

        state->append_FRM_LST = false;
        state->append_PRO_LST = false;
        state->append_PRO_MSG = false;

        printf("Unable to load /proto/tiles/TILES.LST...\nCreating new one...\n");
        return false;
    }

    if (usr_nfo->game_files.PRO_TILES_LST) {
        free(usr_nfo->game_files.PRO_TILES_LST);
    }
    usr_nfo->game_files.PRO_TILES_LST = old_PRO_LST;
    return true;
}

void export_PRO_tiles_POPUP(user_info* usr_nfo, tt_arr_handle* handle, export_state* state, bool auto_export)
{
    static char* lst_path = NULL;

    //input name
    ImGui::Text(
        "In order to get new tiles to appear in the mapper\n"
        "(and thus in the game), each tile must have a proto(.pro)\n"
        "file made, and an entry for each tile appended to\n\n"
        "   Fallout 2/data/art/tiles/TILES.LST\n"
        "   Fallout 2/data/proto/tiles/TILES.LST\n\n"
        "In addition, entries can optionally be made in\n\n"
        "   Fallout 2/data/text/english/game/pro_tile.msg\n\n"
        "to give the tile a name and description in the\n"
        "Fallout 2 mapper (Mapper2.exe).\n\n"
    );

    if (!auto_export) {
        ImGui::Text(
            "For this to work, please provide the path to\n"
            "fallout2.exe in your modded Fallout 2 folder,\n"
            "and have these files extracted to their\n"
            "appropriate locations.\n"
            "(I plan on adding a feature to extract these)\n"
            "(automatically, but currently can't do this.)\n"
        );
    }

    static char FObuf[MAX_PATH] = "";
    if (FObuf[0] == '\0' && usr_nfo->default_game_path[0] != '\0') {
        strncpy(FObuf, usr_nfo->default_game_path, MAX_PATH);
    }
    if (!auto_export) {
        ImGui::InputText("###fallout2.exe", FObuf, MAX_PATH);
    }

    ImGui::Text(
        "\nThese are Optional,\n"
        "and will be applied to all tiles in this set.\n"
    );

    get_material_id();
    input_name();
    input_desc();

    if (!handle) {
        return;
    }

    if (!auto_export) {
        if (ImGui::Button("Add to Fallout 2")) {
            state->export_proto   = true;

            state->load_files     = true;

            state->append_FRM_LST = true;
            state->append_PRO_LST = true;
            state->append_PRO_MSG = true;

            state->chk_game_path  = true;
        }
    }


    //Begin Popups/////////////////////////////////

    if (ImGui::BeginPopupModal("Missing Files")) {
        missing_files_popup(state);
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
            , state->LST_path
        );
        if (ImGui::Button("Append to TILES.LST")) {
            state->auto_export    = true;
            state->append_FRM_LST = true;
            state->append_PRO_LST = true;
            state->append_PRO_MSG = true;

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
            , state->LST_path
        );
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("fallout2.exe not found")) {
        ImGui::Text(
            "The Fallout 2 executable (fallout2.exe) couldn't be found at\n\n"
            "%s\n\n"
            "In order to make these tiles accessible in the\n"
            "Fallout 2 mapper (mapper2.exe), we need to add\n"
            "entries in several files located in the Fallout 2\n"
            "sub-directories --\n"
            "'/art/tiles/TILES.LST',\n"
            "'/proto/tiles/TILES.LST'.\n"
            "(optional)\n"
            "'/text/english/game/pro_tile.msg'(just english for now, will add more later)\n\n"
            "If you want to bypass this for now, just add a file\n"
            "named 'fallout2.exe' into the selected folder and retry.\n"
            , FObuf
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

    if (state->chk_game_path) {
        //copy any game_path changes to user_info for saving to config
        if (fallout2exe_exists(FObuf) == false) {
            ImGui::OpenPopup("fallout2.exe not found");
            set_false(state);
            return;
        }
        strncpy(usr_nfo->default_game_path, FObuf, MAX_PATH);
    }


    if (state->load_files) {
        state->loaded_FRM_LST = load_FRM_tiles_LST(usr_nfo, state);
        state->loaded_PRO_LST = load_PRO_tiles_LST(usr_nfo, state);
        state->loaded_PRO_MSG = load_PRO_tiles_MSG(usr_nfo, state);

        if (!state->loaded_FRM_LST || !state->loaded_PRO_LST || !state->loaded_PRO_MSG) {
            //TODO: maybe want to handle other failures
            //      which would cause io_load_txt_file()
            //      to return NULL/nullptr
            return;
        }
    }

    if (state->make_FRM_LST) {
        //TODO: maybe I should group these together
        //      and check game_files pointers to see
        //      if I need to create new files or not?
        // if (state.make_files) {
        // if (!usr_nfo->game_files.FRM_TILES_LST) {
        usr_nfo->game_files.FRM_TILES_LST = save_NEW_FRM_tiles_LST(handle, usr_nfo->default_game_path, state);
    }
    if (state->make_PRO_LST) {
        usr_nfo->game_files.PRO_TILES_LST = save_NEW_PRO_tiles_LST(handle, usr_nfo, state);
    }
    if (state->make_PRO_MSG) {
        usr_nfo->game_files.PRO_TILE_MSG  = save_NEW_PRO_tile_MSG(handle, usr_nfo, NULL);
    }

    if (!usr_nfo->game_files.FRM_TILES_LST ||
        !usr_nfo->game_files.PRO_TILES_LST ||
        !usr_nfo->game_files.PRO_TILE_MSG) {
        return;
    }
    state->make_FRM_LST = false;
    state->make_PRO_LST = false;
    state->make_PRO_MSG = false;

    bool success = false;
    if (state->append_FRM_LST) {
        //append to art/tiles/TILES.LST
        success = append_TMAP_tiles_LST(usr_nfo, handle, state);
        if (!success) {
            set_false(state);
            return;
        }
    }
    if (state->append_PRO_LST) {
        //append to proto/tiles/TILES.LST
        success = append_TMAP_PRO_tiles_LST(usr_nfo, handle, state);
        if (!success) {
            set_false(state);
            return;
        }
    }
    if (state->append_PRO_MSG) {
        //TODO: need to add option for different languages?
        //TODO: also need to give options to use descriptions
        //      already on pro_tile.msg
        //add name/description to data/text/english/game/pro_tile.msg
        success = append_PRO_tile_MSG(usr_nfo, NULL, handle);
        if (!success) {
            set_false(state);
            return;
        }
    }

    if (state->export_proto) {
        export_protos(usr_nfo, handle);
    }

    set_false(state);
}

//append new protos to list in memory
char* append_PRO_tiles_LST(char* old_PRO_LST, tt_arr_handle* head, bool set_auto)
{
    char* new_PRO_LST = check_PRO_LST_names(old_PRO_LST, head);
    if (new_PRO_LST == nullptr) {
        return old_PRO_LST;
    }

    //append new list_of_tiles to the end of original list
    //in a new buffer large enough to fit both
    int old_LST_size    = strlen(old_PRO_LST);
    int new_LST_size    = strlen(new_PRO_LST);
    int final_size      = old_LST_size+new_LST_size+1;
    char* final_PRO_LST = (char*)malloc(old_LST_size + new_LST_size +1);   //+1 for null char
    snprintf(final_PRO_LST, final_size, "%s%s", old_PRO_LST, new_PRO_LST);

    return final_PRO_LST;
}

//##### I feel like this is a stupid way to write this
//##### but it's better than it was,
//##### and I don't know a better way yet
//this assumes usr_info->default_game_path has been set
//and art/tiles/TILES.LST has been loaded up correctly
//append to data/proto/tiles/TILES.LST
bool append_TMAP_PRO_tiles_LST(user_info* usr_nfo, tt_arr_handle* head, export_state* cur_state)
{
    char* game_path   = usr_nfo->default_game_path;
    char* old_PRO_LST = usr_nfo->game_files.PRO_TILES_LST;
    char* new_PRO_LST = append_PRO_tiles_LST(old_PRO_LST, head, false);

    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/proto/tiles/TILES.LST", game_path);
    char* actual_path = io_path_check(save_path);
    if (actual_path) {
        strncpy(save_path, actual_path, MAX_PATH);
    }

    //backup and save new list
    bool success = io_backup_file(save_path, nullptr);
    success = io_save_txt_file(save_path, new_PRO_LST);
    if (!success) {
        free(new_PRO_LST);
        return false;
    }

    if (new_PRO_LST != usr_nfo->game_files.PRO_TILES_LST) {
        free(usr_nfo->game_files.PRO_TILES_LST);
    }
    usr_nfo->game_files.PRO_TILES_LST = new_PRO_LST;


    return true;
}

//backs up file at "path",
//appends "names" to text file at "path"
//TODO: delete (not used anymore?)
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

bool export_single_tile_PRO(char* game_path, tt_arr* tile, proto_info* info)
{
    if (tile == nullptr) {
        //TODO: place a warning here?
        //      this needs tt_arr*head to work
        return false;
    }

    char path_buff[MAX_PATH];

    tile_proto proto;
    //protoIDs are 1 indexed? (1-indexing happens on assignment now)
    proto.ObjectID        = tile->tile_id | 0x4000000;

    //used as a key/value pair in pro_tile.msg
    //key number from /text/language/game/pro_tile.msg
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

    //TODO: create folder paths if they don't exist
    snprintf(path_buff, MAX_PATH, "%s/data/proto/tiles/%08d.pro", game_path, tile->tile_id);
    char* actual_path = io_path_check(path_buff);
    if (actual_path) {
        strncpy(path_buff, actual_path, MAX_PATH);
    }
    //TODO: maybe want to add io_create_path_from_file()
    //      so folder is created if it didn't exist already?

    char* ptr = strrchr(path_buff, PLATFORM_SLASH);
    char back = ptr[0];
    ptr[0]    = '\0';
    if (!io_isdir(path_buff)) {
        io_make_dir(path_buff);
    }
    ptr[0] = back;

    FILE* tile_pro = fopen(path_buff, "wb");
    if (tile_pro == nullptr) {
        printf("Unable to open proto file: %s\n", path_buff);
        fclose(tile_pro);
        return false;
    }
    fwrite(&proto, sizeof(tile_proto), 1, tile_pro);
    fclose(tile_pro);

    return true;
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
//returns selected value when called
int get_material_id()
{
    static int material_id = 0;
    const char* names[] = { "Glass", "Metal", "Plastic", "Wood", "Dirt", "Stone", "Cement", "Leather" };
    ImGui::Combo("Material Type", &material_id, names, IM_ARRAYSIZE(names));

    return material_id;
}