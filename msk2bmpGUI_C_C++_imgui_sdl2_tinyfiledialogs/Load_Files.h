#pragma once
#include <SDL.h>
#include <filesystem>
//#include <SDL_opengl.h>
#include <glad/glad.h>

struct LF {
    char Opened_File[_MAX_PATH];
    char * c_name;
    char * extension;
    SDL_Surface* image = nullptr;
    SDL_Surface* Pal_Surface = nullptr;
    SDL_Surface* Final_Render = nullptr;
    SDL_Surface* Map_Mask = nullptr;

    GLuint Optimized_Texture = 0;
    GLuint Optimized_Render_Texture = 0;

    int texture_width = 0, texture_height = 0;

    bool file_open_window;
    bool preview_tiles_window;
    bool preview_image_window;
    bool edit_image_window;
    bool edit_map_mask = false;

    char * FilterPattern1[3] = { "*.bmp" , "*.png", "*.frm" };
};
void Load_Files(LF[], struct user_info* user_info, int);
