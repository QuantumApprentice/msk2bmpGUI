//creating pattern files for town-map tiles
//used to auto-magically paint tiles out in the mapper
//all the current research available is here:
//http://duckandcover.cx/forums/viewtopic.php?p=168760

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
// #include <smmintrin.h>

bool is_tile_blank(town_tile* tile)
{
    int buff_size = 36*5;
    bool not_blank = true;
    __m128i ONES = _mm_set_epi64x(-1, -1);
    __m128i* frm_ptr128 = (__m128i*)tile->frm_data;


    int64_t* ptr = (int64_t*)&ONES;

    for (int i = 0; i < buff_size; i++) {
        // if (_mm_test_all_zeros(_mm_loadu_si128(frm_ptr128+i), ONES) == false) {
        //     not_blank = false;
        //     break;
        // }
    }

    return not_blank;
}

//tile-names should already be on TILES.LST
//so we loop through the list and identify the line number
//then assign that line number as the tile_id
void assign_tile_id(tt_arr_handle* handle, const char* FRM_tiles_LST)
{
    int tiles_lst_len = strlen(FRM_tiles_LST);

    for (int i = 0; i < handle->size; i++)
    {
        tt_arr* node = &handle->tile[i];
        if (node->tile_id == -1) {
            //skip known blank tiles
            continue;
        }

        //art FID is 0 indexed, so start at 0 when assigning lines
        int current_line = 0;
        const char* strt = FRM_tiles_LST;
        for (int j = 0; j < tiles_lst_len; j++)
        {
            if (FRM_tiles_LST[j] != '\n') {
                continue;
            }
            current_line++;

            if (tolower(strt[0]) != tolower(node->name_ptr[0])) {
                strt = &FRM_tiles_LST[j+1];
                continue;
            }
            if (io_strncmp(strt, node->name_ptr, strlen(node->name_ptr)) != 0) {
                strt = &FRM_tiles_LST[j+1];
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
    }
}

void export_TMAP_tiles_pattern(user_info* usr_info, tt_arr_handle* handle, char* save_path)
{
    if (handle == nullptr) {
    //TODO: place a warning here, this needs tile_arr*head to work
        return;
    }
    int choice = 0;
    const char* FRM_tiles_LST = usr_info->game_files.FRM_TILES_LST;
    if (FRM_tiles_LST == nullptr) {
        if (usr_info->default_game_path[0] != '\0') {
            usr_info->game_files.FRM_TILES_LST = load_LST_file(usr_info->default_game_path, "/data/art/tiles/", "TILES.LST");
            //TODO: keep using load_LST_file()? or replace with
            // load_FRM_tiles_LST(usr_info, state)?

        }
        if (FRM_tiles_LST == nullptr) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] export_TMAP_tiles_pattern()\n\n"
                "Unable to find art/tiles/TILES.LST.\n\n"
                "TILES.LST is needed to match up\n"       // data/art/tiles/TILES.LST
                "the tile-name to a line number,\n"
                "then that line number is used\n"
                "in the pattern file to indicate\n"
                "which tile is in what position.\n\n"
            );
            printf("Error: export_TMAP_tiles_pattern() Unable to find art/tiles/TILES.LST: %d\n", __LINE__);
            return;
        }
    }

    assign_tile_id(handle, FRM_tiles_LST);

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
            uint32_t temp  = ptr[i].tile_id;
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


    char* actual_path = io_path_check(save_path);
    if (actual_path) {
        strncpy(save_path, actual_path, MAX_PATH);
    }

    bool success = io_create_path_from_file(save_path);
    if (!success) {
        // set_false(state);
        set_popup_warning(
            "Error: save_NEW_FRM_tiles_LST()\n"
            "Unable to create folders\n"
        );
        return;
    }


    FILE* pattern_file = fopen(save_path, "wb");
    if (pattern_file == nullptr) {
        free(out_pattern);
        //TODO: needs a popup warning
        printf("Can't open pattern file...%d", __LINE__);
        return;
    }
    fwrite(out_pattern, 0x168C, 1, pattern_file);
    fclose(pattern_file);

    free(out_pattern);
}

struct PAT_list {
    char** list = NULL;
    int count   = 0;
};

PAT_list check_PAT_files(user_info* usr_nfo)
{
    char* game_path = usr_nfo->default_game_path;
    char path_buff[MAX_PATH];
    int num_patt = 0;
    //check for existing pattern filenames
    do {
        snprintf(path_buff, MAX_PATH, "%s/data/proto/tiles/PATTERNS/%08d", game_path, ++num_patt);
    } while (io_file_exists(path_buff));

    char** pattern_list = (char**)malloc(num_patt * sizeof(char*));
    char*  pattern_str  = (char*) malloc(num_patt * 10);
    char*  ptr = pattern_str;
    for (int i = 0; i < num_patt; i++)
    {
        snprintf(ptr, MAX_PATH, "%08d", i+1);
        pattern_list[i] = ptr;
        ptr += 10;
    }

    PAT_list final_list;
    final_list.list = pattern_list;
    final_list.count = num_patt;

    return final_list;
}

char* select_PAT_name(PAT_list* filenames)
{
    static int pattern_select = filenames->count-1;
    int pattern_count  = filenames->count;
    ImGui::Combo("pattern_filename", &pattern_select, filenames->list, pattern_count);

    return filenames->list[pattern_select];

}

//TODO: arena - this seems like a perfect use for memory arena
void free_PAT_list(PAT_list* filenames)
{
    free(filenames->list[0]);
    free(filenames->list);
    filenames->list  = NULL;
    filenames->count = 0;
}

void export_PAT_file_POPUP(user_info* usr_nfo, tt_arr_handle* handle, export_state* state, bool auto_export)
{
    static PAT_list filenames;
    if (!filenames.list) {
        filenames = check_PAT_files(usr_nfo);
    }

    ImGui::Text(
        "The first slot open for a pattern file is:\n"
    );
    char* selected_PAT = select_PAT_name(&filenames);
    ImGui::Text(
        "To use a different slot number, change it here.\n"
        "(This actually changes the filename.\n"
        "Default selection is a new file.\n"
        "To overwrite an existing entry, select that number.)\n"
    );

    //export button
    bool save_pattern = state->export_pattern;
    if (!auto_export) {
        if (ImGui::Button("Create Pattern file and add to Fallout 2...")) {
            if (handle == nullptr) {
            //TODO: place a warning here, this needs tile_arr*head to work
                return;
            }
            save_pattern = true;
        }
    }

    if (!handle) {
        return;
    }

    if (save_pattern) {
        char save_path[MAX_PATH];
        snprintf(save_path, MAX_PATH, "%s/data/proto/tiles/PATTERNS/%s", usr_nfo->default_game_path, selected_PAT);

        export_TMAP_tiles_pattern(usr_nfo, handle, save_path);
        state->export_pattern = false;
        free_PAT_list(&filenames);
    }
}