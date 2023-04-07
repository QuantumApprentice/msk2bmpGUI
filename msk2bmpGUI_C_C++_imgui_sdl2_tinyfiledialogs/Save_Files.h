#pragma once
#include "Load_Settings.h"
#include "load_FRM_OpenGL.h"
#include "Load_Files.h"

#pragma pack(push, 1)
typedef struct {
    uint32_t version;                                           // 0x0000
    uint16_t FPS = 0;                                           // 0x0004
    uint16_t Action_Frame = 0;                                  // 0x0006
    uint16_t Frames_Per_Orientation;                            // 0x0008
    uint16_t Shift_Orient_x[6]{};                               // 0x000A
    uint16_t Shift_Orient_y[6]{};                               // 0x0016
    uint32_t Frame_0_Offset[6]{};                               // 0x0022
    uint32_t Frame_Area;                                        // 0x003A
    uint16_t Frame_0_Width;                                     // 0x003E
    uint16_t Frame_0_Height;                                    // 0x0040
    uint32_t Frame_0_Size;                                      // 0x0042
    uint16_t Shift_Offset_x = 0;                                // 0x0046
    uint16_t Shift_Offset_y = 0;                                // 0x0048
    //uint8_t  Color_Index = 0;                                 // 0x004A
} FRM_Header;
#pragma pack(pop)

uint8_t* blend_PAL_texture(image_data* img_data);

char* Save_FRM_SDL(SDL_Surface *f_surface, struct user_info* user_info);
char* Save_FRM_OpenGL(image_data* img_data, struct user_info* user_info);

char* Save_IMG(SDL_Surface *b_surface, struct user_info* user_info);

void Save_FRM_tiles_SDL(SDL_Surface *PAL_surface, struct user_info* user_info);
void Save_FRM_Tiles_OpenGL(LF* F_Prop, struct user_info* user_info);

void Save_MSK_Tiles_SDL(SDL_Surface* MSK_surface, struct user_info* user_info);
void Save_MSK_Tiles_OpenGL(image_data* img_data, struct user_info* user_info);

void Save_MSK_Image_SDL(SDL_Surface* surface, FILE* File_ptr, int x, int y);
void Save_MSK_Image_OpenGL(image_data* img_data, FILE* File_ptr);

void Split_to_Tiles_SDL(SDL_Surface *surface, struct user_info* user_info, img_type type, FRM_Header* FRM_Header);
void Split_to_Tiles_OpenGL(image_data* img_data, struct user_info* user_info, img_type type, FRM_Header* frm_header);

void check_file(img_type type, FILE* File_ptr, char* path, char* buffer, int tile_num, char* Save_File_Name);
char* Create_File_Name(img_type type, char* path, int tile_num, char* Save_File_Name);

