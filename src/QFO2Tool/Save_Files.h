#pragma once
#include "Load_Settings.h"
#include "load_FRM_OpenGL.h"
#include "Load_Files.h"
#include "town_map_tiles.h"
#include "MiniSDL.h"

enum Save_Type {
    single_frm,
    single_dir,
    all_dirs,
};

struct Save_Info {
    Save_Type s_type;
    int action_frame = 0;
};

uint8_t* blend_PAL_texture(image_data* img_data);

char* Save_IMG_STB(Surface* b_surface, user_info* usr_nfo);
void Save_Full_MSK_OpenGL(image_data* img_data, user_info* usr_info);


void save_MSK_tile(uint8_t* texture_buffer, FILE* File_ptr, int width, int height);

bool auto_export_question(user_info *usr_info, char *exe_path, char *save_path, img_type save_type);
bool export_auto(user_info *usr_info, char *exe_path, char *save_path, img_type save_type);

void Set_Default_Game_Path(user_info* user_info, char* exe_path);



void init_IFD();

bool save_PNG_popup_INTERNAL(image_data* img_data, user_info* usr_info, float* FO_pal);
void save_as_GIF(image_data* img_data, struct user_info* usr_nfo);

// bool save_FRM_SURFACE(char* save_name, image_data* img_data, user_info* usr_info, Save_Info* sv_info, bool overwrite);
bool ImDialog_save_FRM_SURFACE(image_data* img_data, user_info* usr_info, Save_Info* sv_info);
bool ImDialog_save_TILE_SURFACE(image_data* img_data, user_info* usr_info, Save_Info* sv_info);

tt_arr_handle* export_TMAP_tiles_POPUP(user_info* usr_info, image_data* img_data, Rect* offset);
