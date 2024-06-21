#include <time.h>

#include "platform_io.h"
#include "Load_Settings.h"
#include "Save_Files.h"
#include "Edit_TILES_LST.h"
#include "tinyfiledialogs.h"


//malloc town_tile* linked list and populate with generated tile-names
//return ptr (need to free elsewhere)
//_tt stands for town_tile*
town_tile* generate_new_tile_list_tt(char* name, town_tile* tile_head)
{
    int tile_name_len = strlen(name) + strlen("%03d.FRM\r\n");

    town_tile* ptr = tile_head;

    while (ptr != nullptr) {
        char buff[8];
        strncpy(buff, ptr->name_ptr + ptr->length - 8, 8);

        free(ptr->name_ptr);
        ptr->name_ptr = (char*)malloc(tile_name_len);
        snprintf(ptr->name_ptr, tile_name_len, "%s%s", name, buff);
        ptr->length = tile_name_len;

        ptr = ptr->next;
    }

    return tile_head;
}
//malloc char* and populate with generated tile-names
//return ptr (need to free elsewhere)
char* generate_new_tile_list(char* name, int tile_num)
{
    int tile_name_len = strlen(name) + strlen("%03d.FRM\r\n");
    int new_tile_list_size = (tile_num * tile_name_len);
    char* new_tile_list = (char*)malloc(new_tile_list_size);
    char* list_ptr = new_tile_list;

    char buffer[tile_name_len+1] = {0};
    for (int i = 0; i < tile_num; i++) {
        snprintf(buffer, tile_name_len, "%s%03d.FRM\r\n", name, i);
        strncpy(list_ptr, buffer, tile_name_len);
        list_ptr += tile_name_len-1;
    }
    return new_tile_list;
}

char* load_text_file(char* full_path)
{
    if (io_file_exists(full_path) == false) {
        return nullptr;
    }

    //read the entire file into memory and return ptr
    int file_size = io_file_size(full_path);
    char* text_file_buff = (char*)malloc(file_size);

    FILE* tiles_lst = fopen(full_path, "rb");
    fread(text_file_buff, file_size, 1, tiles_lst);
    text_file_buff[file_size] = '\0';
    fclose(tiles_lst);

    return text_file_buff;
}

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

    return load_text_file(full_path);
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

    return load_text_file(full_path);
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

#if false
// //TODO: delete this
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
            return true;
        }
        if (choice2 == NO) {            //NO:     Select new folder to save to
            char* new_path = tinyfd_selectFolderDialog("Select save folder...", file_path);
            strncpy(file_path, new_path, MAX_PATH);
            if (io_path_check(file_path)) {
                // return create_tiles_lst(file_path);
            }
        }
    }
    return true;
}
#endif

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

//since I can't delete the node like in a linked list
//just skip it by pointing the previous node to the next
//if it's the head, then move the head index to the next node
int skip_this_node(tile_name_arr* node, int index, int head)
{
    if (index == head) {
        return index + 1;
    }
    node[index-1].next++;
    return head;
}

//compare names on tiles_lst to names on new_tiles
//but convert new_tiles to linked list array first
bool check_tile_names_ll_arr(char* tiles_lst, char** new_tiles)
{
    bool append_new_only = false;
    tile_name_arr* linked_lst = make_name_list_arr(*new_tiles);  //need to free linked_lst at the end
    tile_name_arr* node = linked_lst;
    tile_name_arr* prev = nullptr;
    char* name_strt = tiles_lst;             //keeps track of first letter for each name in TILES.LST

    char* str_ptr = tiles_lst;
    int i = 0;
    int head_index = 0;
    while (node[i].next != 0) {
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

                    free(linked_lst);
                    return check_tile_names_ll_arr(tiles_lst, new_tiles);
                }
            }
            if (append_new_only == true) {
                //remove node from index list
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
        }

        index = head_index;
        char* cropped_list = (char*)malloc(node->length * num_tiles);
        while (node[index].next != 0) {
            strncat(cropped_list, node[index].name_ptr, node[index].length);
            index = node[index].next;
        }
        free(*new_tiles);
        //this might be wrong
        *new_tiles = cropped_list;
    }

    free(linked_lst);
    return true;
}

