#pragma once
#include "imgui-docking/imgui.h"
#include "Load_Files.h"
#include "FRM_Convert.h"
//#include "shaders/shader_class.h"


struct variables {
    ImVec4 tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
    ImVec2 uv_min     = ImVec2(0.0f, 0.0f);                 // Top-left
    ImVec2 uv_max     = ImVec2(1.0f, 1.0f);                 // Bottom-right

    bool Palette_Update = false;
    char* program_directory = NULL;

    clock_t CurrentTime = 0;

    ImVec2 mouse_delta;
    ImVec2 new_mouse_pos;

    //TODO: maybe store the color in config settings?
    uint8_t Color_Pick = 230;
    SDL_PixelFormat *pxlFMT_FO_Pal = nullptr;

    bool link_brush_sizes = true;
    ImVec2 brush_size{ 10, 10 };

    shader_info shaders;

    struct LF F_Prop[99]{};

    //if edit_image_open == true, then edit window is open, else false for preview window
    bool edit_image_focused  = false;
    bool tile_window_focused = false;
    bool render_wind_focused = false;

    ImFont* Font;
    int global_font_size = 32;
    int SDL_color = 0;
    int window_number_focus = -1;

};

void SDL_to_OpenGl(SDL_Surface* Temp_Surface, GLuint *Optimized_Texture);
//bool bind_PAL_data(SDL_Surface* surface, struct image_data* img_data);
//void Image2Texture(variables* My_Variables, int counter);
void Image2Texture(SDL_Surface* surface, GLuint* texture, bool* window);
void Prep_Image(LF* F_Prop, SDL_PixelFormat* pxlFMT_FO_Pal, int color_match, bool* window, bool alpha_off = false);
bool bind_NULL_texture(struct image_data* img_data, SDL_Surface* surface, img_type type);
bool checkbox_handler(char* text, bool* alpha);
