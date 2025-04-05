#include "load_FRM_OpenGL.h"
#include "B_Endian.h"
#include "ImGui_Warning.h"

#include "tinyfiledialogs.h"
#include "Load_Files.h"
#include "display_FRM_OpenGL.h"
#include "Image2Texture.h"
#include "platform_io.h"

bool load_FRM_to_SURFACE(const char* file, image_data* img_data, shader_info* shaders);

bool framebuffer_init(GLuint* texture, GLuint* framebuffer, int w, int h)
{
    glDeleteFramebuffers(1, framebuffer);
    if (!glIsTexture(*texture)) {
        glGenTextures(1, texture);
    }
    glBindTexture(GL_TEXTURE_2D, *texture);
    //set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //allocate video memory for texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    //init framebuffer
    glGenFramebuffers(1, framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);

    //attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
    glViewport(0, 0, w, h);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        set_popup_warning(
            "[ERROR] framebuffer_init()\n\n"
            "Framebuffer status is not complete."
        );
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        //reset texture bind to default
        glBindTexture(GL_TEXTURE_2D, 0);
        return false;
    }

    //reset texture and framebuffer bind to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

uint8_t* load_entire_file(const char* file_name, int* file_size)
{
    FILE* File_ptr;
    int file_length = 0;

#ifdef QFO2_WINDOWS
    errno_t err = _wfopen_s(&File_ptr, io_utf8_wchar(file_name), L"rb");
#elif defined(QFO2_LINUX)
    File_ptr = fopen(file_name, "rb");
#endif

    if (!File_ptr) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] load_entire_file()\n\n"
            "Can't open file."
        );

#ifdef QFO2_WINDOWS
        printf("error, can't open FRM file, error: %d : L%d", err, __LINE__);
        if (err == 13) {
            printf("file opened by another program? : L%d", __LINE__);
        }
#elif defined(QFO2_LINUX)
        printf("Error: load_entire_file(), Can't open FRM file, error: %d\t%s : L%d\n", errno, strerror(errno), __LINE__);
#endif
        return NULL;
    }

    int error = fseek(File_ptr, 0, SEEK_END);
    if (error) {
        //TODO: maybe put a popup warning here?
        return NULL;
    }
    file_length = ftell(File_ptr);
    fseek(File_ptr, 0, SEEK_SET);

    if (file_length < 1) {
        //TODO: maybe put a popup warning here?
        return NULL;
    }
    uint8_t* buffer = (uint8_t*)malloc(file_length);
    fread(buffer, file_length, 1, File_ptr);
    fclose(File_ptr);

    if (file_size != NULL) {
        *file_size = file_length;
    }

    return buffer;
}

//#include <algorithm>
//rectangle Union(rectangle a, rectangle b)
//{
//    int top     = std::min(a.y1, b.y1);
//    int left    = std::min(a.x1, b.x1);
//    int bottom  = std::max(a.y2, b.y2);
//    int right   = std::max(a.x2, b.x2);
//
//    rectangle result = { top, left, bottom, right };
//
//    return result;
//}

//used in crop_animation_SURFACE() in Edit_Animation.cpp
void calculate_bounding_box_SURFACE(
        rectangle* bounding_box, rectangle* FRM_bounding_box,
        Surface* anm_frame, rectangle* box)
{
    bounding_box->x1 += anm_frame->x - anm_frame->w/2;
    bounding_box->y1 += anm_frame->y - anm_frame->h;
    bounding_box->x2  = bounding_box->x1 + anm_frame->w;
    bounding_box->y2  = bounding_box->y1 + anm_frame->h;

    *box = *bounding_box;
    if (bounding_box->x1 < FRM_bounding_box->x1) {
        FRM_bounding_box->x1 = bounding_box->x1;
    }
    if (bounding_box->y1 < FRM_bounding_box->y1) {
        FRM_bounding_box->y1 = bounding_box->y1;
    }
    if (bounding_box->x2 > FRM_bounding_box->x2) {
        FRM_bounding_box->x2 = bounding_box->x2;
    }
    if (bounding_box->y2 > FRM_bounding_box->y2) {
        FRM_bounding_box->y2 = bounding_box->y2;
    }

    bounding_box->x1 += anm_frame->w / 2;
    bounding_box->y1 += anm_frame->h;
}

