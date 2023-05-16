#pragma once
#include <filesystem>
#include <SDL.h>

struct rectangle {
    int x1;
    int y1;

    int x2;
    int y2;
};

struct Anim_Header {
    uint16_t FPS = 0;
    uint16_t Action_Frame = 0;
    uint16_t Frames_Per_Orient;
    int16_t  Shift_Orient_x[6];
    int16_t  Shift_Orient_y[6];
    uint32_t Frame_0_Offset[6];
    uint32_t Frame_Area;
};

struct Anim_Frame_Info {
    uint16_t Frame_Width;
    uint16_t Frame_Height;
    uint32_t Frame_Size;
    int16_t  Shift_Offset_x;
    int16_t  Shift_Offset_y;
    SDL_Surface* frame_start;
};

enum Orientation
{
    no_data = -1,
    NE      =  0,
    E       =  1,
    SE      =  2,
    SW      =  3,
    W       =  4,
    NW      =  5
};

struct Anim_Frame {
    //TODO: remove unnecesary info?
    int frame_number = 0;
    Orientation orientation = no_data;
    Anim_Frame_Info* frame_info;
    rectangle bounding_box = {};
};



struct LF;
struct image_data;

bool Drag_Drop_Load_Animation(std::vector <std::filesystem::path>& path_vector, LF* F_Prop);
Orientation assign_direction(char* direction);
void set_names(char** names_array, image_data* img_data);
