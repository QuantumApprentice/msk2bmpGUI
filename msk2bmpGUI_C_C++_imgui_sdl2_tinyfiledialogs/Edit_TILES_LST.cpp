// #include <stdio.h>
#include <time.h>

#include "platform_io.h"
#include "Load_Settings.h"
#include "Save_Files.h"
#include "Edit_TILES_LST.h"
#include "dependencies/tinyfiledialogs/tinyfiledialogs.h"

bool create_tiles_lst(char* file_path);

//malloc new char* and populate with generated names
char* generate_new_tile_list(char* name, int tile_num)
{
    int tile_name_len = strlen(name) + strlen("%03d.FRM\n");
    int new_tile_list_size = (tile_num * tile_name_len);
    char* new_tile_list = (char*)malloc(new_tile_list_size);
    char* list_ptr = new_tile_list;

    char buffer[tile_name_len+1] = {0};
    for (int i = 0; i < tile_num; i++) {
        snprintf(buffer, tile_name_len, "%s%03d.FRM\n", name, i);
        strncpy(list_ptr, buffer, tile_name_len);
        list_ptr += tile_name_len-1;
    }
    return new_tile_list;
}

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
            //TODO: this needs checking and maybe cleaning up
            if (io_path_check(file_path)) {
                return create_tiles_lst(file_path);
            }
        }
    }
    return true;
}

//TODO: this might need to return something more detailed
//      or I need to change the general structure
//TODO: delete
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
        return nullptr;
    }
    return name;
}

//testing array list version/////////////////////////////////////////////////////start
struct tile_name_arr {
    char*    name_ptr;
    uint32_t length;
    uint32_t next;           //points to array index of next viable name
};