//used in load_FRM_to_SURFACE() here
void calculate_bounding_box(rectangle* bounding_box, rectangle* FRM_bounding_box,
                            FRM_Frame* frame_start, rectangle* box)
{
    bounding_box->x1 += frame_start->Shift_Offset_x - frame_start->Frame_Width / 2;
    bounding_box->y1 += frame_start->Shift_Offset_y - frame_start->Frame_Height;

    bounding_box->x2 = bounding_box->x1 + frame_start->Frame_Width;
    bounding_box->y2 = bounding_box->y1 + frame_start->Frame_Height;

    *box = *bounding_box;

    if (bounding_box->x1 < FRM_bounding_box->x1) {
        FRM_bounding_box->x1 = bounding_box->x1;
    }
    if (bounding_box->y1 < FRM_bounding_box->y1) {
        FRM_bounding_box->y1 = bounding_box->y1;
    }
    if (bounding_box->x2 > FRM_bounding_box->x2) {
        FRM_bounding_box->x2 = bounding_box->x2;
    }
    if (bounding_box->y2 > FRM_bounding_box->y2) {
        FRM_bounding_box->y2 = bounding_box->y2;
    }

    bounding_box->x1 += frame_start->Frame_Width / 2;
    bounding_box->y1 += frame_start->Frame_Height;
}

Direction assign_direction_FRM(const char* direction)
{
    if (!io_strncmp(direction, "FR0\n", sizeof("FR0\n"))) {
        return NE;
    }
    if (!io_strncmp(direction, "FR1\0", sizeof("FR1\0"))) {
        return E;
    }
    if (!io_strncmp(direction, "FR2\0", sizeof("FR2\0"))) {
        return SE;
    }
    if (!io_strncmp(direction, "FR3\0", sizeof("FR3\0"))) {
        return SW;
    }
    if (!io_strncmp(direction, "FR4\0", sizeof("FR4\0"))) {
        return W;
    }
    if (!io_strncmp(direction, "FR5\0", sizeof("FR5\0"))) {
        return NW;
    }
    //default
    return NE;
}

