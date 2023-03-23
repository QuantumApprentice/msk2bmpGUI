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
    SDL_Surface* image = nullptr;
    SDL_Surface* Pal_Surface = nullptr;
    SDL_Surface* Final_Render = nullptr;
    SDL_Surface* Map_Mask = nullptr;

    image_data img_data;
    image_data edit_data;

    GLuint palette_buffer;
    GLuint palette_texture;
    GLuint render_buffer;
    GLuint render_texture;

    GLuint Optimized_Texture = 0;
    GLuint Optimized_Render_Texture = 0;
    GLuint Optimized_Mask_Texture = 0;

    int texture_width = 0, texture_height = 0;

    img_type type;
    bool file_open_window;
    bool preview_tiles_window;
    bool preview_image_window;
    bool edit_image_window;
    bool edit_map_mask = false;
    bool image_is_tileable = false;
    bool window_focused = false;
};

void Load_Files   (LF* F_Prop, struct user_info* user_info, SDL_PixelFormat* pxlFMT);
void Load_Edit_MSK(LF* F_Prop, struct user_info* user_info);
