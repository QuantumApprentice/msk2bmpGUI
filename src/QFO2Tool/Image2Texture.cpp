// #include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"
#include "ImGui_Warning.h"


bool copy_it_all_ANM(image_data* src, image_data* dst)
{
    ////////////////////////////////////////////////////////
    //copy the FRM_data pointer and all data
    //  (legacy right now, not sure if I want to)
    //  (keep this or do something with it)
    //  (right now it does nothing)
    dst->FRM_data = (uint8_t*)malloc(src->FRM_size);
    if (!dst->FRM_data) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] copy_it_all_ANM()\n\n"
            "Unable to allocate memory for dst->FRM_data."
        );
        printf("Unable to allocate memory for dst->FRM_data: %d", __LINE__);
        return false;
    }
    memcpy(dst->FRM_data, src->FRM_data, src->FRM_size);
    FRM_Header* header = (FRM_Header*)dst->FRM_data;
    ////////////////////////////////////////////////////////

    int num_orients = (header->Frame_0_Offset[1]) ? 6 : 1;
    int num_frames  = header->Frames_Per_Orient;
    if (num_orients < 6) {
        dst->display_orient_num = src->display_orient_num;
    }

    dst->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir) * 6);
    if (!dst->ANM_dir) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] copy_it_all_ANM()\n\n"
            "Unable to allocate memory for ANM_dir."
        );
        printf("Unable to allocate memory for ANM_dir: %d", __LINE__);
        return false;
    }
    new(dst->ANM_dir) ANM_Dir[6];

    ANM_Dir* src_dir = src->ANM_dir;
    ANM_Dir* dst_dir = dst->ANM_dir;
    for (int i = 0; i < num_orients; i++) {
        if (num_orients < 6) {
            i = dst->display_orient_num;
        }
        dst_dir[i].orientation = (Direction)i;
        dst_dir[i].num_frames  = num_frames;

        if (src->ANM_dir[i].frame_data == NULL) {
            break;
        }

        dst_dir[i].frame_box = (rectangle*)malloc(sizeof(rectangle)  * num_frames);
        if (!dst_dir[i].frame_box) {
            set_popup_warning(
                "[ERROR] copy_it_all_ANM()\n\n"
                "Unable to allocate memory for anm_dir[i].bounding_box."
            );
            printf("Unable to allocate memory for anm_dir[%d].bounding_box: %d", i, __LINE__);
            return false;
        }
        memcpy(dst_dir[i].frame_box, src_dir[i].frame_box, sizeof(rectangle)*num_frames);

        // allocate frame space on dst, copy from image_data to dst
        dst_dir[i].frame_data = (Surface**)malloc(sizeof(Surface*)*num_frames);
        if (!dst_dir[i].frame_data) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] copy_it_all_ANM()\n\n"
                "Unable to allocate memory for dst_dir[i].frame_data."
            );
            printf("Unable to allocate memory for dst_dir[%d].frame_data: %d", i, __LINE__);
            return false;
        }

        for (int j = 0; j < num_frames; j++) {
            // duplicate surface and assign to dst
            Surface* src_srfc = src_dir[i].frame_data[j];
            Surface* dst_srfc = Copy8BitSurface(src_srfc);
            if (dst_srfc == NULL) {
                //TODO: log out to file
                set_popup_warning(
                    "[ERROR] copy_it_all_ANM()\n\n"
                    "Unable to allocate memory for Surface* dst."
                );
                printf("Unable to allocate memory for Surface* dst: %d", __LINE__);
                return false;
            }
            dst_dir[i].frame_data[j] = dst_srfc;
        }
    }

    memcpy(dst->ANM_bounding_box, src->ANM_bounding_box, sizeof(rectangle[6]));
    dst->FRM_hdr = (FRM_Header*)dst->FRM_data;

    return true;
}

