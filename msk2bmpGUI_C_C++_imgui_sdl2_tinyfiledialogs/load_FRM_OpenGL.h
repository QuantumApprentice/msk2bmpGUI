#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>

#include "imgui-docking/imgui.h"

struct mesh {
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;
    GLuint vertexCount = 0;
};

#pragma pack(push, 1)
typedef struct {
    uint32_t version;                                                 // 0x0000
    uint16_t FPS = 0;                                                 // 0x0004
    uint16_t Action_Frame = 0;                                        // 0x0006
    uint16_t Frames_Per_Orientation;                                  // 0x0008
    uint16_t* Shift_Orient_x = (uint16_t*)malloc(6*sizeof(uint16_t)); // 0x000A
    uint16_t* Shift_Orient_y = (uint16_t*)malloc(6*sizeof(uint16_t)); // 0x0016
    uint32_t* Frame_0_Offset = (uint32_t*)malloc(6*sizeof(uint32_t)); // 0x0022
    uint32_t Frame_Area;                                              // 0x003A
    uint16_t Frame_0_Width;                                           // 0x003E
    uint16_t Frame_0_Height;                                          // 0x0040
    uint32_t Frame_0_Size;                                            // 0x0042
    int16_t  Shift_Offset_x = 0;                                       // 0x0046
    int16_t  Shift_Offset_y = 0;                                       // 0x0048
    //uint8_t  Color_Index = 0;                                       // 0x004A
} FRM_Header;
#pragma pack(pop)

struct image_data {
    FRM_Header FRM_Info{};

    uint8_t* FRM_data = NULL;
    uint8_t* MSK_data = NULL;

    GLuint FRM_texture;
    GLuint MSK_texture;
    GLuint PAL_texture;
    GLuint render_texture;
    GLuint framebuffer;
    int width;
    int height;

    float scale = 1.0;

    ImVec2 offset{};
};

//FRM loading
bool init_framebuffer(struct image_data* img_data);
bool load_FRM_OpenGL(const char* file_name, image_data* img_data);
