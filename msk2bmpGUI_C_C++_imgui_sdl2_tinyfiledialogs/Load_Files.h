#pragma once
#include <SDL.h>

#include <filesystem>
#include <optional>
#include <set>

#include "load_FRM_OpenGL.h"
#include "Load_Settings.h"
#include "shader_class.h"

//File info
struct LF {
    char Frst_File[MAX_PATH];
    char Prev_File[MAX_PATH];
    char Opened_File[MAX_PATH];
    char Next_File[MAX_PATH];
    char Last_File[MAX_PATH];

    char * c_name;
    char * extension;
    //TODO: remove IMG_Surface, not used anymore
    SDL_Surface* IMG_Surface = nullptr;
    bool alpha          = true;
    bool show_stats     = false;
    bool show_squares   = false;
    bool show_tiles     = false;

    image_data img_data;
    image_data edit_data;

    bool file_open_window = false;
    bool preview_tiles_window = false;
    bool show_image_render = false;
    bool edit_image_window = false;
    bool image_is_tileable = false;
    bool edit_MSK = false;
};

struct shader_info {
    float palette[768];
    Shader* render_PAL_shader;
    Shader* render_FRM_shader;
    Shader* render_OTHER_shader;
    mesh giant_triangle;
};

char* Program_Directory();
bool Load_Files(LF* F_Prop, image_data* img_data, struct user_info* user_info, shader_info* shaders);
bool File_Type_Check(LF* F_Prop, shader_info* shaders, image_data* img_data);
bool Drag_Drop_Load_Files(char* file_name, LF* F_Prop, image_data* img_data, shader_info* shaders);
std::optional<bool> handle_directory_drop(char* file_name, LF* F_Prop, int* window_number_focus, int* counter, shader_info* shaders);
void handle_file_drop(char* file_name, LF* F_Prop, int* counter, shader_info* shaders);
void prep_extension(LF* F_Prop, user_info* usr_info, char* file_name);
void Next_Prev_File(char* next, char* prev, char* frst, char* last, char* current);
SDL_Surface* Surface_32_Check(SDL_Surface* surface);
void load_tile_texture(GLuint* texture, char* file_name);
