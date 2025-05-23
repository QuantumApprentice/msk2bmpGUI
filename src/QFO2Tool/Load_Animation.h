#pragma once
#include <set>
#include <vector>

#include "MiniSDL.h"

#include "Load_Settings.h"

struct rectangle {
    int x1 = 0;
    int y1 = 0;

    int x2 = 0;
    int y2 = 0;
};

struct ANM_Header {
    uint16_t FPS = 0;
    uint16_t Action_Frame = 0;
    uint16_t Frames_Per_Orient;
    int16_t  Shift_Orient_x[6];
    int16_t  Shift_Orient_y[6];
    uint32_t Frame_0_Offset[6];
    uint32_t Frame_Area;
};

struct ANM_Frame {
    uint16_t Frame_Width;
    uint16_t Frame_Height;
    uint32_t Frame_Size;
    int16_t  Shift_Offset_x;
    int16_t  Shift_Offset_y;
    Surface* frame_start = NULL;
};

enum Direction
{
    no_data = -1,
    NE      =  0,
    E       =  1,
    SE      =  2,
    SW      =  3,
    W       =  4,
    NW      =  5
};

struct ANM_Dir {
    int num_frames        = 0;
    Direction orientation = no_data;
    Surface** frame_data  = NULL;
    rectangle* frame_box  = {};
};

struct LF;
struct image_data;
struct shader_info;

bool Drag_Drop_Load_Animation(std::vector <std::filesystem::path>& path_set, LF* F_Prop);
Direction assign_direction(char* direction);
void set_directions(const char** names_array, image_data* img_data);
void Gui_Video_Controls(image_data* img_data, img_type type);
void Next_Prev_Buttons(LF* F_Prop, image_data* img_data, shader_info* shaders);