void prep_image_SURFACE(LF* F_Prop, Palette* pal, int color_match_algo, bool* window, bool alpha)
{
    image_data* src = &F_Prop->img_data;
    image_data* dst = &F_Prop->edit_data;

    dst->width  = src->width;
    dst->height = src->height;
    dst->scale  = src->scale;
    dst->offset = src->offset;

    int dir = src->display_orient_num = dst->display_orient_num = src->display_orient_num;

#pragma region copy_it_all
    if (src->type == FRM || src->type == MSK) {
        dst->type = src->type;

        if (src->type == FRM) {
            //FRM needs full tree copy
            //uses dst.FRM_texture
            dst->FRM_size = src->FRM_size;
            copy_it_all_ANM(src, dst);
            dst->FRM_texture = init_texture(
                dst->ANM_dir[src->display_orient_num].frame_data[0],
                dst->width,
                dst->height,
                dst->type);
        }
        if (src->type == MSK) {
            //MSK just needs a surface copy,
            //uses dst.MSK_texture
            dst->MSK_data = src->MSK_data;
            dst->MSK_srfc = Copy8BitSurface(src->MSK_srfc);
            dst->MSK_texture = init_texture(
                dst->MSK_srfc,
                dst->width,
                dst->height,
                dst->type);
            dst->ANM_bounding_box[dir].x1;
            dst->ANM_bounding_box[dir].y1;
            dst->ANM_bounding_box[dir].x2 = src->width;
            dst->ANM_bounding_box[dir].y2 = src->height;
        }
    }
    if (src->type == OTHER) {
        dst->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir)*6);
        if (!dst->ANM_dir) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] prep_image_SURFACE()\n\n"
                "Unable to allocate memory for ANM_dir."
            );
            printf("Unable to allocate memory for ANM_dir: %d", __LINE__);
            return;
        }
        new(dst->ANM_dir) ANM_Dir[6];

        dst->ANM_dir[dir].frame_data = (Surface**)malloc(sizeof(Surface*));
        if (!dst->ANM_dir[dir].frame_data) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] prep_image_SURFACE()\n\n"
                "Unable to allocate memory for frame_data."
            );
            printf("Unable to allocate memory for frame_data: %d", __LINE__);
            return;
        }

        dst->ANM_dir[dir].frame_data[0] = PAL_Color_Convert(
            src->ANM_dir[dir].frame_data[0],
            pal, color_match_algo);

        if (!dst->ANM_dir[dir].frame_data[0]) {
            //TODO: log out to file
            //PAL_Color_Convert() has its own warning popup
            // printf("Unable to allocate memory for ANM_dir: %d", __LINE__);
            return;
        }

        dst->ANM_dir[dir].frame_box = (rectangle*)calloc(1, sizeof(rectangle));
        if (!dst->ANM_dir[dir].frame_box) {
            set_popup_warning(
                "[ERROR] prep_image_SURFACE()\n\n"
                "Unable to allocate memory for ANM_dir[dir].frame_box."
            );
            printf("Unable to allocate memory for ANM_dir[%d].frame_box: %d", dir, __LINE__);
            return;
        }
        dst->ANM_dir[dir].frame_box->x2 = dst->width;
        dst->ANM_dir[dir].frame_box->y2 = dst->height;
        dst->ANM_bounding_box[dir].x2   = dst->width;
        dst->ANM_bounding_box[dir].y2   = dst->height;

        dst->ANM_dir[dir].orientation = NE;

        dst->FRM_hdr = (FRM_Header*)calloc(1, sizeof(FRM_Header));
        if (!dst->FRM_hdr) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] prep_image_SURFACE()\n\n"
                "Unable to allocate memory for FRM_hdr."
            );
            printf("Unable to allocate memory for FRM_hdr: %d", __LINE__);
            return;
        }

        dst->type = FRM;

        dst->FRM_texture = init_texture(
            dst->ANM_dir[dir].frame_data[0],
            dst->width,
            dst->height,
            dst->type);

        int size = dst->width * dst->height;

        dst->ANM_dir[dir].num_frames = 1;

        //TODO: this needs to work with ANM_dir[]
        //disable/enable alpha channel?
        if (alpha) {
            for (int i = 0; i < size; i++) {
                // if (dst->FRM_dir->frame_data[0]->frame_start[i] == 0) {
                //     dst->FRM_dir->frame_data[0]->frame_start[i] = 1;
                // }
            }
        }
    }

    bool success = framebuffer_init(
        &dst->render_texture,
        &dst->framebuffer,
        dst->width, dst->height);
    if (!success) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] prep_image_SURFACE()\n\n"
            "Framebuffer failed to attach correctly."
        );
        printf("[ERROR] Image framebuffer failed to attach correctly?\n");
        printf("glError: %d\n", glGetError());
        return;
    }
    //open edit window
    *window = true;
}

GLuint init_texture(Surface* src, int w, int h, img_type type)
{
    if (!src) {
        return false;
    }
    if (!src->pxls) {
        return false;
    }

    int alignment = 1;      //FRM & MSK
    int pxl_type  = GL_RED; //FRM & MSK //6403
    if (type == OTHER) {    //everything else
        alignment = 4;
        pxl_type  = GL_RGBA;            //6408
    }

    GLuint texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    //have to glBindTexture() before glIsTexture()
    if (!glIsTexture(texture)) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] init_texture()\n\n"
            "Failed to allocate texture."
        );
        printf("[ERROR] Failed to allocate texture\n");
        return false;
    }
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
    //control alignment of the image (FRM/MSK are 1-byte aligned) when converted to texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    //bind FRM_data to FRM_texture for "indirect" editing
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, pxl_type, GL_UNSIGNED_BYTE, src->pxls);

    return texture;
}

bool checkbox_handler(const char* text, bool* alpha)
{
    ImGui::Checkbox(text, alpha);
    if (*alpha) {
        return false;
    }
    return true;
}