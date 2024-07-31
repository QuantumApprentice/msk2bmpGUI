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
char* load_tiles_lst_game(char* game_path)
{
    //nothing to load if default_game_path not set
    if (game_path == nullptr) {
        return nullptr;
    }
    //check if file exists
    char full_path[MAX_PATH] = {0};
    snprintf(full_path, MAX_PATH, "%s%s", game_path, "/data/art/tiles/TILES.LST");

    return io_load_text_file(full_path);
}

//read TILES.LST into memory directly from provided path
char* load_tiles_lst(char* path)
{
    //nothing to load if default_game_path not set
    if (path == nullptr) {
        return nullptr;
    }
    //check if file exists
    char full_path[MAX_PATH] = {0};
    snprintf(full_path, MAX_PATH, "%s%s", path, "/TILES.LST");

    return io_load_text_file(full_path);
}

//create/overwrite TILES.LST at provided path
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

char* make_tile_list_arr(tt_arr_handle* handle, uint8_t* match_buff)
{
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
    if (buff_size < 1) {
        tinyfd_notifyPopup("TILES.LST not updated...",
                        "All new tile-names were already\n"
                        "found on TILES.LST.\n"
                        "No new tile-names were added.\n",
                        "info");
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
            size_t amount_to_copy = strlen(node->name_ptr);
            memcpy(c, node->name_ptr, amount_to_copy);
            *(c + amount_to_copy)   = '\r';
            *(c + amount_to_copy+1) = '\n';
            c += amount_to_copy+2;
        }

        shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        }
        tile_num++;
    }
    c[0] = '\0';
//test above for speed///////////////////////////////
    return cropped_list;
}

char* check_tile_names_arr(char* tiles_lst, tt_arr_handle* handle, bool set_auto)
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
    char* strt = tiles_lst;             //keeps track of first letter of name on TILES.LST

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
                    return check_tile_names_arr(tiles_lst, handle, set_auto);
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
    cropped_list = make_tile_list_arr(handle, matches);

    free(matches);
    return cropped_list;
}

char* append_tiles_lst_arr(char* tiles_lst_path, tt_arr_handle* handle, bool set_auto)
{
    if (io_file_exists(tiles_lst_path) == false) {return nullptr;}

    int tiles_lst_size = io_file_size(tiles_lst_path);

    //TODO: use io_load_text_file() instead
    //load original TILES.LST into memory
    char* old_tiles_list = (char*)malloc(tiles_lst_size+1);
    FILE* tiles_lst = fopen(tiles_lst_path, "rb");
    if (tiles_lst == nullptr) {
        printf("\n\nUnable to open %s", tiles_lst_path);
        return nullptr;
    }

    int size = fread(old_tiles_list, tiles_lst_size, 1, tiles_lst);
    if (size != 1) {
        printf("\n\nUnable to read entire TILES.LST file?\n\n");
        printf("fread     size: %d\n", size);
    }
    fclose(tiles_lst);
    old_tiles_list[tiles_lst_size] = '\0';      //fread doesn't auto-append null character

    //search old_tiles_list (TILES.LST)
    //for matching names from new_tiles_list
    char* cropped_list = check_tile_names_arr(old_tiles_list, handle, set_auto);
    if (cropped_list == nullptr) {
        return old_tiles_list;
    }

    //TODO: use an io_ function to append text instead
    //append new list_of_tiles to the end of original list
    //in a new buffer large enough to fit both
    int new_lst_size   = strlen(cropped_list);
    char* final_tiles_list = (char*)malloc(tiles_lst_size + new_lst_size +1);   //+1 for null char
    strncpy(final_tiles_list, old_tiles_list, tiles_lst_size);
    final_tiles_list[tiles_lst_size] = '\0';
    strncat(final_tiles_list, cropped_list, new_lst_size);

    //write combined lists out
    io_backup_file(tiles_lst_path, nullptr);
    write_tiles_lst(tiles_lst_path, final_tiles_list);

    free(old_tiles_list);
    return final_tiles_list;
}

