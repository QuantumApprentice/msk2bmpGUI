#pragma once
#include <SDL.h>
#include <filesystem>

#include "load_FRM_OpenGL.h"
#include "Load_Settings.h"

//File info
struct LF {
    char Opened_File[MAX_PATH];
    char * c_name;
    char * extension;
    SDL_Surface* IMG_Surface = nullptr;
    SDL_Surface* PAL_Surface = nullptr;
    SDL_Surface* Final_Render = nullptr;
    SDL_Surface* Map_Mask = nullptr;

    image_data img_data;
    image_data edit_data;

    GLuint Optimized_Render_Texture = 0;
    GLuint Optimized_Mask_Texture = 0;

    //int texture_width = 0, texture_height = 0;

    img_type type;
    bool file_open_window = false;
    bool preview_tiles_window = false;
    bool show_image_render = false;
    bool edit_image_window = false;
    bool edit_map_mask = false;
    bool image_is_tileable = false;
};

bool Load_Files   (LF* F_Prop, struct user_info* user_info, SDL_PixelFormat* pxlFMT);
void Load_Edit_MSK(LF* F_Prop, struct user_info* user_info);