//testing array list version/////////////////////////////////////////////////////end

//testing this linked list struct version////////////////////////////////////////start

//create a linked list of tile names
//from the char* tiles_list passed in
tile_name* make_name_list(char* tiles_list)
{
    char* strt = tiles_list;
    char* end  = tiles_list;
    tile_name* strt_tile = nullptr;
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

//mem-free all tiles in town_tile* linked list
void free_tile_name_lst_tt(town_tile* list)
{
    town_tile* current = list;
    town_tile* next    = list->next;
    while (current != nullptr) {
        next = current->next;
        free(current);
        current = next;
    }
}

#if false
//mem-free all tiles in tile_name* linked list
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
#endif

//remove & free matching node
town_tile* free_town_tile_node_tt(town_tile* node, town_tile* prev, town_tile** head)
{
    town_tile* tmp = node;
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

//remove & free matching node
// tile_name* free_tile_name_node(tile_name* node, tile_name* prev, tile_name** head)
// {
//     tile_name* tmp = node;
//     if (prev == nullptr) {
//         node = node->next;
//         *head = node;
//     } else {
//         prev->next = node->next;
//         node = node->next;
//     }
//     free(tmp);
//     return node;
// }
//testing this linked list struct version////////////////////////////////////////end

int skip_or_rename_tt(town_tile* node)
{
    char msg_buff[MAX_PATH] = {
        "One of the new tile-names matches\n"
        "a tile-name already on TILES.LST.\n\n"
    };
    strncat(msg_buff, node->name_ptr, node->length);
    strncat(msg_buff, "\n\n"
        "YES:   Skip and append only new names?\n"
        "NO:    Rename the new tiles?\n", 75);

    int choice = tinyfd_messageBox(
                "Match found...",
                msg_buff,
                "yesnocancel", "warning", 2);
    return choice;
}

#if false
// int skip_or_rename(tile_name* node)
// {
//     char msg_buff[MAX_PATH] = {
//         "One of the new tile-names matches\n"
//         "a tile-name already on TILES.LST.\n\n"
//     };
//     strncat(msg_buff, node->name_ptr, node->length);
//     strncat(msg_buff, "\n\n"
//         "YES:   Skip and append only new names?\n"
//         "NO:    Rename the new tiles?\n", 75);
//     int choice = tinyfd_messageBox(
//                 "Match found...",
//                 msg_buff,
//                 "yesnocancel", "warning", 2);
//     return choice;
// }
#endif

//_tt stands for town_tile*
char* make_tile_list_tt(town_tile* head, uint8_t* match_buff)
{
    //total_size = total length of all names
    town_tile* node = head;
    uint8_t shift_ctr  = 0;
    int     tile_num   = 0;
    int     total_size = 0;
    while (node != nullptr) {
        int indx = tile_num/8;
        uint8_t shift = 1 << shift_ctr;
        if (!(match_buff[indx] & shift)) {
            total_size += node->length+1;
        }
            shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        } else {
        }
        tile_num++;
        node = node->next;
    }

    //if there are no nodes (or none with viable names)
    if (total_size < 1) {
        tinyfd_notifyPopup("TILES.LST not updated...",
                        "All new tile-names were already\n"
                        "found on TILES.LST.\n"
                        "No new tile-names were added.\n",
                        "info");
        //TODO: test this free()
        // free_tile_name_lst(linked_lst);
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

    char* cropped_list = (char*)malloc(total_size+1);
    node       = head;
    shift_ctr  = 0;
    tile_num   = 0;
    char* c    = cropped_list;
    while (node != nullptr)
    {
        if (!(match_buff[tile_num/8] & 1 << shift_ctr)) {
            size_t amount_to_copy = node->length+1;
            memcpy(c, node->name_ptr, amount_to_copy);
            *(c + node->length)   = '\r';
            *(c + node->length+1) = '\n';
            c += amount_to_copy+1;
        }

            shift_ctr++;
        if (shift_ctr >= 8) {
            shift_ctr = 0;
        } else {
        }
        tile_num++;
        node = node->next;
    }
    c[0] = '\0';
//test above for speed///////////////////////////////
    return cropped_list;
}

//compare names on tiles_lst to names on new_tiles
//but convert new_tiles to town_tile* linked list first
char* check_tile_names_ll_tt(char* tiles_lst, town_tile* new_tiles, bool set_auto)
{
    bool append_new_only = set_auto;
    int num_tiles = 0;
    int tiles_lst_len = strlen(tiles_lst);
    town_tile* node = new_tiles;
    while (node != nullptr)
    {
        num_tiles++;
        node = node->next;
    }
    node = new_tiles;       //reset the node for next steps

    //TODO: malloc matches
    uint8_t matches[1+num_tiles/8] = {0};    //TODO: should be able to make this more precise
    uint8_t shift_ctr = 0;
    int     node_ctr  = 0;

    town_tile* prev = nullptr;
    char* strt = tiles_lst;             //keeps track of first letter of name on TILES.LST

    while (node != nullptr) {
        for (int char_ctr = 0; char_ctr < tiles_lst_len; char_ctr++)
        {
            if (tiles_lst[char_ctr] != '\n' && tiles_lst[char_ctr] != '\0') {
                continue;
            }
            //check first char of strt == first char of node.name_ptr
            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                strt = &tiles_lst[char_ctr+1];
                continue;
            }
            if (strncmp(strt, node->name_ptr, node->length) != 0) {
                strt = &tiles_lst[char_ctr+1];
                continue;
            }
            //first match found, ask what to do
            if (append_new_only == false) {
                int choice = skip_or_rename_tt(node);
                if (choice == CANCEL) {     //return and cancel out of the whole thing
                    free_tile_name_lst_tt(new_tiles);
                    return nullptr;
                }
                if (choice == YES)    {     //just append names not already on TILES.LST
                    append_new_only = true;
                }
                if (choice == NO)     {     //pick a new name and re-make new_tiles then re-check
                    char old_name[node->length+1];
                    strncpy(old_name, node->name_ptr, node->length);
                    old_name[node->length-7] = '\0';

                    char* new_name = get_new_name(old_name);
                    new_tiles = generate_new_tile_list_tt(new_name, new_tiles);

                    return check_tile_names_ll_tt(tiles_lst, new_tiles, set_auto);
                }
            }
            if (append_new_only == true) {
                int ctr = node_ctr/8;
                int shift = 1 << shift_ctr;
                //identify this node as having a duplicate match
                // matches[node_ctr/8] |= 1 << shift_ctr;
                matches[ctr] |= shift;
                //increment all the counters
                shift_ctr++;
                if (shift_ctr >= 8) {
                    shift_ctr = 0;
                } else {
                }
                node_ctr++;
                // char_ctr = 0;

                //TODO: i have the distinct impression
                //      there should be a better way
                //      to write this
                // strt = tiles_lst;
                // if (node == nullptr) {
                    break;
                // } else {
                    // continue;
                // }
            }
        }
        strt = tiles_lst;
        if (node == nullptr) {
            break;
        }
        prev = node;
        node = node->next;
    }

    //generate new list from remaining nodes in linked_lst
    char* cropped_list = nullptr;
    cropped_list = make_tile_list_tt(new_tiles, matches);

    return cropped_list;
}

#if false
//compare names on tiles_lst to names on new_tiles
//but convert new_tiles to linked list first
bool check_tile_names_ll(char* tiles_lst, char** new_tiles, bool set_auto)
{
    bool append_new_only = set_auto;
    int tiles_lst_len = strlen(tiles_lst);
    tile_name* linked_lst = make_name_list(*new_tiles);  //need to free linked_lst at the end
    tile_name* node = linked_lst;
    tile_name* prev = nullptr;
    char* strt = tiles_lst;             //keeps track of first letter of name on TILES.LST
    while (node != nullptr) {
        for (int i = 0; i < tiles_lst_len; i++)
        {
            if (tiles_lst[i] != '\n' && tiles_lst[i] != '\0') {
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
            //first match found, ask what to do
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
                    return check_tile_names_ll(tiles_lst, new_tiles, set_auto);
                }
            }
            if (append_new_only == true) {
                //remove node from list if match found
                node = free_tile_name_node(node, prev, &linked_lst);
                //TODO: i have the distinct impression
                //      there should be a better way
                //      to write this
                strt = tiles_lst;
                i = 0;
                if (node == nullptr) {
                    break;
                }
            }
        }
        strt = tiles_lst;
        if (node == nullptr) {
            break;
        }
        prev = node;
        node = node->next;
    }
    //generate new list from remaining nodes in linked_lst
    if (append_new_only == true) {
        node = linked_lst;
        // int num_tiles = 0;
        int total_size = 0;
        while (node != nullptr) {
            // num_tiles++;
            total_size += node->length+1;
            node = node->next;
        }
        //if there are no nodes (or none with viable names)
        if (total_size < 1) {
            tinyfd_notifyPopup("TILES.LST not updated...",
                            "All new tile-names were already\n"
                            "found on TILES.LST.\n"
                            "No new tile-names were added.\n",
                            "info");
            //TODO: test this free()
            // free_tile_name_lst(linked_lst);
            return false;
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
        char* cropped_list = (char*)malloc(total_size+1);
        node = linked_lst;
        char* c = cropped_list;
        while (node != nullptr)
        {
            size_t amount_to_copy = node->length+1;
            memcpy(c, node->name_ptr, amount_to_copy);
            c += amount_to_copy;
            node = node->next;
        }
        c[0] = '\0';
//test above for speed///////////////////////////////
        free(*new_tiles);
        *new_tiles = cropped_list;
    }
    free_tile_name_lst(linked_lst);
    return true;
}
#endif

#if false
//compare names on tiles_lst to names on new_tiles
//increments by '\n' to check each name individually
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
                            "YES:   Skip and append only new names?\n"
                            "NO:    Rename the new tiles?", 72);
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
#endif

