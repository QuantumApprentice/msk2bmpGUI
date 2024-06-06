#pragma once
#include "load_FRM_OpenGL.h"

struct town_tile {
    char*    name_ptr;
    uint8_t* frm_data;
    uint32_t length;
    // uint32_t next;           //points to array index of next viable name
    town_tile* next;
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
void crop_single_tile(int img_w, int img_h, uint8_t* tile_buff, uint8_t* frm_pxls, int x, int y);
char* crop_TMAP_tiles(int offset_x, int offset_y, image_data *img_data, char* file_path, char* name);
