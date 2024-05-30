/*
    export tiles
    --GAME   == export to game folder
        --check if default_game_path set
            --if not set, auto_export_question() to set game path
        --ask user if they want to append to TILES.LST?
            --YES: check if TILES.LST can be found
                --if found, append?
                    --if append, do append stuff
                    --if not append, 
                --if not found, create
            --NO : 
    --FOLDER == export to user selected folder
        --export tiles to folder
        --check if TILES.LST can be found
*/






















#include "platform_io.h"
#include "Load_Settings.h"
#include "Save_Files.h"
#include "Edit_TILES_LST.h"
#include "dependencies/tinyfiledialogs/tinyfiledialogs.h"

bool create_tiles_lst(char* file_path);

//read TILES.LST into memory using the game pat
char* load_tiles_lst_game(char* game_path)
{
    //nothing to load if default_game_path not set
    if (game_path == nullptr) {
        return nullptr;
    }
    //check if file exists
    char full_path[MAX_PATH] = {0};
    snprintf(full_path, MAX_PATH, "%s%s", game_path, "/data/art/tiles/TILES.LST");
    if (io_file_exists(full_path) == false) {
        return nullptr;
    }

    //read the entire file into memory and return ptr
    int file_size = io_file_size(full_path);
    char* tiles_lst_buff = (char*)malloc(file_size);

    FILE* tiles_lst = fopen(full_path, "rb");
    fread(tiles_lst_buff, file_size, 1, tiles_lst);
    tiles_lst_buff[file_size] = '\0';
    fclose(tiles_lst);

    return tiles_lst_buff;
}

//this one creates/overwrites TILES.LST
char* write_tiles_lst(char* tiles_lst_path, char* list_of_tiles)
{
    if (tiles_lst_path == nullptr) {return nullptr;}

    // char buffer[MAX_PATH];
    // snprintf(buffer, MAX_PATH, "%s/%s", tiles_lst_path, "TILES.LST");
    // FILE* tiles_lst = fopen(buffer, "wb");
    FILE* tiles_lst = fopen(tiles_lst_path, "wb");
    fwrite(list_of_tiles, strlen(list_of_tiles), 1, tiles_lst);
    fclose(tiles_lst);
    return list_of_tiles;
}

bool io_file_check(char* file_path)
{
        //check if file exists
    //TODO: need to fclose(tiles_lst_ptr); and finish up this section
    // FILE* tiles_lst_ptr = nullptr;
    if (io_file_exists(file_path)) {
        int choice2 = tinyfd_messageBox(
                "Warning",
                "TILES.LST exists...\n"
                "Overwrite?\n\n"
                "--YES:    Overwrite existing file\n"
                "--NO:     Select new folder to save to\n"
                "--Cancel: Cancel\n",
                "yesnocancel", "warning", 2);
        if (choice2 == CANCEL) {        //Cancel: Cancel
            return false;
        }
        if (choice2 == YES) {           //YES:    Overwrite existing file
            return true;// write_tiles_lst(full_path, nullptr);
        }
        if (choice2 == NO) {            //NO:     Select new folder to save to
            char* new_path = tinyfd_selectFolderDialog("Select save folder...", file_path);
            strncpy(file_path, new_path, MAX_PATH);
            if (io_path_check(file_path)) {
                return create_tiles_lst(file_path);
            }
        }
    }
    return true;
}

//TODO: this might need to return something more detailed
//      or I need to change the general structure
bool create_tiles_lst(char* file_path)
{
    char full_path[MAX_PATH];
    snprintf(full_path, MAX_PATH, "%s/%s", file_path, "TILES.LST");

    bool success = io_path_check(file_path);
    if (success == false) {return false;}
    success = io_file_check(full_path);
    if (success == false) {return false;}

    write_tiles_lst(full_path, "");
    return true;
}

struct tile_name {
    char* name_ptr;
    int length;
    tile_name* next;
};