tile_name_arr* make_name_list_arr(char* new_tiles_list)
{
    tile_name_arr* large_list = (tile_name_arr*)malloc(sizeof(tile_name_arr) * 10000);
    int node = 0;
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

int skip_or_rename_arr(tile_name_arr* node)
{
    char msg_buff[MAX_PATH] = {
        "One of the new tile-names matches\n"
        "a tile-name already on TILES.LST.\n\n"
    };
    strncat(msg_buff, node->name_ptr, node->length);
    strncat(msg_buff, "\n"
        "--YES:   Skip and append only new names?\n"
        "--NO:    Rename the new tiles?\n", 74);

    int choice = tinyfd_messageBox(
                "Match found...",
                msg_buff,
                "yesnocancel", "warning", 2);
    return choice;
}

int skip_this_node(tile_name_arr* node, int index, int head)
{
    if (index == head) {
        return index + 1;
    }
    node[index-1].next++;
    return head;
}


//check tile-names but convert new_tiles to linked list array first
bool check_tile_names_ll_arr(char* tiles_lst, char** new_tiles)
{
    bool append_new_only = false;
    int tiles_lst_len = strlen(tiles_lst);
    tile_name_arr* linked_lst = make_name_list_arr(*new_tiles);  //need to free linked_lst at the end
    tile_name_arr* node = linked_lst;
    tile_name_arr* prev = nullptr;
    char* name_strt = tiles_lst;             //keeps track of first letter for each name in TILES.LST

    char* str_ptr = tiles_lst;
    int i = 0;
    int head_index = 0;
    while (node[i].next != 0) {
        // for (int i = 0; i < tiles_lst_len; i++)
        while (*str_ptr != '\0')
        {
            if (*str_ptr != '\n') {
                continue;
            }
            //check first char of strt == first char of node.name_ptr
            if (name_strt[0] != node[i].name_ptr[0]) {
                name_strt = str_ptr + 1;
                continue;
            }
            //full string compare if we got this far
            if (strncmp(name_strt, node[i].name_ptr, node[i].length) != 0) {
                name_strt = str_ptr + 1;
                continue;
            }
            //match found, ask what to do
            if (append_new_only != true) {
                int choice = skip_or_rename_arr(&node[i]);
                if (choice == CANCEL) {     //return and cancel out of the whole thing
                    // free_tile_name_lst(linked_lst);
                    free(linked_lst);
                    return false;
                }
                if (choice == YES)    {     //just append names not already on TILES.LST
                    append_new_only = true;
                }
                if (choice == NO)     {     //pick a new name and re-make new_tiles then re-check
                    char old_name[node[i].length+1];
                    strncpy(old_name, node[i].name_ptr, node[i].length);
                    old_name[node[i].length-7] = '\0';

                    char* new_name = get_new_name(old_name);
                    int num_tiles = strlen(*new_tiles) / node[i].length;

                    free(*new_tiles);
                    *new_tiles = generate_new_tile_list(new_name, num_tiles);

                    // free_tile_name_lst(linked_lst);
                    free(linked_lst);
                    return check_tile_names_ll_arr(tiles_lst, new_tiles);
                }
            }
            if (append_new_only == true) {
                //remove node from list
                // free_tile_name_node(node, prev);
                head_index = skip_this_node(node, i, head_index);
            }
        }
        i++;
    }

    //generate new list from remaining nodes in linked_lst
    if (append_new_only == true) {
        node = linked_lst;
        int num_tiles = 0;
        int index = head_index;
        while (node[index].next != 0) {
            num_tiles++;
            index = node[index].next;
            // node = node->next;
        }

        index = head_index;
        char* cropped_list = (char*)malloc(node->length * num_tiles);
        while (node[index].next != 0) {
            strncat(cropped_list, node[index].name_ptr, node[index].length);
            // node = node[index].next;
            index = node[index].next;
        }
        free(*new_tiles);
        //this might be wrong
        *new_tiles = cropped_list;
    }

    // free_tile_name_lst(linked_lst);
    free(linked_lst);
    return true;
}

//testing array list version/////////////////////////////////////////////////////end

//testing this linked list struct version////////////////////////////////////////start
struct tile_name {
    char* name_ptr;
    int length;
    tile_name* next;
};

//create a linked list of tile names
//from the char* tiles_list passed in
tile_name* make_name_list(char* tiles_list)
{
    char* strt = tiles_list;
    char* end  = tiles_list;
    tile_name* strt_tile = nullptr; // = (tile_name*)malloc(sizeof(tile_name));
    tile_name* prev_tile = nullptr;
    tile_name* newt_tile = nullptr;
    int tiles_list_len = strlen(tiles_list);

    for (int i = 0; i < tiles_list_len; i++)
    {
        if (tiles_list[i] == '\n') {
            end = &tiles_list[i];

            newt_tile = (tile_name*)malloc(sizeof(tile_name));

            newt_tile->length   = end - strt;
            newt_tile->name_ptr = strt;
            newt_tile->next = nullptr;

            if (prev_tile == nullptr) {
                strt_tile = newt_tile;
            } else {
                prev_tile->next = newt_tile;
            }
            prev_tile = newt_tile;
            strt = &tiles_list[i+1];
        }
    }
    return strt_tile;
}

//mem-free all tiles in a linked list
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

//remove & free matching node
tile_name* free_tile_name_node(tile_name* node, tile_name* prev, tile_name** head)
{
    tile_name* tmp = node;
    if (prev == nullptr) {
        node = node->next;
        *head = node;
    } else {
        prev->next = node->next;
        node = node->next;
    }
    free(tmp);
    return node;
}
//testing this linked list struct version////////////////////////////////////////end

int skip_or_rename(tile_name* node)
{
    char msg_buff[MAX_PATH] = {
        "One of the new tile-names matches\n"
        "a tile-name already on TILES.LST.\n\n"
    };
    strncat(msg_buff, node->name_ptr, node->length);
    strncat(msg_buff, "\n\n"
        "--YES:   Skip and append only new names?\n"
        "--NO:    Rename the new tiles?\n", 75);

    int choice = tinyfd_messageBox(
                "Match found...",
                msg_buff,
                "yesnocancel", "warning", 2);
    return choice;
}


//check tile-names but convert new_tiles to linked list first
bool check_tile_names_ll(char* tiles_lst, char** new_tiles)
{
    bool append_new_only = false;
    int tiles_lst_len = strlen(tiles_lst);
    tile_name* linked_lst = make_name_list(*new_tiles);  //need to free linked_lst at the end
    tile_name* node = linked_lst;
    tile_name* prev = nullptr;
    // tile_name* next = nullptr;
    char* strt = tiles_lst;             //keeps track of first letter of name on TILES.LST

    // char buff_a[tiles_lst_len/2];
    // char buff_b[tiles_lst_len/2];
    // char*buff_ptr = buff_a;

    // strncpy(buff_a, tiles_lst, tiles_lst_len/2);
    // strncpy(buff_b, tiles_lst+(tiles_lst_len/2), tiles_lst_len/2);

    // while (buff_ptr != nullptr) {
        while (node != nullptr) {
            for (int i = 0; i < tiles_lst_len; i++)
            {
                if (tiles_lst[i] != '\n' || tiles_lst[i] == '\0') {
                    continue;
                }
                //check first char of strt == first char of node.name_ptr
                if (strt[0] != node->name_ptr[0]) {
                    strt = &tiles_lst[i+1];
                    continue;
                }
                if (strncmp(strt, node->name_ptr, node->length) != 0) {
                    strt = &tiles_lst[i+1];
                    continue;
                }
                //match found, ask what to do
                if (append_new_only != true) {
                    int choice = skip_or_rename(node);
                    if (choice == CANCEL) {     //return and cancel out of the whole thing
                        free_tile_name_lst(linked_lst);
                        return false;
                    }
                    if (choice == YES)    {     //just append names not already on TILES.LST
                        append_new_only = true;
                    }
                    if (choice == NO)     {     //pick a new name and re-make new_tiles then re-check
                        char old_name[node->length+1];
                        strncpy(old_name, node->name_ptr, node->length);
                        old_name[node->length-7] = '\0';

                        char* new_name = get_new_name(old_name);
                        int num_tiles = strlen(*new_tiles) / node->length;

                        free(*new_tiles);
                        *new_tiles = generate_new_tile_list(new_name, num_tiles);

                        free_tile_name_lst(linked_lst);
                        return check_tile_names_ll(tiles_lst, new_tiles);
                    }
                }
                if (append_new_only == true) {
                    //remove node from list
                    node = free_tile_name_node(node, prev, &linked_lst);
                    strt = &tiles_lst[i+1];
                }
            }
            prev = node;
            node = node->next;
        }
        // if (buff_ptr == buff_a) {
        //     buff_ptr = buff_b;
        // } else {
        //     buff_ptr = nullptr;
        // }
    // }

    //generate new list from remaining nodes in linked_lst
    if (append_new_only == true) {
        node = linked_lst;
        int num_tiles = 0;
        int total_size = 0;
        while (node != nullptr) {
            num_tiles++;
            total_size += node->length;
            node = node->next;
        }

        char* cropped_list = (char*)malloc(total_size);
        cropped_list[0] = '\0';
        node = linked_lst;
        while (node != nullptr) {
            strncat(cropped_list, node->name_ptr, node->length+1);
            node = node->next;
        }
        free(*new_tiles);
        //this might be wrong
        *new_tiles = cropped_list;
    }

    free_tile_name_lst(linked_lst);
    return true;
}

//compare names on tiles_lst to names on new_tiles
//increments by '\n' to check each name individually
//TODO: change to linked list data type?
//TODO: delete? not sure if I'm going to use this
bool check_tile_names(char* tiles_lst, char* new_tiles)
{
    bool append_only_new = false;
    int match = 0;
    char* t_str_ptr = tiles_lst;
    char* t_end_ptr = tiles_lst;
    char* new_ptr   = new_tiles;

    int new_name_len = 0;
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
                            "--NO:    Rename the new tiles?", 72);
                        int choice = tinyfd_messageBox(
                            "Match found...",
                            msg_buff,
                            "yesnocancel", "warning", 2);
                        if (choice == CANCEL) {
                            //TODO: maybe change this function
                            //      to return a char* w/the name?
                            //      or nullptr for cancel?
                            return false;
                        }
                        if (choice == YES) {
                            append_only_new = true;
                        }
                        if (choice == NO) {

                            return true;        //?
                        }
                    }
                }
                t_str_ptr = t_end_ptr + 1;
            }
        }
        new_ptr += new_name_len;
    }
    return true;
}

