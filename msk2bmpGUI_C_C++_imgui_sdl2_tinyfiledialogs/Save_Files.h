#pragma once
#include "Load_Settings.h"
#include "load_FRM_OpenGL.h"
#include "Load_Files.h"

uint8_t* blend_PAL_texture(image_data* img_data);

//char* Save_FRM_SDL(SDL_Surface *f_surface, struct user_info* user_info);
char* Save_FRM_Image_OpenGL(image_data* img_data, struct user_info* user_info);
char* Save_FRM_Animation_OpenGL(image_data* img_data, user_info* user_info, char* name);
char* Save_FRx_Animation_OpenGL(image_data* img_data, char* default_save_path, char* name);
bool Save_Single_Dir_Animation_OpenGL(image_data* img_data, FILE* File_ptr, int dir);
bool Save_Single_FRx_Animation_OpenGL(image_data* img_data, char* c_name, int dir);
char* Set_Save_File_Name(image_data* img_data, char* name);

char* Save_IMG_SDL(SDL_Surface *b_surface, struct user_info* user_info);
void Save_Full_MSK_OpenGL(image_data* img_data, user_info* usr_info);

//void Save_FRM_tiles_SDL(SDL_Surface *PAL_surface, struct user_info* user_info);
void Save_FRM_Tiles_OpenGL(LF* F_Prop, struct user_info* user_info, char* exe_path);
void save_TMAP_tiles(user_info* usr_info, char* exe_path,
                     image_data* img_data,
                     int x, int y);

//void Save_MSK_Tiles_SDL(SDL_Surface* MSK_surface, struct user_info* user_info);
void Save_MSK_Tiles_OpenGL(image_data* img_data, struct user_info* user_info, char* exe_path);

//void Save_MSK_Image_SDL(SDL_Surface* surface, FILE* File_ptr, int x, int y);
void Save_MSK_Image_OpenGL(uint8_t* texture_buffer, FILE* File_ptr, int width, int height);

//void Split_to_Tiles_SDL(SDL_Surface *surface, struct user_info* user_info, img_type type, FRM_Header* FRM_Header);
void Split_to_Tiles_OpenGL(image_data* img_data, struct user_info* user_info, img_type save_type, FRM_Header* frm_header, char* exe_path);

bool auto_export_question(user_info *usr_info, char *exe_path, char *save_path, img_type save_type);
bool export_auto(user_info *usr_info, char *exe_path, char *save_path, img_type save_type);
void check_file(char* path, char* path_name, char* name, int tile_num, img_type type);
void Create_File_Name(char* buffer, char* name, img_type type, char* path, int tile_num);

void Set_Default_Game_Path(user_info* user_info, char* exe_path);