//append new tile-names to the end of TILES.LST
char* append_tiles_lst_ll_tt(char* tiles_lst_path, town_tile* new_tiles_list, bool set_auto)
{
    if (io_file_exists(tiles_lst_path) == false) {return nullptr;}

    int tiles_lst_size = io_file_size(tiles_lst_path);

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

    //search old_tiles_list (TILES.LST)
    //for matching names from new_tiles_list
    char* cropped_list = check_tile_names_ll_tt(old_tiles_list, new_tiles_list, set_auto);
    if (cropped_list == nullptr) {
        return old_tiles_list;
    }

    //append new list_of_tiles to the end of original list
    //in a new buffer large enough to fit both
    int new_lst_size   = strlen(cropped_list);
    char* final_tiles_list = (char*)malloc(tiles_lst_size + new_lst_size);
    strncpy(final_tiles_list, old_tiles_list, tiles_lst_size);
    if (final_tiles_list[tiles_lst_size] == '\n') {
        final_tiles_list[tiles_lst_size+1] = '\0';
    } else {
        final_tiles_list[tiles_lst_size] = '\0';
    }
    strncat(final_tiles_list, cropped_list, new_lst_size);

    //write combined lists out
    io_backup_file(tiles_lst_path);
    write_tiles_lst(tiles_lst_path, final_tiles_list);

    free(old_tiles_list);
    return final_tiles_list;
}

