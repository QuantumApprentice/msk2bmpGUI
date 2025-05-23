#pragma once
#include "load_FRM_OpenGL.h"

// BakerStaunch
// So your grid structure might become:
//
// total row count,
// total col count,
// first non-transparent row,
// first non-transparent column,
// last non-transparent row,
// last non-transparent column
//
// Then your loops can just change to be
// for (int row = first non-transparent-row;
//           i <= last non-transparent-row;
//           row++)


struct export_state {
    char save_name[16] = "tile_";
    char LST_path[MAX_PATH];
    const char* language[4] = {
        "english",
        "french",
        "russian",
        "etc"
    };
    bool auto_export    = false;

    bool art = false;
    bool pro = false;
    bool pat = false;

    bool export_proto   = false;
    bool export_pattern = false;

    bool chk_game_path  = false;

    bool load_files     = false;
    bool loaded_FRM_LST = false;
    bool loaded_PRO_LST = false;
    bool loaded_PRO_MSG = false;

    bool make_FRM_LST   = false;
    bool make_PRO_LST   = false;
    bool make_PRO_MSG   = false;

    bool append_FRM_LST = false;
    bool append_PRO_LST = false;
    bool append_PRO_MSG = false;
};


//TODO: delete? this was for the linked list version
struct town_tile {
    char*    name_ptr = nullptr;
    uint8_t* frm_data = nullptr;
    uint32_t length  = 0;
    uint32_t row     = 0;
    uint32_t tile_id = 0;       //line number where it appears in TILES.LST
    town_tile* next  = nullptr;
};

struct tt_arr {
    char     name_ptr[14];  //names can't be longer than 8 characters plus extension (plus '\0')
    uint8_t  frm_data[80*36];
    uint32_t row     = 0;   //might not need these
    uint32_t col     = 0;   //might not need these
    uint32_t tile_id = 0;       //line number where it appears in TILES.LST (proto? or art?)
};

struct tt_arr_handle {
    int size = 0;
    int row_cnt = 0;
    int col_cnt = 0;
    tt_arr tile[];
};

//marking this static makes a local copy for each translation unit?
static int tile_mask[] = {
    43, 50,     //row 1
    39, 51,     //row 2
    35, 53,     //row 3
    31, 54,     //row 4
    27, 55,     //row 5
    22, 57,     //row 6
    18, 58,     //row 7
    14, 59,     //row 8
    11, 60,     //row 9
     7, 62,     //row 10
     3, 63,     //row 11
     0, 65,     //row 12
     1, 66,     //row 13
     3, 68,     //row 14
     4, 69,     //row 15
     6, 70,     //row 16
     7, 72,     //row 17
     8, 73,     //row 18
     9, 74,     //row 19
    11, 75,     //row 20
    12, 77,     //row 21
    13, 78,     //row 22
    14, 79,     //row 23
    16, 80,     //row 24
    17, 79,     //row 25
    19, 75,     //row 26
    20, 71,     //row 27
    21, 67,     //row 28
    23, 62,     //row 29
    24, 58,     //row 30
    25, 54,     //row 31
    26, 50,     //row 32
    28, 46,     //row 33
    29, 42,     //row 34
    30, 39,     //row 35
    32, 35,     //row 36
};

tt_arr_handle* crop_export_TMAP_tiles(Rect* offset, Surface* src, char* save_fldr, export_state* state, char* save_path, bool overwrite);
void crop_single_tile(uint8_t* tile_buff, uint8_t* frm_pxls, int img_w, int img_h, int x, int y);
