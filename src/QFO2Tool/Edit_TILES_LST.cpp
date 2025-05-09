//Building and cutting new tiles for Fallout 1 and 2. by Pixote
//https://www.nma-fallout.com/threads/faq-guides-tutorials.156494/page-2#post-3839275
#include <time.h>

#include "platform_io.h"
#include "Load_Settings.h"
#include "Save_Files.h"
#include "Edit_TILES_LST.h"
#include "ImGui_Warning.h"
#include "Proto_Files.h"

void generate_new_tile_list_arr(char* name, tt_arr_handle* handle)
{
    tt_arr* node = handle->tile;
    int counter = 0;

    for (int i = 0; i < handle->size; i++)
    {
        if (node[i].tile_id == 1) {
            continue;
        }
        snprintf(node[i].name_ptr, 14, "%s%03d.FRM\r\n", name, counter);
    }
}


//TODO: should I use this? or the one below?
//read game TILES.LST into memory using the game path
//TODO: delete
char* load_tiles_lst_game(char* game_path)
{
    //nothing to load if default_game_path not set
    if (game_path == nullptr) {
        return nullptr;
    }
    //check if file exists
    char full_path[MAX_PATH] = {0};
    snprintf(full_path, MAX_PATH, "%s%s", game_path, "/data/art/tiles/TILES.LST");
    return io_load_txt_file(full_path);
}


//read TILES.LST into memory directly from provided path
//TODO: delete? only used in export_TMAP_tiles_pattern()
char* load_LST_file(char* game_path, char* LST_path, char* LST_file)
{
    //nothing to load if default_game_path not set
    if (game_path == nullptr) {
        return nullptr;
    }
    if (LST_path == NULL) {
        return NULL;
    }
    if (LST_file == NULL) {
        return NULL;
    }
    //check if file exists
    char full_path[MAX_PATH] = {0};
    snprintf(full_path, MAX_PATH, "%s%s%s", game_path, LST_path, LST_file);

    return io_load_txt_file(full_path);
}

//create/overwrite TILES.LST at provided path
//TODO: delete?
char* write_tiles_lst(char* tiles_lst_path, char* list_of_tiles)
{
    if (tiles_lst_path == nullptr) {return nullptr;}

    FILE* tiles_lst = fopen(tiles_lst_path, "wb");
    fwrite(list_of_tiles, strlen(list_of_tiles), 1, tiles_lst);
    fclose(tiles_lst);

    return list_of_tiles;
}


//testing array list version/////////////////////////////////////////////////////start

//create a array of linked lists of tile names
//from the char* tiles_list passed in
tile_name_arr* make_name_list_arr(char* new_tiles_list)
{
    tile_name_arr* large_list = (tile_name_arr*)malloc(sizeof(tile_name_arr) * 10000);
    int node         = 0;
    char* lst_ptr    = new_tiles_list;
    char* name_start = new_tiles_list;
    char* name_end   = nullptr;

    while (*lst_ptr != '\0') {
        char c = *lst_ptr;
        if (c != '\n') {
            lst_ptr++;
            continue;
        }
        name_end = lst_ptr;
        large_list[node].length = name_end - name_start;
        large_list[node].name_ptr = name_start;
        large_list[node].next = node + 1;

        name_start = name_end + 1;
        lst_ptr++;
        node++;
    }

    large_list[node-1].next = 0;
    return large_list;
}


//returns list of tile names being exported
//passing NULL into match_buff_src will
//auto allocate an empty buffer of appropriate size
//this creates a list that includes all(?) handle->names
char* make_FRM_tile_LST(tt_arr_handle* handle, uint8_t* match_buff_src)
{
    uint8_t* match_buff = match_buff_src;
    if (match_buff_src == NULL) {
        //create a blank buffer
        match_buff = (uint8_t*)calloc(1+handle->size/8, 1);
    }
    //buff_size = total length of all names
    tt_arr* tiles = handle->tile;
    uint8_t shift_ctr  = 0;
    int     tile_num   = 0;
    int     buff_size = 0;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == -1) {
            continue;
        }

        int indx = tile_num/8;
        uint8_t shift = 1 << shift_ctr;
        if (!(match_buff[indx] & shift)) {
            buff_size += strlen(node->name_ptr)+2;  //+2 for /r/n
        }
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
        tile_num++;
    }