bool load_FRM_to_SURFACE(const char* file, image_data* img_data, shader_info* shaders)
{
    uint8_t* buffer = load_entire_file(file, &img_data->FRM_size);
    if (!buffer) {
        return false;
    }
    FRM_Header* header = (FRM_Header*)buffer;
    B_Endian::flip_header_endian(header);
    img_data->FRM_hdr = header;

    int num_orients = (header->Frame_0_Offset[1]) ? 6 : 1;
    int num_frames  = header->Frames_Per_Orient;
    Direction dir   = no_data;
    const char* ext_ptr = strrchr(file, '.') + 1;
    if (num_orients < 6) {
        dir = assign_direction_FRM(ext_ptr);
        img_data->display_orient_num = dir;
    }

    img_data->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir)*6);
    if (!img_data->ANM_dir) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] load_FRM_to_SURFACE()\n\n"
            "Unable to allocate memory for ANM_dir."
        );
        printf("Unable to allocate memory for ANM_dir: %d", __LINE__);
        free(buffer);
        img_data->FRM_hdr = NULL;
        return false;
    }
    new(img_data->ANM_dir) ANM_Dir[6];


    ANM_Dir* anm_dir = img_data->ANM_dir;
    int buff_offset  = sizeof(FRM_Header);
    for (int i = 0; i < num_orients; i++)
    {
        if (num_orients < 6) {
            i = dir;
        }

        anm_dir[i].num_frames  = num_frames;
        anm_dir[i].orientation = (Direction)i;

        //TODO: change to ptr assignment after malloc-ing entire memory above ^^
        anm_dir[i].frame_data  = (Surface**)malloc(sizeof(Surface*) * num_frames);
        if (!anm_dir[i].frame_data) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] load_FRM_to_SURFACE()\n\n"
                "Unable to allocate memory for anm_dir[i].frame_data"
            );
            printf("Unable to allocate memory for anm_dir[%d].frame_data: %d", i, __LINE__);
            free(anm_dir);
            free(buffer);
            img_data->FRM_hdr = NULL;
            return false;
        }
        anm_dir[i].frame_box = (rectangle*)malloc(sizeof(rectangle)  * num_frames);
        if (!anm_dir[i].frame_box) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] load_FRM_to_SURFACE()\n\n"
                "Unable to allocate memory for anm_dir[i].frame_data"
            );
            printf("Unable to allocate memory for anm_dir[%d].bounding_box: %d", i, __LINE__);
            for (int i = 0; i < 6; i++) {
                free(anm_dir[i].frame_data);
            }
            free(anm_dir);
            free(buffer);
            img_data->FRM_hdr = NULL;
            return false;
        }

        rectangle frame_box         = {};
        rectangle FRM_bounding_box  = {};

        for (int j = 0; j < num_frames; j++) {
            FRM_Frame* frame_start = (FRM_Frame*)(buffer + buff_offset);
            B_Endian::flip_frame_endian(frame_start);
            calculate_bounding_box(&frame_box, &FRM_bounding_box, frame_start, &anm_dir[i].frame_box[j]);

            int w = frame_start->Frame_Width;
            int h = frame_start->Frame_Height;
            Surface* img = Create_8Bit_Surface(w, h, shaders->FO_pal);
            if (!img) {
                //TODO: log out to file
                set_popup_warning(
                    "[ERROR] load_FRM_to_SURFACE()\n\n"
                    "Unable to allocate memory for Surface."
                );
                printf("Unable to allocate memory for Surface: %d", __LINE__);
                for (int i = 0; i < 6; i++) {
                    free(anm_dir[i].frame_data);
                    free(anm_dir[i].frame_box);
                }
                free(anm_dir);
                free(buffer);
                img_data->FRM_hdr = NULL;
                return false;
            }

            memcpy(img->pxls, frame_start->frame_start, w*h);

            anm_dir[i].frame_data[j] = img;
            anm_dir[i].frame_data[j]->x = frame_start->Shift_Offset_x;
            anm_dir[i].frame_data[j]->y = frame_start->Shift_Offset_y;

            buff_offset += frame_start->Frame_Size + sizeof(FRM_Frame);
        }
        img_data->ANM_bounding_box[i] = FRM_bounding_box;
    }

    int this_dir = (num_orients < 6) ? dir : 0;
    img_data->width  = img_data->ANM_bounding_box[this_dir].x2 - img_data->ANM_bounding_box[this_dir].x1;
    img_data->height = img_data->ANM_bounding_box[this_dir].y2 - img_data->ANM_bounding_box[this_dir].y1;

    img_data->FRM_data = buffer;
    return true;
}

//TODO: replace with surface equivalent?
//load FRM image from char* file_name
//stores GLuint and size info to img_data
//returns true on success, else false
bool load_FRM_OpenGL(const char* file_name, image_data* img_data, shader_info* shaders)
{
    //read in FRM data including animation frames
    bool success = load_FRM_to_SURFACE(file_name, img_data, shaders);
    if (!success) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] load_FRM_OpenGL()\n\n"
            "Couldn't load FRM image data."
        );
        printf("Couldn't load FRM image data: %d", __LINE__);
        return false;
    }
    img_data->display_frame_num = 0;
    int dir = img_data->display_orient_num;


    img_data->FRM_texture = init_texture(
        img_data->ANM_dir[dir].frame_data[0],
        img_data->width,
        img_data->height,
        img_data->type);

    if (!img_data->FRM_texture) {
        //TODO: log out to file
        //init_texture() has its own popup warning
        printf("init_texture failed: %d", __LINE__);
        return false;
    }

    success = framebuffer_init(
        &img_data->render_texture,
        &img_data->framebuffer,
        img_data->width, img_data->height);
    if (!success) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] load_FRM_OpenGL()\n\n"
            "framebuffer_init failed."
        );
        printf("framebuffer_init failed: %d", __LINE__);
        return false;
    }

    SURFACE_to_texture(
        img_data->ANM_dir[img_data->display_orient_num].frame_data[0],
        img_data->FRM_texture, img_data->width, img_data->height, 1);

    draw_texture_to_framebuffer(
        shaders->FO_pal, shaders->render_FRM_shader,
        &shaders->giant_triangle, img_data->framebuffer,
        img_data->FRM_texture, img_data->width, img_data->height);

    return success;
}