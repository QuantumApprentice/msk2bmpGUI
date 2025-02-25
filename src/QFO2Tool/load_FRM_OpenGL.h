#pragma once
#include <glad/glad.h>
#include <stdlib.h>

#include "imgui.h"
#include "Load_Animation.h"
#include "Load_Settings.h"

struct mesh {
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;
    GLuint vertexCount = 0;
};


#pragma pack(push, 1)
struct FRM_Header {
    uint32_t version = 0;                       // 0x0000
    uint16_t FPS = 0;                           // 0x0004
    uint16_t Action_Frame = 0;                  // 0x0006
    uint16_t Frames_Per_Orient = 0;             // 0x0008
    int16_t  Shift_Orient_x[6] = {};            // 0x000A
    int16_t  Shift_Orient_y[6] = {};            // 0x0016
    uint32_t Frame_0_Offset[6] = {};            // 0x0022
    uint32_t Frame_Area = 0;                    // 0x003A
};

struct FRM_Frame {
    uint16_t Frame_Width;                       // 0x003E
    uint16_t Frame_Height;                      // 0x0040
    uint32_t Frame_Size;                        // 0x0042
    int16_t  Shift_Offset_x;                    // 0x0046
    int16_t  Shift_Offset_y;                    // 0x0048
    uint8_t  frame_start[];
};
#pragma pack(pop)

struct FRM_Dir {
    int num_frames  = 0;
    Direction orientation = no_data;
    FRM_Frame** frame_data = NULL;
    rectangle* bounding_box = {};
};

struct image_data {
    FRM_Header* FRM_hdr = NULL;     //same as FRM_data
    FRM_Dir*    FRM_dir = NULL;     //TODO: remove
    rectangle FRM_bounding_box[6];  //TODO: remove

    ANM_Header* ANM_hdr = NULL;
    ANM_Dir*    ANM_dir = NULL;
    rectangle ANM_bounding_box[6];

    img_type type     = UNK;
    uint8_t* FRM_data = NULL;       //same as FRM_hdr
    uint8_t* MSK_data = NULL;
    Surface* MSK_srfc = NULL;

    GLuint FRM_texture;
    GLuint MSK_texture;
    GLuint PAL_texture;
    GLuint render_texture;
    GLuint framebuffer;
    int width;
    int height;
    int display_frame_num;
    int display_orient_num;
    int playback_speed;
    int alpha_threshold = 5;
    int FRM_size = 0;

    float scale = 1.0;

    ImVec2 offset{};
};

//FRM loading
bool framebuffer_init(GLuint* texture, GLuint* framebuffer, int w, int h);
bool init_framebuffer(struct image_data* img_data);
bool load_FRM_OpenGL(const char* file_name, image_data* img_data, shader_info* shaders);

void calculate_bounding_box(rectangle* bounding_box, rectangle* FRM_bounding_box, FRM_Frame* frame_start, rectangle* box);//, FRM_Dir* frm_dir, int i, int j);
void calculate_bounding_box_SURFACE(rectangle* bounding_box, rectangle* FRM_bounding_box, ANM_Frame* anm_frame, rectangle* box);

bool Render_FRM0_OpenGL(image_data* img_data, int dir);