void add_TMAP_tiles_to_lst_arr(user_info* usr_nfo, tt_arr_handle* handle, char* save_buff)
{
    char save_path[MAX_PATH];
    strcpy(save_path, usr_nfo->default_save_path);

    //TODO: load TILES.LST directly instead of from memory
    char* tiles_lst = usr_nfo->game_files.FRM_TILES_LST;
    char* game_path = nullptr;
    bool success = false;

    // //Auto option
    // if (usr_nfo->auto_export == true) {
    //     success = auto_export_TMAP_tiles_lst(usr_nfo, save_buff, tiles_lst, *new_tile_list);
    //     if (success == false) {
    //         return;
    //     }
    // }

    success = io_isdir(save_path);
    if (success == false) {
    //TODO: figure out the behavior of tinyfd_notifyPopup()
        char popup_string[MAX_PATH + 24];
        snprintf(popup_string, MAX_PATH+24, "Unable to find %s\n", save_path);
        tinyfd_notifyPopup(
            "Warning...folder does not exist.",
            popup_string,
            "error"
        );
        return;
    }

    strncat(save_path, "/TILES.LST", 11);
    success = io_file_exists(save_path);

    if (success == false) {
        //TILES.LST doesn't exist in selected folder
        int choice = tinyfd_messageBox(
            //TODO: this needs a re-write
            //      maybe change choice to select game folder?
            "Cannot find TILES.LST...",
            "Unable to find TILES.LST in the Fallout 2\n"
            "game directory -- Would you like to make a\n"
            "new one? This new TILES.LST will be blank\n"
            "except for the new tiles made here.\n"
            "--IMPORTANT--\n"
            "The Fallout game engine reads tiles in\n"
            "from TILES.LST based on the line number.\n"
            "The new TILES.LST file will override the\n"
            "game list. Only do this if you want to create\n"
            "the whole tile system from scratch,\n"
            "or to preview the results before manually merging.\n\n"
            "YES:    Create new TILES.LST\n"
            "NO:     Select new folder to save TILES.LST\n",
            "yesnocancel", "warning", 2);
        if (choice == CANCEL) {       // Cancel =  null out buffer and return
            return;
        }
        if (choice == YES) {          // Yes = Create new TILES.LST in this folder
            //blank match_buff so all tiles are added to new_tile_list
            uint8_t* match_buff = (uint8_t*)calloc(1+handle->size/8, 1);
            char* new_tile_list = make_tile_list_arr(handle, match_buff);
            tiles_lst = write_tiles_lst(save_path, new_tile_list);
            free(match_buff);
        }
        if (choice == NO) {           // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
            game_path = tinyfd_selectFolderDialog(
                "Select directory to save to...", usr_nfo->default_save_path);
            strncpy(usr_nfo->default_save_path, game_path, MAX_PATH);
            return add_TMAP_tiles_to_lst_arr(usr_nfo, handle, save_path);
        }
    } else {
        //TILES.LST exists in selected folder
        int choice = tinyfd_messageBox(
            "Warning",
            "Append new tiles to TILES.LST?\n"
            "(A backup will be made.)\n"
            "--IMPORTANT--\n"
            "The Fallout game engine reads tiles in\n"
            "from TILES.LST based on the line number.\n"
            "Be careful not to change the order of\n"
            "tiles once they are on the list.\n\n"
            "YES:    Append new tiles to end of list\n"
            "NO:     Create new TILES.LST?\n"
            "            (a backup will be made.)",
            "yesnocancel", "warning", 2);
        if (choice == CANCEL) {          // Cancel =  null out buffer and return
            return;
        }
        if (choice == YES) {             // Yes = Append to TILES.LST
            tiles_lst = append_tiles_lst_arr(save_path, handle, false);
        }
        if (choice == NO) {              // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
        //backup original file
            io_backup_file(save_path, nullptr);

        //over-write original file
            int num_tiles = 0;
            tt_arr* node = handle->tile;
            // while (node != nullptr) {
            for (int i = 0; i < handle->size; i++)
            {
                if (node[i].tile_id == 1) {
                    continue;
                }
                num_tiles++;
            }
            uint8_t* match_buff = (uint8_t*)calloc(1, 1+num_tiles/8);   //TODO: make this more precise?
            char* new_tile_list = make_tile_list_arr(handle, match_buff);

            tiles_lst = write_tiles_lst(save_path, new_tile_list);
        }
    }

    if (usr_nfo->game_files.FRM_TILES_LST != nullptr) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    // strcpy(usr_nfo->game_files.FRM_TILES_LST, tiles_lst);
    usr_nfo->game_files.FRM_TILES_LST = tiles_lst;
}