#if false
//append new tile-names to the end of TILES.LST
char* append_tiles_lst(char* tiles_lst_path, char** new_tiles_list, bool set_auto)
{
    if (tiles_lst_path == nullptr) {return nullptr;}
    int tiles_lst_size = io_file_size(tiles_lst_path);
    //load TILES.LST into memory
    char* old_tiles_list = (char*)malloc(tiles_lst_size+1);
    FILE* tiles_lst = fopen(tiles_lst_path, "rb");
    int size = fread(old_tiles_list, tiles_lst_size, 1, tiles_lst);
    if (size != 1) {
        printf("\n\nUnable to read entire TILES.LST file?\n\n");
        printf("fread     size: %d\n", size);
    }
    fclose(tiles_lst);
    //search final_tiles_list (TILES.LST)
    //for matching names from new_tiles_list
    bool success = check_tile_names_ll(old_tiles_list, new_tiles_list, set_auto);
    if (success == false) {
        return old_tiles_list;
    }
    io_backup_file(tiles_lst_path);
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
#endif

bool auto_export_TMAP_tiles_lst(user_info* usr_nfo, char* save_buff, char* tiles_lst, char* new_tile_list)
{
    bool success = false;
    //if default_game_path not set
    //ask user if they want to auto-export
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
                    "Cannot find TILES.LST...",
                    "TILES.LST is used to identify town map tiles\n"
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

                    "YES:    Create new TILES.LST in /Fallout 2/data/art/tiles/\n"
                    "NO:     Re-check game folder\n",
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
            usr_nfo->game_files.FRM_TILES_LST = tiles_lst;
            return false;
        }
    }
        return true;
}

