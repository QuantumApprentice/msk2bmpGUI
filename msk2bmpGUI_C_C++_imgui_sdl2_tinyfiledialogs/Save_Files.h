#pragma once

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

enum flag {
    MSK = 0,
    FRM = 1
};

char* Save_FRM(SDL_Surface *f_surface, struct user_info* user_info);
char* Save_IMG(SDL_Surface *b_surface, struct user_info* user_info);
void Save_FRM_tiles(SDL_Surface *PAL_surface, struct user_info* user_info);
void Save_Map_Mask(SDL_Surface* MSK_surface, struct user_info* user_info);

wchar_t* Create_File_Name(bool type, char* Save_File_Name, char* buffer, int q);
void Split_to_Tiles(SDL_Surface *surface, struct user_info* user_info, bool type, FRM_Header* FRM_Header);