#pragma region                                                         popup
    if (buff_size < 1) {
        //if there are no nodes (or none with viable names)
        ImGui::OpenPopup("TILES.LST Unmodified");
        return nullptr;
    }

    char* cropped_list = (char*)malloc(buff_size+1);
    shift_ctr  = 0;
    tile_num   = 0;
    char* c    = cropped_list;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == -1) {
            continue;
        }

        if (!(match_buff[tile_num/8] & 1 << shift_ctr)) {
            size_t copy_len = strlen(node->name_ptr);
            memcpy(c, node->name_ptr, copy_len);
            //TODO: delete after testing
            // *(c + copy_len)   = '\r';
            // *(c + copy_len+1) = '\n';
            c[copy_len]   = '\r';
            c[copy_len+1] = '\n';
            c += copy_len+2;

            node->tile_id = tile_num;
        }

        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
        tile_num++;
    }
    c[0] = '\0';

    if (match_buff_src == NULL) {
        free(match_buff);
    }

//test above for speed///////////////////////////////
    return cropped_list;
}

char* save_NEW_FRM_tiles_LST(tt_arr_handle* handle, char* game_path, export_state* state)
{
    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/art/tiles/TILES.LST", game_path);

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

    //NULL match_buff so all tiles are added to new_tile_list
    char* new_tile_LST = make_FRM_tile_LST(handle, NULL);
    success = io_save_txt_file(save_path, new_tile_LST);
    if (!success) {
        return NULL;
    }
    state->make_FRM_LST = false;

    return new_tile_LST;
}


char* check_FRM_LST_names(char* old_tiles_LST, tt_arr_handle* handle, export_state* state)
{
    bool append_new_only = state->auto_export;
    int num_tiles = 0;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &handle->tile[i];
        if (node->tile_id != 1) {
            num_tiles++;
        }
    }

    int tiles_lst_len = strlen(old_tiles_LST);
    uint8_t shift_ctr = 0;

    uint8_t* matches = (uint8_t*)calloc(1+num_tiles/8, 1);
    char* strt       = old_tiles_LST;           //keeps track of first letter of name on TILES.LST

    int match_ctr = 0;
    tt_arr* tiles = handle->tile;
    for (int i = 0; i < handle->size; i++)
    {
        int line_ctr = 0;                   //.LST file line numbers are 1-indexed (not 0-indexed)
        tt_arr* node = &tiles[i];
        if (node->tile_id == -1) {
            continue;
        }

        for (int char_ctr = 0; char_ctr < tiles_lst_len; char_ctr++)
        {
            if (old_tiles_LST[char_ctr] != '\n' && old_tiles_LST[char_ctr] != '\0') {
                continue;
            }
            line_ctr++;
            //compare first character before full string
            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                strt = &old_tiles_LST[char_ctr+1];
                continue;
            }
            //TODO: replace with io_strncasecmp()? or at least strncsecmp()?
            if (strncmp(strt, node->name_ptr, strlen(node->name_ptr)) != 0) {
                strt = &old_tiles_LST[char_ctr+1];
                continue;
            }
            node->tile_id = line_ctr;

            //first match found, ask what to do
            if (append_new_only == false) {
                ImGui::OpenPopup("Append to FRM LST");
                state->auto_export    = false;
                state->export_proto   = false;
                state->export_pattern = false;
                state->chk_game_path  = false;

                // state->make_FRM_LST   = false;
                // state->make_PRO_LST   = false;
                // state->make_PRO_MSG   = false;

                state->load_files     = false;

                // state->loaded_FRM_LST = false;
                // state->loaded_PRO_LST = false;
                // state->loaded_PRO_MSG = false;

                state->append_FRM_LST = false;
                state->append_PRO_LST = false;
                state->append_PRO_MSG = false;

                return NULL;
            }

            if (append_new_only == true) {
                //identify this node as having a duplicate match
                matches[match_ctr/8] |= 1 << shift_ctr;
                break;
            }
        }
        //increment all the counters
        match_ctr++;
        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
        strt = old_tiles_LST;
    }

    //TODO: maybe pull this function out to the surface?
    //      would make the whole process flatter,
    //      but its a pita to refactor
    //generate new list from remaining nodes in linked_lst
    char* cropped_list = nullptr;
    cropped_list = make_FRM_tile_LST(handle, matches);

    free(matches);
    return cropped_list;
}