tile_name* make_name_list(char* tiles_list)
{
    char* strt = tiles_list;
    char* end  = tiles_list;
    tile_name* start_tile = (tile_name*)malloc(sizeof(tile_name));
    tile_name* lst_tile = start_tile;
    tile_name* new_tile = nullptr;

    for (int i = 0; i < strlen(tiles_list); i++)
    {
        if (*(strt + i) == '\n') {
            end = strt + i;

            if (lst_tile == start_tile) {
                new_tile = start_tile;
            } else {
                new_tile = (tile_name*)malloc(sizeof(tile_name));
            }
            new_tile->length   = end - strt;
            new_tile->name_ptr = strt;

            lst_tile->next = new_tile;
            lst_tile       = new_tile;
        }
    }
    return start_tile;
}

void free_tile_name_lst(tile_name* list)
{
    tile_name* current = list;
    tile_name* next    = list->next;
    while (current != nullptr) {
        next = current->next;
        free(current);
        current = next;
    }
}

//compare names on tiles_lst to names on new_tiles
//increments by '\n' to check each name individually
//TODO: change to linked list data type?
bool check_tile_names(char* tiles_lst, char* new_tiles)
{
    bool append_only_new = false;
    int new_name_len = 0;
    int match = 0;
    char* t_str_ptr = tiles_lst;
    char* t_end_ptr = tiles_lst;
    char* new_ptr   = new_tiles;

    for (int i = 0; i < strlen(new_tiles); i++)
    {
        if (*(new_tiles + i) == '\n') {
            new_name_len = i;
            break;
        }
    }

    char msg_buff[MAX_PATH] = {
        "One of the new tile-names matches\n"
        "a tile-name already on TILES.LST.\n\n"
    };

    // while (new_ptr < (new_tiles + strlen(new_tiles))) {
    for (char* new_ptr = new_tiles; new_ptr < new_tiles + strlen(new_tiles);) {
        for (int i = 0; i < strlen(tiles_lst); i++) {

            if (*(tiles_lst + i) == '\n') {
                t_end_ptr = tiles_lst + i;
                if (*t_str_ptr == *new_ptr) {
                    match = strncmp(t_str_ptr, new_ptr, t_end_ptr - t_str_ptr);
                    if (match == 0) {
                        strncat(msg_buff, t_str_ptr, t_end_ptr - t_str_ptr+1);
                        strncat(msg_buff, 
                            "--YES:   Skip and append only new names?\n"
                            "--NO:    Rename the new tiles?", 72
                        );
                        int choice = tinyfd_messageBox(
                            "Match found...",
                            msg_buff,
                            "yesnocancel", "warning", 2);
                        if (choice == CANCEL) {
                            //TODO: maybe change this function
                            //      to return a char* w/the name?
                            //      or nullptr for cancel?
                            return;
                        }
                        if (choice == YES) {
                            append_only_new = true;
                        }
                        if (choice == NO) {

                        }








                        return true;
                    }
                }
                t_str_ptr = t_end_ptr + 1;
            }
        }
        new_ptr += new_name_len;
    }
    return false;
}

//TODO: this one appends new tiles to the end of TILES.LST
char* append_tiles_lst(char* tiles_lst_path, char* list_of_tiles)
{
    if (tiles_lst_path == nullptr) {return nullptr;}
    // char full_path[MAX_PATH];
    // snprintf(full_path, MAX_PATH, "%s/%s", tiles_lst_path, "TILES.LST");

    int tiles_lst_size = io_file_size(tiles_lst_path);
    int new_lst_size   = strlen(list_of_tiles);

    //load TILES.LST into memory
    char* final_tiles_list = (char*)malloc(tiles_lst_size + new_lst_size);
    FILE* tiles_lst = fopen(tiles_lst_path, "rb");
    fread(final_tiles_list, tiles_lst_size, 1, tiles_lst);
    fclose(tiles_lst);

    // char* final_tiles_list = load_tiles_lst(full_path);
    // char* temp = load_tiles_lst(full_path);
    // strncpy(final_tiles_list, temp, tiles_lst_size);
    // free(temp);

    //search final_tiles_list (TILES.LST)
    //for matching names from list_of_tiles
    if (check_tile_names(final_tiles_list, list_of_tiles) == true) {

        
    }


    //append new list_of_tiles to the end of original list
    char* list_ptr = final_tiles_list + tiles_lst_size;
    if (list_ptr != final_tiles_list) {
        if (*list_ptr == '\0') {
            *list_ptr = '\n';
            list_ptr++;
        }
    }

    strncpy(list_ptr, list_of_tiles, new_lst_size);
    list_ptr += new_lst_size;
    *list_ptr = '\0';

    //write combined lists out
    // FILE* tiles_lst = fopen(full_path, "wb");
    // fwrite(final_tiles_list, tiles_lst_size + new_lst_size, 1, tiles_lst);
    // fclose(tiles_lst);
    write_tiles_lst(tiles_lst_path, final_tiles_list);

    // free(final_tiles_list);
    return final_tiles_list;
}

