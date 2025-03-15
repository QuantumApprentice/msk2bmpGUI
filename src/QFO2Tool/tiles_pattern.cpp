//creating pattern files for town-map tiles
//used to auto-magically paint tiles out in the mapper
//all the current research available is here:
//http://duckandcover.cx/forums/viewtopic.php?p=168760

#include <tinyfiledialogs.h>
#include <string.h>

#include "Edit_TILES_LST.h"
#include "platform_io.h"
#include "tiles_pattern.h"

#include "ImGui_Warning.h"

//need to check if mapper can access above 4096
//need to check pattern file can access above 4096

//lay patterns out entire row at a time
//must be in little endian format
//rows start from the right in the x direction
//  and y from the top
//why are tile entries separated by 16 bytes?
//last 12 bytes (footer?) identify x/y sizes,
//  pattern entry number?
#include <emmintrin.h>
#include <smmintrin.h>

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

//tile-names should already be on TILES.LST
//so we loop through the list and identify the line number
//then assign that line number as the tile_id
void assign_tile_id_arr(tt_arr_handle* handle, const char* tiles_lst)
{
    int tiles_lst_len = strlen(tiles_lst);

    tt_arr* tiles = handle->tile;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        //skip known blank tiles
        if (node->tile_id == 1) {
            continue;
        }

        //art FID is 0 indexed, so start at 0 when assigning lines
        int current_line = 0;
        const char* strt = tiles_lst;
        for (int j = 0; j < tiles_lst_len; j++)
        {
            if (tiles_lst[j] != '\n') {
                continue;
            }
            // if (tiles_lst[i] >= 'A' && tiles_lst[i] <= 'Z') {
            //     c1 = 'a' + tiles_lst[i] - 'A';
            // }
            // if (node[i].name_ptr[0] >= 'A' && node[i].name_ptr[0] <= 'Z') {
            //     c2 = 'a' + node[i].name_ptr[0] - 'A';
            // }
            // if (c1 != c2) {
            //     continue;
            // }

            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                strt = &tiles_lst[j+1];
                current_line++;
                continue;
            }
            if (io_strncmp(strt, node->name_ptr, strlen(node->name_ptr)) != 0) {
                strt = &tiles_lst[j+1];
                current_line++;
                continue;
            }
            //match found, assign current line # to current tile_id
            node->tile_id = current_line;
            break;
        }
        if (tiles[i].tile_id == 0) {
            //TODO: this needs its own popup window asking for next step
            printf("we've got a problem here, unable to find matching name\n");
        }
    }
}

void TMAP_tiles_pattern_arr(user_info* usr_info, tt_arr_handle* handle, char* file_buff)
{
    if (handle == nullptr) {
    //TODO: place a warning here, this needs tile_arr*head to work
        return;
    }
    //TODO:
    //create new TILES.LST?
    //point to TILES.LST?
    int choice = 0;
    const char* tiles_lst = usr_info->game_files.FRM_TILES_LST;
    if (tiles_lst == nullptr) {
        if (strlen(usr_info->default_game_path) > 1) {
            usr_info->game_files.FRM_TILES_LST = load_tiles_lst_game(usr_info->default_game_path);
        }
        if (tiles_lst == nullptr) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] TMAP_tiles_pattern_arr()\n\n"
                "Unable to find TILES.LST.\n\n"
                "TILES.LST is needed to match up\n"       // data/art/tiles/TILES.LST
                "the tile-name to a line number,\n"
                "then that line number is used\n"
                "in the pattern file to indicate\n"
                "which tile is in what position.\n\n"
            );
            printf("Error: TMAP_tiles_pattern_arr() Unable to find TILES.LST: %d\n", __LINE__);
            return;
        }
    }

    assign_tile_id_arr(handle, tiles_lst);

    //  pattern file is array of
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

    //all tile entries appear to need to be (tile_id-1) | 0x4000000
    //or else they won't show the correct tile pattern
    //0x4000000 == tile frm (engine looks for art id in TILES.LST)
    //in the preview window (so this takes the FrmID?)
    //maybe pass in proto info?

    //TODO: watch out!
    //      only 320 lines available at this current size
    tt_arr* tiles = handle->tile;
    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &tiles[i];
        //empty tile entries (id==1) also need to be | 0x4000000
        out_pattern[i].tile_id = node->tile_id | 0x4000000;
    }

    //flip the entries around so they line up
    //correctly when the mapper lays them out
    pattern* ptr = out_pattern;
    for (int k = 0; k < handle->row_cnt; k++)
    {
        int j = 0;
        for (int i = handle->col_cnt-1; i > (handle->col_cnt-1)/2; i--)
        {
            uint32_t temp = ptr[i].tile_id;
            ptr[i].tile_id = ptr[j].tile_id;
            ptr[j].tile_id = temp;
            j++;
        }
        ptr = &ptr[handle->col_cnt];
    }

    //assign the row and column count
    //so the mapper knows width/height of pattern
    uint32_t* u32_ptr = (uint32_t*)&out_pattern[360];        //offset for footer
    u32_ptr[0] = handle->col_cnt;
    u32_ptr[1] = handle->row_cnt;
    u32_ptr[2] = 0;//unkown exactly what does this do?

    FILE* pattern_file = fopen(file_buff, "wb");
    if (pattern_file == nullptr) {
        free(out_pattern);
        printf("Can't open pattern file...%d", __LINE__);
        return;
    }
    fwrite(out_pattern, 0x168C, 1, pattern_file);
    fclose(pattern_file);

    free(out_pattern);
}


void export_pattern_file(user_info* usr_nfo, tt_arr_handle* handle)
{
    if (fallout2exe_exists(usr_nfo->default_game_path) == false) {
        ImGui::Text(
            "Default game path is not set.\n"
            "Please set the path for fallout2.exe here:"
        );
        ImGui::InputText("###fallout2.exe", usr_nfo->default_game_path, MAX_PATH);
    }

    //check pattern filename
    int patt_num = 1;
    static char file_buff[MAX_PATH];
    snprintf(file_buff, MAX_PATH, "%sdata/proto/tiles/PATTERNS/00000001", usr_nfo->default_game_path);
    while (io_file_exists(file_buff)) {
        snprintf(file_buff, MAX_PATH, "%sdata/proto/tiles/PATTERNS/%08d", usr_nfo->default_game_path, ++patt_num);
    }
    //allow user to change filename
    static char patt_buff[12];
    if (patt_buff[0] == '\0') {
        snprintf(patt_buff, 12, "%08d", patt_num);
    }
    ImGui::Text(
        "%s\n"
        "was the first slot open for pattern files.\n"
        "To use a different slot number, change it here.\n"
        "(This actually changes the filename, if you\n"
        "want this file to work, change only the end number\n"
        "and it must be contiguous from any previous number.)\n",
        patt_buff
    );
    ImGui::InputText("###patternfile", patt_buff, 9);


    //export button
    if (ImGui::Button("Create Pattern file and add to Fallout 2...")) {
        if (handle == nullptr) {
        //TODO: place a warning here, this needs tile_arr*head to work
            return;
        }
        if (atoi(patt_buff) != patt_num) {
            snprintf(file_buff, MAX_PATH, "%sdata/proto/tiles/PATTERNS/%s", usr_nfo->default_game_path, patt_buff);
        }

        TMAP_tiles_pattern_arr(usr_nfo, handle, file_buff);

        ImGui::EndPopup();
    }
    if (ImGui::Button("Close")) {
        ImGui::CloseCurrentPopup();
    }
}