//checks old_FRM_LST for matching tilenames from handle.tiles
//  if only matches found, returns old_FRM_LST
//  if new entries made (some non-matches found)
//      allocates space for total (old LST w/new names appended)
//      and returns allocated ptr
char* append_FRM_tiles_LST(char* old_FRM_LST, tt_arr_handle* handle, export_state* state)
{
    //search old_tiles_list (TILES.LST)
    //for matching names from tiles in handle
    char* new_FRM_LST = check_FRM_LST_names(old_FRM_LST, handle, state);
    if (new_FRM_LST == nullptr) {
        //either matches found or no new names added to LST file
        return old_FRM_LST;
    }

    //append new_FRM_LST to the end of old_FRM_LST
    //in a new buffer large enough to fit both
    int old_LST_size    = strlen(old_FRM_LST);
    int new_LST_size    = strlen(new_FRM_LST);
    int final_size      = old_LST_size + new_LST_size + 1;      //+1 for null char
    char* final_FRM_LST = (char*)malloc(final_size);
    snprintf(final_FRM_LST, final_size, "%s%s", old_FRM_LST, new_FRM_LST);

    return final_FRM_LST;
}

void set_false(export_state* state)
{
    state->auto_export    = false;
    state->export_proto   = false;
    state->export_pattern = false;
    state->chk_game_path  = false;

    state->make_FRM_LST   = false;
    state->make_PRO_LST   = false;
    state->make_PRO_MSG   = false;

    state->load_files     = false;

    // state->loaded_FRM_LST = false;
    // state->loaded_PRO_LST = false;
    // state->loaded_PRO_MSG = false;

    state->append_FRM_LST = false;
    state->append_PRO_LST = false;
    state->append_PRO_MSG = false;
}

bool load_FRM_tiles_LST(user_info* usr_nfo, export_state* state)
{
    char* LST_path = state->LST_path;
    snprintf(LST_path, MAX_PATH, "%s/data/art/tiles/TILES.LST", usr_nfo->default_game_path);
    char* actual_path = io_path_check(LST_path);
    if (actual_path) {
        strncpy(LST_path, actual_path, MAX_PATH);
    }

    char* FRM_tiles_lst = io_load_txt_file(LST_path);
    if (FRM_tiles_lst == NULL) {
        //TODO: may want to handle other failures
        //      which would cause io_load_txt_file()
        //      to return NULL/nullptr
        ImGui::OpenPopup("Missing Files");

        state->auto_export    = false;
        state->export_proto   = false;
        state->export_pattern = false;
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

        printf("Unable to load /art/tiles/TILES.LST...\nCreating new one...\n");
        return false;
    }

    if (usr_nfo->game_files.FRM_TILES_LST) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    usr_nfo->game_files.FRM_TILES_LST = FRM_tiles_lst;

    return true;
}