bool auto_export_TMAP_tiles_lst(user_info* usr_nfo, char* save_buff, char* tiles_lst, char* new_tile_list)
{
    bool success = false;
    //if default_game_path not set
    if (usr_nfo->default_game_path[0] == '\0') {
        success = auto_export_question(usr_nfo, usr_nfo->exe_directory, save_buff, TILE);
        if (success == false) {
            return false;
        }
    }
    //if TILES_LST isn't already loaded
    //check if file exists and load if it does
    if (tiles_lst == nullptr) {
        tiles_lst = load_tiles_lst_game(usr_nfo->default_game_path);
    }
    //if TILES_LST can't be found
    if (tiles_lst == nullptr) {
        //TODO: notify user that can't find TILES.LST
        int choice = tinyfd_messageBox(
                    "Cant find TILES.LST...",
                    "TILES.LST is used to identify the town map tiles\n"
                    "used in the game."

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

                    "If you want to add to the vanilla game tiles,\n"
                    "extract TILES.LST from master.dat into the\n"
                    "Fallout 2 game folder, which should place it in\n"
                    "/Fallout 2/data/art/tiles/TILES.LST\n"
                    "Then select -NO- and the program will re-check the file.\n\n"

                    "--YES:    Create new TILES.LST in /Fallout 2/data/art/tiles/\n"
                    "--NO:     Re-check game folder\n",
                    "yesnocancel", "warning", 2);

        if (choice == CANCEL) {
            return false;
        }

        if (choice == NO) {         //re-check folder
            return auto_export_TMAP_tiles_lst(usr_nfo, save_buff, tiles_lst, new_tile_list);
        }

        if (choice == YES) {        //create TILES.LST
            snprintf(save_buff, MAX_PATH, "%s/%s", usr_nfo->default_game_path, "TILES.LST");
            tiles_lst = write_tiles_lst(save_buff, new_tile_list);
            usr_nfo->game_files.TILES_LST = tiles_lst;
            return false;
        }
    } else {
        return true;
    }
}

