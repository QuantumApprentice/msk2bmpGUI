#pragma once
#include "imgui.h"
#include "Load_Files.h"
#include "FRM_Convert.h"


struct variables {
    char* exe_directory = NULL;

    //TODO: maybe store Color_Pick in config settings?
    Palette* FO_Palette = nullptr;
    GLuint tile_texture_prev;
    GLuint tile_texture_rend;
    shader_info shaders;
    ImVec4 tint_col   = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
    ImVec2 uv_min     = ImVec2(0.0f, 0.0f);                 // Top-left
    ImVec2 uv_max     = ImVec2(1.0f, 1.0f);                 // Bottom-right
    bool link_brush_sizes = true;
    ImVec2 brush_size{ 10, 10 };
    uint8_t Color_Pick = 230;

    int color_match_algo = 0;   //0 used to be SDL, is now Euclidian_Distance

    bool Palette_Update = false;

    uint64_t CurrentTime_ms = 0;        //TODO: need to test on 32-bit apps

    ImVec2 mouse_delta;
    ImVec2 new_mouse_pos;
    // uint8_t Color_Pick = 230;

    struct LF F_Prop[99]{};

    //if edit_image_open == true, then edit window is open, else false for preview window
    bool edit_image_focused  = false;
    bool tile_window_focused = false;
    bool render_wind_focused = false;

    ImFont* Font;
    int global_font_size = 32;
    int window_number_focus = -1;
};

void Surface_to_OpenGl(Surface* Temp_Surface, GLuint *Optimized_Texture);
bool checkbox_handler(const char* text, bool* alpha);
GLuint init_texture(Surface* src, int w, int h, img_type type);

void prep_image_SURFACE(LF* F_Prop, Palette* pal, int color_match_algo, bool* window, bool alpha);
