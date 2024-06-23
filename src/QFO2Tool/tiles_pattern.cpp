//creating pattern files for town-map tiles
//used to auto-magically paint tiles out in the mapper
//all the current research available is here:
//http://duckandcover.cx/forums/viewtopic.php?p=168760

#include <tinyfiledialogs.h>
#include <string.h>

#include "Edit_TILES_LST.h"
#include "platform_io.h"
#include "tiles_pattern.h"

//need to check if mapper can access above 4096
//need to check pattern file can access above 4096

//lay patterns out entire row at a time
//must be in little endian format
//rows start from the right in the x direction
//  and y from the top?
//why are tile entries separated by 16 bytes?
//last 12 bytes (footer?) identify x/y sizes,
//  pattern entry number?

bool is_tile_blank(town_tile* tile)
{
    int buff_size = 36*5;
    bool not_blank = true;
    __m128i ONES = _mm_set_epi64x(-1, -1);
    __m128i* frm_ptr128 = (__m128i*)tile->frm_data;


    int64_t* ptr = (int64_t*)&ONES;

    for (int i = 0; i < buff_size; i++) {
        if (_mm_test_all_zeros(_mm_loadu_si128(frm_ptr128+i), ONES) == false) {
            not_blank = false;
            break;
        }
    }

    return not_blank;
}

void assign_tile_id_w(town_tile* head, const char* tiles_lst)
{
    char c1, c2;
    const char* strt = tiles_lst;
    town_tile* node = head;
    int tiles_lst_len = strlen(tiles_lst);
    int current_line = 1;

    while (node != nullptr) {
        //skip if tile buffer is blank
        if (is_tile_blank(node)) {
            node->tile_id = 1;              //1? or 0? pretty sure 1 is blank tile in the mapper
            node = node->next;
            continue;
        }

        for (int i = 0; i < tiles_lst_len; i++)
        {
            if (tiles_lst[i] != '\n') {
                continue;
            }
            // if (tiles_lst[i] >= 'A' && tiles_lst[i] <= 'Z') {
            //     c1 = 'a' + tiles_lst[i] - 'A';
            // }
            // if (node->name_ptr[0] >= 'A' && node->name_ptr[0] <= 'Z') {
            //     c2 = 'a' + node->name_ptr[0] - 'A';
            // }
            // if (c1 != c2) {
            //     continue;
            // }

            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                strt = &tiles_lst[i+1];
                current_line++;
                continue;
            }
            if (io_strncmp(&strt[0], node->name_ptr, node->length) != 0) {
                strt = &tiles_lst[i+1];
                current_line++;
                continue;
            }
            //match found, assign current line # to current tile_id
            node->tile_id = current_line;
            break;
        }
        if (node->tile_id == 0) {
            //TODO: this needs its own popup window asking for next step
            printf("we've got a problem here, unable to find matching name\n");
        }
        strt = tiles_lst;
        current_line = 1;
        node = node->next;
    }
}

void assign_tile_id_f(town_tile* head, const char* tiles_lst)
{
    char c1, c2;
    const char* strt  = tiles_lst;
    town_tile* node   = head;
    int tiles_lst_len = strlen(tiles_lst);
    int current_line  = 1;

    for (int i = 0; i < tiles_lst_len; i++) {

        if (tiles_lst[i] != '\n') {
            continue;
        }

        while (node != nullptr) {
            //skip if tile buffer is blank
            while (is_tile_blank(node) == true) {
                node->tile_id = 1;              //1? or 0? pretty sure 1 is blank tile in the mapper
                node = node->next;
            }
            // if (tiles_lst[i] >= 'A' && tiles_lst[i] <= 'Z') {
            //     c1 = 'a' + tiles_lst[i] - 'A';
            // }
            // if (node->name_ptr[0] >= 'A' && node->name_ptr[0] <= 'Z') {
            //     c2 = 'a' + node->name_ptr[0] - 'A';
            // }
            // if (c1 != c2) {
            //     continue;
            // }

            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                node = node->next;
                continue;
            }
            if (io_strncmp(strt, node->name_ptr, node->length) != 0) {
                node = node->next;
                continue;
            }
            //match found, assign current line # to current tile_id
            node->tile_id = current_line;
            if (node->tile_id == 0) {
                //TODO: this needs its own popup window asking for next step
                printf("we've got a problem here, unable to find matching name\n");
            }
            node = node->next;
        }
        current_line++;
        node = head;
        strt = &tiles_lst[i+1];
    }
}