//append names of new tiles to end of TILES.LST
//or create new TILES.LST with only these new tiles
void add_TMAP_tiles_to_lst(user_info* usr_nfo, char* new_tile_list, char* save_buff)
{
    char* tiles_lst = usr_nfo->game_files.TILES_LST;
    char* game_path = nullptr;
    bool success = false;

    //Auto option
    if (usr_nfo->auto_export == true) {
        success = auto_export_TMAP_tiles_lst(usr_nfo, save_buff, tiles_lst, new_tile_list);
        if (success == false) {
            return;
        }
    }

    char popup_string[MAX_PATH + 24];
    snprintf(popup_string, MAX_PATH+24, "Unable to find %s\n", save_buff);
    success = io_isdir(save_buff);
    if (success == false) {
    //TODO: figure out the behavior of this function
        tinyfd_notifyPopup(
            "Warning...folder does not exist.",
            popup_string,
            "error"
        );
        return;
    }

    // snprintf(save_buff, MAX_PATH, "%s/%s", save_buff, "TILES.LST");
    strncat(save_buff + strlen(save_buff), "/TILES.LST", 11);
    success = io_file_exists(save_buff);

    if (success == false) {
        //TILES.LST doesn't exist in selected folder
        int choice = tinyfd_messageBox(
            //TODO: this needs a re-write
            //      maybe change choice to select game folder
            "Cant find TILES.LST...",
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
            "--YES:    Create new TILES.LST\n"
            "--NO:     Select new folder to save TILES.LST\n"
            "--CANCEL: Cancel...do I need to explain?\n",
            "yesnocancel", "warning", 2);
        if (choice == CANCEL) {       // Cancel =  null out buffer and return
            return;
        }
        if (choice == YES) {          // Yes = Create new TILES.LST in this folder
            tiles_lst = write_tiles_lst(save_buff, new_tile_list);
        }
        if (choice == NO) {           // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
            game_path = tinyfd_selectFolderDialog(
                "Select directory to save to...", usr_nfo->default_save_path);
            strncpy(usr_nfo->default_save_path, game_path, MAX_PATH);
            // snprintf(save_buff, MAX_PATH, "%s/%s", usr_nfo->default_save_path, "TILES.LST");
            // tiles_lst = write_tiles_lst(save_buff, new_tile_list);
            return add_TMAP_tiles_to_lst(usr_nfo, new_tile_list, save_buff);
        }
    } else {
        //TILES.LST exists in selected folder
        int choice = tinyfd_messageBox(
            "Warning",
            "Append new tiles to TILES.LST?\n"
            "--IMPORTANT--\n"
            "The Fallout game engine reads tiles in\n"
            "from TILES.LST based on the line number.\n"
            "Be careful not to change the order of\n"
            "tiles once they're on the list.\n\n"
            "--YES:    Append new tiles to end of list\n"
            "--NO:     Create new TILES.LST?\n"
            "--CANCEL: Why am I writing this one out?\n",
            "yesnocancel", "warning", 2);
        if (choice == CANCEL) {          // Cancel =  null out buffer and return
            return;
        }
        if (choice == YES) {             // Yes = Append to TILES.LST
            //TODO: this could be cleaned up to use usr_nfo.game_files.TILES_LST
            tiles_lst = append_tiles_lst(save_buff, new_tile_list);
        }
        if (choice == NO) {              // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
            game_path = tinyfd_selectFolderDialog(
                "Select directory to save to...", usr_nfo->default_save_path);
            strncpy(usr_nfo->default_save_path, game_path, MAX_PATH);
            // snprintf(save_buff, MAX_PATH, "%s/%s", usr_nfo->default_save_path, "TILES.LST");
            // tiles_lst = write_tiles_lst(save_buff, new_tile_list);
            return add_TMAP_tiles_to_lst(usr_nfo, new_tile_list, save_buff);
        }
    }

    if (usr_nfo->game_files.TILES_LST != nullptr) {
        free(usr_nfo->game_files.TILES_LST);
    }
    usr_nfo->game_files.TILES_LST = tiles_lst;
///////////everything below here needs to change///////////////////////////////////
    // } else {
    //     //the file was found and already loaded into memory?
    //     //append to existing list in /Fallout 2/data/art/tiles/TILES.LST
    //     //this might not be the best way to do this
    //     //do I need to have a whole popup branching system?
    //     int len_lst = strlen(usr_nfo->game_files.TILES_LST);
    //     int len_new = strlen(new_tile_list);

    //     char tiles_lst[len_lst+len_new] = {0};
    //     snprintf(save_buff, MAX_PATH, "%s/%s", usr_nfo->default_game_path, "TILES.LST");
    //     strncpy(tiles_lst, usr_nfo->game_files.TILES_LST, len_lst);
    //     strncpy(tiles_lst+len_lst, new_tile_list, len_new);
    //     write_tiles_lst(save_buff, tiles_lst);
    // }
///////////everything above here needs to change///////////////////////////////////
}