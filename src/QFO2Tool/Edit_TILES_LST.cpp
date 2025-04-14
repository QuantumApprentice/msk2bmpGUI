//Building and cutting new tiles for Fallout 1 and 2. by Pixote
//https://www.nma-fallout.com/threads/faq-guides-tutorials.156494/page-2#post-3839275
#include <time.h>

#include "platform_io.h"
#include "Load_Settings.h"
#include "Save_Files.h"
#include "Edit_TILES_LST.h"
#include "tinyfiledialogs.h"

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
//TODO: delete? not used yet
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

//ask user for new name,
//use old_name as default
char* get_new_name(char* old_name)
{
    // create the filename for the current list of tiles
    // assigns final save path string to Full_Save_File_Path
    char* name = tinyfd_inputBox(
                "Tile Name...",
                "Please type a default tile name for these,\n"
                "exporting will append a tile number to this name.\n",
                old_name);
    if (name == nullptr) {
        return nullptr;
    }
    //TODO: check if game engine will take more than 8 character names
    //      if not, then limit this to 8 (name length + tile digits)
    //      possibly give bypass?
    if (strlen(name) >= 32) {
        printf("name too long?");
        // return nullptr;
    }
    return name;
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

//same function as skip_or_rename()
//but uses array of linked lists
int skip_or_rename_arr(tile_name_arr* node)
{
    char msg_buff[MAX_PATH] = {
        "One of the new tile-names matches\n"
        "a tile-name already on TILES.LST.\n\n"
    };
    strncat(msg_buff, node->name_ptr, node->length);
    strncat(msg_buff, "\n"
        "YES:   Skip and append only new names?\n"
        "NO:    Rename the new tiles?\n", 74);

    int choice = tinyfd_messageBox(
                "Match found...",
                msg_buff,
                "yesnocancel", "warning", 2);
    return choice;
}

int skip_or_rename_arr(char* name)
{
    char msg_buff[255];
    snprintf(msg_buff, 255,
        "One of the new tile-names matches\n"
        "a tile-name already on TILES.LST.\n\n"
        "%s\n\n"
        "YES:   Skip and append only new names?\n"
        "NO:    Rename the new tiles?\n", name
    );

    int choice = tinyfd_messageBox(
                "Match found...",
                msg_buff,
                "yesnocancel", "warning", 2);
    return choice;
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
        if (node->tile_id == 1) {
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

    //if there are no nodes (or none with viable names)
#pragma region                                                            open popup check
    if (buff_size < 1) {
        ImGui::OpenPopup("TILES.LST Unmodified");
        return nullptr;
    }

//test below for speed///////////////////////////////
    // char* cropped_list = (char*)malloc(total_size+1);
    // cropped_list[0] = '\0';
    // node = linked_lst;
    // while (node != nullptr) {
    //     strncat(cropped_list, node->name_ptr, node->length+1);
    //     // strncat(cropped_list, "\0", 1);
    //     node = node->next;
    // }

    char* cropped_list = (char*)malloc(buff_size+1);
    // tiles       = handle->tile;
    shift_ctr  = 0;
    tile_num   = 0;
    char* c    = cropped_list;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == 1) {
            continue;
        }

        if (!(match_buff[tile_num/8] & 1 << shift_ctr)) {
            size_t copy_len = strlen(node->name_ptr);
            memcpy(c, node->name_ptr, copy_len);
            // *(c + copy_len)   = '\r';
            // *(c + copy_len+1) = '\n';
            c[copy_len]   = '\r';
            c[copy_len+1] = '\n';
            c += copy_len+2;
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

char* save_NEW_tiles_LST(tt_arr_handle* handle, char* save_path)
{
    //blank match_buff so all tiles are added to new_tile_list
    char* new_tile_list = make_FRM_tile_LST(handle, NULL);
    char* tiles_lst     = write_tiles_lst(save_path, new_tile_list);

    return tiles_lst;
}

char* check_FRM_LST_names(char* tiles_lst, tt_arr_handle* handle, bool set_auto)
{
    bool append_new_only = set_auto;
    int num_tiles = 0;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &handle->tile[i];
        if (node->tile_id != 1) {
            num_tiles++;
        }
    }

    int tiles_lst_len = strlen(tiles_lst);
    uint8_t shift_ctr = 0;

    uint8_t* matches = (uint8_t*)calloc(1+num_tiles/8, 1);
    char* strt       = tiles_lst;             //keeps track of first letter of name on TILES.LST

    int match_ctr = 0;
    tt_arr* tiles = handle->tile;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        if (node->tile_id == 1) {
            continue;
        }

        for (int char_ctr = 0; char_ctr < tiles_lst_len; char_ctr++)
        {
            if (tiles_lst[char_ctr] != '\n' && tiles_lst[char_ctr] != '\0') {
                continue;
            }
            //check first char of strt == first char of node[i].name_ptr
            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                strt = &tiles_lst[char_ctr+1];
                continue;
            }
            //TODO: replace with io_strncasecmp()? or at least strncsecmp()?
            if (strncmp(strt, node->name_ptr, strlen(node->name_ptr)) != 0) {
                strt = &tiles_lst[char_ctr+1];
                continue;
            }
            //first match found, ask what to do
            if (append_new_only == false) {
                int choice = skip_or_rename_arr(node->name_ptr);
                if (choice == CANCEL) {     //return and cancel out of the whole thing
                    // free_tile_name_lst_tt(new_tiles);
                    return nullptr;
                }
                if (choice == YES)    {     //just append names not already on TILES.LST
                    append_new_only = true;
                }
                if (choice == NO)     {     //pick a new name and re-make new_tiles then re-check
                    int length = strlen(node->name_ptr);
                    char* old_name = (char*)malloc(length);
                    strncpy(old_name, node->name_ptr, length);
                    old_name[length-7] = '\0';

                    char* new_name = get_new_name(old_name);
                    generate_new_tile_list_arr(new_name, handle);

                    free(old_name);
                    return check_FRM_LST_names(tiles_lst, handle, set_auto);
                }
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
        strt = tiles_lst;
    }

    //generate new list from remaining nodes in linked_lst
    char* cropped_list = nullptr;
    cropped_list = make_FRM_tile_LST(handle, matches);

    free(matches);
    return cropped_list;
}

char* append_FRM_tiles_LST(char* old_tiles_LST, tt_arr_handle* handle, bool set_auto)
{
    //search old_tiles_list (TILES.LST)
    //for matching names from tiles in handle
    char* cropped_list = check_FRM_LST_names(old_tiles_LST, handle, set_auto);     //TODO: this also has a tinyfd_ popup
    if (cropped_list == nullptr) {
        return old_tiles_LST;
    }

    //TODO: use an io_ function to append text instead
    //append new list_of_tiles to the end of original list
    //in a new buffer large enough to fit both
    int tiles_LST_size     = strlen(old_tiles_LST);
    int new_lst_size       = strlen(cropped_list);
    char* final_tiles_LST = (char*)malloc(tiles_LST_size + new_lst_size +1);   //+1 for null char
    strncpy(final_tiles_LST, old_tiles_LST, tiles_LST_size);
    final_tiles_LST[tiles_LST_size] = '\0';    //needed for strncat() to work
    strncat(final_tiles_LST, cropped_list, new_lst_size);

    free(old_tiles_LST);
    return final_tiles_LST;
}

char* load_FRM_tiles_LST(user_info* usr_nfo, tt_arr_handle* handle)
{
    static char LST_path[MAX_PATH];
    snprintf(LST_path, MAX_PATH, "%s/data/art/tiles/TILES.LST", usr_nfo->default_game_path);
    char* FRM_tiles_lst = io_load_txt_file(LST_path);

    if (FRM_tiles_lst == NULL) {
        //TODO: may want to handle other failures
        //      which would cause io_load_txt_file()
        //      to return NULL/nullptr
        ImGui::OpenPopup("Create LST File");
        return LST_path;
    }

    if (usr_nfo->game_files.FRM_TILES_LST) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    usr_nfo->game_files.FRM_TILES_LST = FRM_tiles_lst;
    ImGui::OpenPopup("Append to LST");

    return LST_path;
}

// Returns current full save path if fail
//TODO: need to change to return new_tiles_lst?
char* add_TMAP_tiles_to_lst_arr(user_info* usr_nfo, tt_arr_handle* handle, char* LST_path)
{
    // //Auto option
    // if (usr_nfo->auto_export == true) {
    //     success = auto_export_TMAP_tiles_lst(usr_nfo, save_buff, tiles_lst, *new_tile_list);
    //     if (success == false) {
    //         return;
    //     }
    // }

    //append new tiles to list in memory
    char* FRM_tiles_lst = usr_nfo->game_files.FRM_TILES_LST;
    char* new_tiles_lst = append_FRM_tiles_LST(FRM_tiles_lst, handle, false);
    if (new_tiles_lst != usr_nfo->game_files.FRM_TILES_LST) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    usr_nfo->game_files.FRM_TILES_LST = new_tiles_lst;

    //write combined lists out
    io_backup_file(LST_path, nullptr);
    io_save_txt_file(LST_path, new_tiles_lst);

    return LST_path;
}