//TODO: this one appends new tiles to the end of TILES.LST
char* append_tiles_lst(char* tiles_lst_path, char** new_tiles_list)
{

    if (tiles_lst_path == nullptr) {return nullptr;}
    // char full_path[MAX_PATH];
    // snprintf(full_path, MAX_PATH, "%s/%s", tiles_lst_path, "TILES.LST");

    int tiles_lst_size = io_file_size(tiles_lst_path);

    //load TILES.LST into memory
    char* old_tiles_list = (char*)malloc(tiles_lst_size+1);
    FILE* tiles_lst = fopen(tiles_lst_path, "rb");
    int size = fread(old_tiles_list, tiles_lst_size, 1, tiles_lst);
    if (size < tiles_lst_size) {
        printf("\n\nUnable to read entire TILES.LST file?\n\n");
    }
    fclose(tiles_lst);

    //search final_tiles_list (TILES.LST)
    //for matching names from new_tiles_list
    bool success = check_tile_names_ll(old_tiles_list, new_tiles_list);
    if (success == false) {
        return nullptr;
    }

    char time_buff[32];
    char rename_buff[MAX_PATH];
    time_t t = time(NULL);
    tm* tp = localtime(&t);
    strftime(time_buff, 32, "_%Y%m%d_%H%M%S", tp);
    snprintf(rename_buff, MAX_PATH, "%s%s%s", tiles_lst_path, time_buff, ".LST");

    int error = rename(tiles_lst_path, rename_buff);
    // if (error != 0) {
        perror("Error renaming TILES.LST: ");
    // }

    //append new list_of_tiles to the end of original list
    //in a new buffer large enough to fit both
    int new_lst_size   = strlen(*new_tiles_list);
    char* final_tiles_list = (char*)malloc(tiles_lst_size + new_lst_size);
    strncpy(final_tiles_list, old_tiles_list, tiles_lst_size);
    if (final_tiles_list[tiles_lst_size] == '\n') {
        final_tiles_list[tiles_lst_size+1] = '\0';
    } else {
        final_tiles_list[tiles_lst_size] = '\0';
    }
    strncat(final_tiles_list, *new_tiles_list, new_lst_size);

    //write combined lists out
    write_tiles_lst(tiles_lst_path, final_tiles_list);

    free(old_tiles_list);
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
                    "used in the game.\n"

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
void add_TMAP_tiles_to_lst(user_info* usr_nfo, char** new_tile_list, char* save_buff)
{
    char* tiles_lst = usr_nfo->game_files.TILES_LST;
    char* game_path = nullptr;
    bool success = false;

    //Auto option
    if (usr_nfo->auto_export == true) {
        success = auto_export_TMAP_tiles_lst(usr_nfo, save_buff, tiles_lst, *new_tile_list);
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
            "--NO:     Select new folder to save TILES.LST\n",
            "yesnocancel", "warning", 2);
        if (choice == CANCEL) {       // Cancel =  null out buffer and return
            return;
        }
        if (choice == YES) {          // Yes = Create new TILES.LST in this folder
            tiles_lst = write_tiles_lst(save_buff, *new_tile_list);
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
            "tiles once they are on the list.\n\n"
            "--YES:    Append new tiles to end of list\n"
            "--NO:     Create new TILES.LST?\n",
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