//append names of new tiles to end of TILES.LST
//or create new TILES.LST with only these new tiles
void add_TMAP_tiles_to_lst_tt(user_info* usr_nfo, town_tile* new_tile_ll, char* save_buff)
{
    char save_path[MAX_PATH];
    // strcpy(save_path, save_buff);
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
            int num_tiles = 0;
            town_tile* ptr = new_tile_ll;
            while (ptr != nullptr) {
                num_tiles++;
                ptr = ptr->next;
            }
            uint8_t match_buff[1+num_tiles/8] = {0};
            char* new_tile_list = make_tile_list_tt(new_tile_ll, nullptr);
            tiles_lst = write_tiles_lst(save_path, new_tile_list);
        }
        if (choice == NO) {           // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
            game_path = tinyfd_selectFolderDialog(
                "Select directory to save to...", usr_nfo->default_save_path);
            strncpy(usr_nfo->default_save_path, game_path, MAX_PATH);
            return add_TMAP_tiles_to_lst_tt(usr_nfo, new_tile_ll, save_path);
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
            tiles_lst = append_tiles_lst_ll_tt(save_path, new_tile_ll, false);
        }
        if (choice == NO) {              // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
        //backup original file
            io_backup_file(save_path);

        //over-write original file
            int num_tiles = 0;
            town_tile* ptr = new_tile_ll;
            while (ptr != nullptr) {
                num_tiles++;
                ptr = ptr->next;
            }
            uint8_t* match_buff = (uint8_t*)calloc(1, 1+num_tiles/8);   //TODO: make this more precise?
            char* new_tile_list = make_tile_list_tt(new_tile_ll, match_buff);

            tiles_lst = write_tiles_lst(save_path, new_tile_list);
        }
    }

    if (usr_nfo->game_files.FRM_TILES_LST != nullptr) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    // strcpy(usr_nfo->game_files.FRM_TILES_LST, tiles_lst);
    usr_nfo->game_files.FRM_TILES_LST = tiles_lst;
}

#if false
//append names of new tiles to end of TILES.LST
//or create new TILES.LST with only these new tiles
void add_TMAP_tiles_to_lst(user_info* usr_nfo, char** new_tile_list, char* save_buff)
{
    char* tiles_lst = usr_nfo->game_files.FRM_TILES_LST;
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
    strncat(save_buff, "/TILES.LST", 11);
    success = io_file_exists(save_buff);
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
            *strrchr(save_buff, '/') = '\0';
            return;
        }
        if (choice == YES) {          // Yes = Create new TILES.LST in this folder
            tiles_lst = write_tiles_lst(save_buff, *new_tile_list);
        }
        if (choice == NO) {           // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
            game_path = tinyfd_selectFolderDialog(
                "Select directory to save to...", usr_nfo->default_save_path);
            strncpy(usr_nfo->default_save_path, game_path, MAX_PATH);
            *strrchr(save_buff, '/') = '\0';
            return add_TMAP_tiles_to_lst(usr_nfo, new_tile_list, save_buff);
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
            *strrchr(save_buff, '/') = '\0';
            return;
        }
        if (choice == YES) {             // Yes = Append to TILES.LST
            tiles_lst = append_tiles_lst(save_buff, new_tile_list, false);
        }
        if (choice == NO) {              // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
        //over-write original file
            io_backup_file(save_buff);
            tiles_lst = write_tiles_lst(save_buff, *new_tile_list);
        }
    }
    *strrchr(save_buff, '/') = '\0';
    if (usr_nfo->game_files.FRM_TILES_LST != nullptr) {
        free(usr_nfo->game_files.FRM_TILES_LST);
    }
    usr_nfo->game_files.FRM_TILES_LST = tiles_lst;
}
#endif