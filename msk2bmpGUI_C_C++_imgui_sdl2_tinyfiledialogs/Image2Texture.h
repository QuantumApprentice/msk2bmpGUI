#pragma once
#include "imgui-docking/imgui.h"
#include "Load_Files.h"
#include "FRM_Convert.h"
#include "shaders/shader_class.h"


struct variables {
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
    ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
    ImVec2 uv_max = ImVec2(1.0f, 1.0f);
    bool Render_Tiles = false;
    bool Preview_Tiles = false;
    bool Render_Window = false;

    bool Palette_Update = false;

    int Render_Width = 0, Render_Height = 0;
    clock_t CurrentTime = 0;

    position mouse_delta;
    position new_mouse_pos;

    //TODO: maybe store the color in config settings?
    uint8_t Color_Pick = 230;
    //SDL_Color *PaletteColors = nullptr;
    SDL_PixelFormat *pxlFMT_FO_Pal = nullptr;
    float palette[768] = {};


    //Shader stuff
    //Shader color_cycle { "shaders//zoom_shader.vert", "shaders//color_cycle_1D.frag" };
    Shader render_PAL_shader { "shaders//passthru_shader.vert", "shaders//render_PAL.frag" };
    Shader render_FRM_shader { "shaders//passthru_shader.vert", "shaders//render_FRM.frag" };
    mesh giant_triangle;

    float old_zoom = 1.0;
    float new_zoom = 1.0;
    float bottom_left_pos[2] = { 0.0, 0.0 };
    bool panning = false;

    SDL_Surface* Temp_Surface = nullptr;

    struct LF F_Prop[99]{};
};

//void Image2Texture(variables* My_Variables, int counter);
void Image2Texture(SDL_Surface* surface,      GLuint* texture,      bool* window);
void SDL_to_OpenGl(SDL_Surface* Temp_Surface, GLuint *Optimized_Texture);
void Prep_Image(LF* F_Prop, SDL_PixelFormat* pxlFMT_FO_Pal, bool color_match, bool* window);
bool bind_PAL_data(SDL_Surface* surface, struct image_data* img_data);
bool bind_NULL_texture(struct image_data* img_data, SDL_Surface* surface);