//Create new art TILES.LST then append to old art TILES.LST
//  full LST is then attached to usr_nfo.game_files.FRM_TILES_LST
//  On fail returns false, game_files.FRM_TILES_LST not changed
bool append_TMAP_tiles_LST(user_info* usr_nfo, tt_arr_handle* handle, export_state* state)
{
    char* game_path = usr_nfo->default_game_path;

    //append new tiles.LST to old tiles.LST in memory
    char* FRM_tiles_lst = usr_nfo->game_files.FRM_TILES_LST;
    char* new_tiles_lst = append_FRM_tiles_LST(FRM_tiles_lst, handle, state);
    if (new_tiles_lst == FRM_tiles_lst) {
        return true;
    }

    char save_path[MAX_PATH];
    snprintf(save_path, MAX_PATH, "%s/data/art/tiles/TILES.LST", game_path);
    char* actual_path = io_path_check(save_path);
    if (actual_path) {
        strncpy(save_path, actual_path, MAX_PATH);
    }

    //write combined lists out
    bool success = io_backup_file(save_path, nullptr);
    success = io_save_txt_file(save_path, new_tiles_lst);
    if (!success) {
        set_popup_warning(
            "[ERROR] append_TMAP_tiles_LST()"
            "Unable to append to FRM TILES.LST\n"
        );
        free(new_tiles_lst);
        return false;
    }

    if (usr_nfo->game_files.FRM_TILES_LST) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    usr_nfo->game_files.FRM_TILES_LST = new_tiles_lst;
    state->append_FRM_LST = false;

    return true;
}

//makes a char* list of tilenames from handle
//      and appends (or replaces) original TILES.LST
void append_FRM_tiles_POPUP(user_info* usr_nfo, tt_arr_handle* handle, export_state* state, bool auto_export)
{
    //input name
    ImGui::Text(
        "In order to get new FRMs to appear in the Fallout 2\n"
        "mapper (mapper2.exe), new entries must be made in\n\n"
        "   Fallout 2/data/art/tiles/TILES.LST\n\n"
        "For this to work, please provide the path to\n"
        "fallout2.exe in your modded Fallout 2 folder,\n"
        "and have this file extracted to its\n"
        "appropriate location.\n"
    );

    static char FObuff[MAX_PATH] = "";
    if (FObuff[0] == '\0' && usr_nfo->default_game_path[0] != '\0') {
        strncpy(FObuff, usr_nfo->default_game_path, MAX_PATH);
    }
    ImGui::InputText("###fallout2.exe", FObuff, MAX_PATH);

    ImGui::Text(
        "(I plan on adding a feature to extract these from)\n"
        "(master.dat automatically, currently unimplemented.)\n\n"
        "If only exporting FRMs, the mapper must be set in\n"
        "'Librarian' mode and new protos must be made from\n"
        "these new FRMs."
    );

    if (!auto_export) {
        if (ImGui::Button("Append to art/tiles/TILES.LST")) {
            state->chk_game_path  = true;
            state->load_files     = true;
            state->append_FRM_LST = true;
        }
    }

    export_tiles_POPUPS(state, FObuff);

    if (!handle) {
        return;
    }

    if (state->chk_game_path) {
        //copy any game_path changes to user_info for saving to config
        if (fallout2exe_exists(FObuff) == false) {
            ImGui::OpenPopup("fallout2.exe not found");
            set_false(state);
            return;
        }
        strncpy(usr_nfo->default_game_path, FObuff, MAX_PATH);
    }

    if (state->load_files) {
        state->loaded_FRM_LST = load_FRM_tiles_LST(usr_nfo, state);
        //TODO: maybe isolate this better?
        //      these state->loaded_ are used in the create_LST popup
        //      so need to at least check if the other two files exist
        state->loaded_PRO_LST = load_PRO_tiles_LST(usr_nfo, state);
        state->loaded_PRO_MSG = load_PRO_tiles_MSG(usr_nfo, state);

        if (!state->loaded_FRM_LST) {
            //TODO: maybe want to handle other failures
            //      which would cause io_load_txt_file()
            //      to return NULL/nullptr?
            return;
        }
    }

    //append to art/tiles/TILES.LST
    if (state->append_FRM_LST) {
        bool success = append_TMAP_tiles_LST(usr_nfo, handle, state);
        if (!success) {
            set_false(state);
            return;
        }
    }

    if (state->make_FRM_LST) {
        usr_nfo->game_files.FRM_TILES_LST = save_NEW_FRM_tiles_LST(handle, usr_nfo->default_game_path, state);
    }
}