#include "Proto_Files.h"
void TMAP_tiles_make_row(user_info* usr_info, town_tile* head)
{
    int choice = 0;
    const char* tiles_lst = usr_info->game_files.FRM_TILES_LST;
    if (tiles_lst == nullptr) {
        if (strlen(usr_info->default_game_path) > 1) {
            usr_info->game_files.FRM_TILES_LST = load_tiles_lst_game(usr_info->default_game_path);
        }
        if (tiles_lst == nullptr) {
            choice = tinyfd_messageBox(
                "TILES.LST missing...",
                "Unable to find TILES.LST.\n\n"

                "TILES.LST is needed to match up\n"       //which one? proto one? or frm one?
                "the tile-name to a line number\n"
                "in TILES.LST,\n"
                "then that line number is used\n"
                "in the pattern file to indicate\n"
                "which tile is in what position.\n\n"

                "",
                "cancel", "warning", 1
            );

            if (choice == YES) {
                //point to TILES.LST?
            }
            if (choice == NO) {
                //create new TILES.LST?
            }
            if (choice == CANCEL) {
                return;
            }
        }
    }

    //TODO: speed test these
    assign_tile_id_f(head, tiles_lst);  // TILES.LST loop outside

    assign_tile_id_w(head, tiles_lst);  // town_map* loop outside


    town_tile* node = head;
    int col_max  = 0;
    int col_cnt  = 0;
    int last_row = node->row;
    //get the length of the longest row
    //and also the total number of rows
    while (node != nullptr)
    {
        //need the line number from TILES.LST
        if (node->row != last_row) {
            if (col_cnt > col_max) {
                col_max = col_cnt;
            }
            col_cnt  = 0;
            last_row = node->row;
        }
        col_cnt++;
        node = node->next;
    }

    int out_size = col_max*(last_row);

    //lay out tiles from node until we get to the end of a row
    //once we're at the end of a row, but not at row_max
    //fill the rest of the row w/blank tiles
    //  out_pattern is array of
    //  4x int struct w/pragma pack applied
    #pragma pack(push, 1)
    struct pattern {
        uint32_t tile_id;
        uint32_t unkown_b;
        uint32_t unkown_c;
        uint32_t unkown_d;
    };
    #pragma pack(pop)

    //0x168C is the total length of a pattern file
    //includes length of footer (12 bytes)
    //total number of tile lines is 360? possibly more (needs more research)
    pattern* out_pattern = (pattern*)calloc(1, 0x168C);

    node          = head;
    last_row      = node->row;
    int col_indx  = 0;
    int tile_indx = 0;
    while (node != nullptr)
    {
        if (node->row != last_row) {
            while (col_indx < col_max) {
                out_pattern[tile_indx++].tile_id = 1;               //0? or 1?
                col_indx++;
            }
            last_row = node->row;
            col_indx = 0;
        }
        //TODO: the tile_id isn't the actual line number of the art?
        //      should I assign it correctly in the first place?
        out_pattern[tile_indx++].tile_id = node->tile_id-1;
        node = node->next;
        col_indx++;
    }
    //TODO: remove this if it's unnecessary to keep tiles aligned
    //writes out extra blank tiles at the end
    while (col_indx < col_max) {
        out_pattern[tile_indx++].tile_id = 1;               //0? or 1?
        col_indx++;
    }

    pattern* ptr = out_pattern;
    for (int k = 0; k < last_row; k++)
    {
        int j = 0;
        for (int i = col_max-1; i > (col_max-1)/2; i--)
        {
            uint32_t temp = ptr[i].tile_id;
            ptr[i].tile_id = ptr[j].tile_id;
            ptr[j].tile_id = temp;
            j++;
        }
        ptr = &ptr[col_max];
    }


    uint32_t* u32_ptr = (uint32_t*)&out_pattern[360];        //offset for footer
    u32_ptr[0] = col_max;
    u32_ptr[1] = last_row;
    u32_ptr[2] = 0;//unkown exactly what this does?

    printf("how we doin?\n");

    char file_buff[MAX_PATH];
    snprintf(file_buff, MAX_PATH, "%s/data/proto/tiles/PATTERNS/00000002", usr_info->default_game_path, head->name_ptr);

    FILE* pattern_file = fopen(file_buff, "wb");
    fwrite(out_pattern, 0x168C, 1, pattern_file);
    fclose(pattern_file);



    //TODO: actually save the array out
    //write out 1 row at a time?
    //16 bytes per row
    //tile_id in little endian
    //  using first(last?) four bytes of the 16

    
    free(out_pattern);
}

