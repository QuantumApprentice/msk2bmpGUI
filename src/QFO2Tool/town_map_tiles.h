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

struct town_tile {
    char*    name_ptr = nullptr;
    uint8_t* frm_data = nullptr;
    uint32_t length  = 0;
    uint32_t row     = 0;
    uint32_t tile_id = 0;       //line number where it appears in TILES.LST
    // uint32_t next;           //points to array index of next viable name
    town_tile* next  = nullptr;
};

//marking this static makes a local copy for each translation unit?
static int tile_mask[] = {
    43, 50,
    39, 51,
    35, 53,
    31, 54,
    27, 55,
    22, 57,
    18, 58,
    14, 59,
    11, 60,
     7, 62,
     3, 63,
     0, 65,
     1, 66,
     3, 68,
     4, 69,
     6, 70,
     7, 72,
     8, 73,
     9, 74,
    11, 75,
    12, 77,
    13, 78,
    14, 79,
    16, 80,
    17, 79,
    19, 75,
    20, 71,
    21, 67,
    23, 62,
    24, 58,
    25, 54,
    26, 50,
    28, 46,
    29, 42,
    30, 39,
    32, 35
};

town_tile* crop_TMAP_tile_ll(int offset_x, int offset_y, image_data *img_data, char* name);
void crop_single_tile(uint8_t* tile_buff, uint8_t* frm_pxls, int img_w, int img_h, int x, int y);
char* crop_TMAP_tiles(int offset_x, int offset_y, image_data *img_data, char* file_path, char* name);
