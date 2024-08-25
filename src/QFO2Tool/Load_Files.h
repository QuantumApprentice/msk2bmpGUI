#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <optional>
#include <set>

#include "load_FRM_OpenGL.h"
#include "Load_Settings.h"
#include "shader_class.h"


#ifdef QFO2_WINDOWS
    #define PLATFORM_SLASH  ('\\')
#elif defined(QFO2_LINUX)
    #define PLATFORM_SLASH  ('/')
#endif

//File info
struct LF {
    char Frst_File[MAX_PATH];
    char Prev_File[MAX_PATH];
    char Opened_File[MAX_PATH];
    char Next_File[MAX_PATH];
    char Last_File[MAX_PATH];

    char * c_name;
    char * extension;
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

//wrapper for array of strings
struct dropped_files {
    size_t count;
    size_t total_size;
    //lay out null terminated strings one after another?
    char* first_path;
};

char* Program_Directory();
void dropped_files_callback(GLFWwindow* window, int count, const char** paths);
bool Load_Files(LF* F_Prop, image_data* img_data, struct user_info* user_info, shader_info* shaders);
bool File_Type_Check(LF* F_Prop, shader_info* shaders, image_data* img_data, const char* file_name);
bool Drag_Drop_Load_Files(const char* file_name, LF* F_Prop, image_data* img_data, shader_info* shaders);
std::optional<bool> handle_directory_drop(char* file_name, LF* F_Prop, int* window_number_focus, int* counter, shader_info* shaders);
void handle_file_drop(char* file_name, LF* F_Prop, int* counter, shader_info* shaders);
void prep_extension(LF* F_Prop, user_info* usr_info, const char* file_name);
void Next_Prev_File(char* next, char* prev, char* frst, char* last, char* current);
void load_tile_texture(GLuint* texture, char* file_name);
