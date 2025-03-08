// #include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"
#include "ImGui_Warning.h"



//TODO: delete this? (no longer used?)
//Used to convert generic image surfaces to textures
bool Image2Texture(Surface* src, GLuint* texture)
{
    if (!src) {
        return false;
    }
    if (src->channels < 4) {
        Surface* Temp_Surface = NULL;
        Temp_Surface = Convert_Surface_to_RGBA(src);
        // Temp_Surface = Unpalettize_Image(surface);
        Surface_to_OpenGl(Temp_Surface, texture);
        FreeSurface(Temp_Surface);
    } else {
        Surface_to_OpenGl(src, texture);
    }

    if (!glIsTexture(*texture)) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] Image2Texture()\n\n"
            "Unable to allocate texture."
            );
        printf("Error: Unable to allocate texture.\n");
        printf("glError: %d\n", glGetError());
        return false;
    }
    return true;
}

void Surface_to_OpenGl(Surface *src, GLuint *texture)
{
    // OpenGL conversion from surface to texture
    if (!glIsTexture(*texture)) {
        glDeleteTextures(1, texture);
    }
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, src->w, src->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, src->pxls);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

}

//TODO: delete, replaced by init_texture() and init_framebuffer()
//this is still called in Prep_Image(), which itself has been replaced by prep_image_SURFACE()
void init_FRM(image_data* edit_data)
{
    int width  = edit_data->width;
    int height = edit_data->height;
    int size   = width * height;

    edit_data->FRM_hdr = (FRM_Header*)malloc(sizeof(FRM_Header));
    new(edit_data->FRM_hdr) FRM_Header;

    edit_data->FRM_hdr->Frame_Area = size + sizeof(FRM_Frame);
    edit_data->FRM_dir = (FRM_Dir*)calloc(6,sizeof(FRM_Dir));
    edit_data->FRM_dir[0].frame_data    = (FRM_Frame**)malloc(sizeof(FRM_Frame*));
    edit_data->FRM_dir[0].frame_data[0] = (FRM_Frame*)(edit_data->FRM_data + sizeof(FRM_Header));

    edit_data->FRM_dir[0].frame_data[0]->Frame_Width    = width;
    edit_data->FRM_dir[0].frame_data[0]->Frame_Height   = height;
    edit_data->FRM_dir[0].frame_data[0]->Frame_Size     = size;
    edit_data->FRM_dir[0].frame_data[0]->Shift_Offset_x = 0;
    edit_data->FRM_dir[0].frame_data[0]->Shift_Offset_y = 0;

    edit_data->FRM_dir[0].bounding_box = (rectangle*)malloc(sizeof(rectangle));
    new(edit_data->FRM_dir->bounding_box) rectangle;

    edit_data->FRM_dir[0].bounding_box->x2 = width;
    edit_data->FRM_dir[0].bounding_box->y2 = height;
    edit_data->FRM_bounding_box[0].x2      = width;
    edit_data->FRM_bounding_box[0].y2      = height;

    edit_data->FRM_dir->orientation = NE;


}

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
    // init_FRM(dst);

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

//TODO: delete, replaced by copy_it_all_ANM()
void copy_it_all(image_data* img_data, image_data* edit_data)
{
    edit_data->FRM_data = (uint8_t*)malloc(img_data->FRM_size);
    memcpy(edit_data->FRM_data, img_data->FRM_data, img_data->FRM_size);
    FRM_Header* header = (FRM_Header*)edit_data->FRM_data;

    // init_FRM(edit_data);

    int num_orients = (header->Frame_0_Offset[1]) ? 6 : 1;
    int num_frames  = header->Frames_Per_Orient;
    if (num_orients < 6) {
        edit_data->display_orient_num = img_data->display_orient_num;
    }

    edit_data->FRM_dir = (FRM_Dir*)malloc(sizeof(FRM_Dir) * 6);
    if (!edit_data->FRM_dir) {
        printf("Unable to allocate memory for FRM_dir: %d", __LINE__);
    }
    else {
        new(edit_data->FRM_dir) FRM_Dir[6];
    }

    FRM_Dir* frm_dir = edit_data->FRM_dir;

    for (int i = 0; i < num_orients; i++)
    {
        if (num_orients < 6) {
            i = edit_data->display_orient_num;
        }
        //TODO: change to ptr assignment after malloc-ing entire memory above ^^
        frm_dir[i].frame_data  = (FRM_Frame**)malloc(sizeof(FRM_Frame*) * num_frames);
        if (!frm_dir[i].frame_data) {
            printf("Unable to allocate memory for frm_dir[%d].frame_data: %d", i, __LINE__);
            return;
        }
        frm_dir[i].bounding_box = (rectangle*)malloc(sizeof(rectangle)  * num_frames);
        if (!frm_dir[i].bounding_box) {
            printf("Unable to allocate memory for frm_dir[%d].bounding_box: %d", i, __LINE__);
            return;
        }

        int buff_offset        = header->Frame_0_Offset[i] + sizeof(FRM_Header);
        frm_dir[i].orientation = (Direction)i;
        frm_dir[i].num_frames  = num_frames;

        for (size_t j = 0; j < num_frames; j++)
        {
            FRM_Frame* frame_start     = (FRM_Frame*)(edit_data->FRM_data + buff_offset);
            frm_dir[i].frame_data[j]   = frame_start;
            frm_dir[i].bounding_box[j] = img_data->FRM_dir[i].bounding_box[j];
            buff_offset += frame_start->Frame_Size + sizeof(FRM_Frame);
        }
    }

    memcpy(edit_data->FRM_bounding_box, img_data->FRM_bounding_box, sizeof(rectangle[6]));
    int this_dir = img_data->display_orient_num;
    // edit_data->width  = edit_data->FRM_bounding_box[this_dir].x2 - edit_data->FRM_bounding_box[this_dir].x1;
    // edit_data->height = edit_data->FRM_bounding_box[this_dir].y2 - edit_data->FRM_bounding_box[this_dir].y1;
    edit_data->FRM_hdr = (FRM_Header*)edit_data->FRM_data;
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
                if (dst->FRM_dir->frame_data[0]->frame_start[i] == 0) {
                    dst->FRM_dir->frame_data[0]->frame_start[i] = 1;
                }
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

//TODO: delete Prep_Image(), replaced by prep_image_SURFACE()
//Palettize to 8-bit FO pallet, and dither
// void Prep_Image(LF* F_Prop, SDL_PixelFormat* pxlFMT_FO_Pal, int color_match, bool* window, bool alpha_off) {
void Prep_Image(LF* F_Prop, Palette* palette, int color_match_algo, bool* window, bool alpha_off) {
    image_data* img_data = &F_Prop->img_data;
    image_data* edit_data = &F_Prop->edit_data;

    if (F_Prop->img_data.type == FRM) {
        //copy the FRM_data pointer for editing
        //TODO: need to allocate and copy by entire FRM_data
        //      need to allocate and assign FRM_dir and FRM_frame pointers

        edit_data->FRM_size = img_data->FRM_size;
        edit_data->width    = img_data->width;
        edit_data->height   = img_data->height;
        edit_data->scale    = img_data->scale;
        edit_data->offset   = img_data->offset;

#pragma region copy_it_all
        // copy_it_all(img_data, edit_data);
        copy_it_all_ANM(img_data, edit_data);

        edit_data->type = FRM;

        //bind edit data for editing
        bind_NULL_texture(edit_data, NULL, edit_data->type);
        //set edit window bool to true, opens edit window
        *window = true;

    }
    else if (F_Prop->img_data.type == MSK) {
        //copy the MSK_data pointer for editing
        //TODO: need to copy by value instead
        edit_data->MSK_data = img_data->MSK_data;
        edit_data->MSK_srfc = Copy8BitSurface(img_data->MSK_srfc);

        edit_data->width    = img_data->width;
        edit_data->height   = img_data->height;
        edit_data->scale    = img_data->scale;
        edit_data->offset   = img_data->offset;
        edit_data->type     = MSK;

        //bind edit data for editing
        // bind_NULL_texture(edit_data, NULL, img_data->type);
        edit_data->FRM_texture = init_texture(
            edit_data->MSK_srfc,
            edit_data->width,
            edit_data->height,
            edit_data->type);


        bool success = framebuffer_init(
            &edit_data->render_texture,
            &edit_data->framebuffer,
            edit_data->width, edit_data->height);
        if (!success) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] init_texture()\n\n"
                "Framebuffer failed to attach correctly."
            );
            printf("[ERROR] Image framebuffer failed to attach correctly?\n");
            printf("glError: %d\n", glGetError());
            return;
        }

        //set edit window bool to true, opens edit window
        *window = true;

    }
    else {
        // edit_data->FRM_data
        //     = FRM_Color_Convert(&img_data->ANM_dir[0]frame_data[0],
        //                         palette, color_match_algo);
        edit_data->type = FRM;
        int width  = img_data->ANM_dir[0].frame_data[0]->w;
        int height = img_data->ANM_dir[0].frame_data[0]->h;
        int size   = width * height;

        edit_data->scale  = img_data->scale;
        edit_data->offset = img_data->offset;

        edit_data->width  = width;
        edit_data->height = height;
        //TODO: need to allocate header? info etc?
        //      need to assign FRM_dir[] pointers into FRM_data
        init_FRM(edit_data);
        edit_data->FRM_dir[0].num_frames = 1;


        if (alpha_off) {
            for (int i = 0; i < size; i++)
            {
                if (edit_data->FRM_dir->frame_data[0]->frame_start[i] == 0) {
                    edit_data->FRM_dir->frame_data[0]->frame_start[i] = 1;
                }
            }
        }
        //bind edit data for editing
        bind_NULL_texture(edit_data, NULL, FRM);
        //set edit window bool to true, opens edit window
        *window = true;
    }
}

//TODO: this should replace bind_NULL_texture()
GLuint init_texture(Surface* src, int w, int h, img_type type)
{
    if (!src) {
        return false;
    }
    if (!src->pxls) {
        return false;
    }

    int alignment = 1;      //FRM & MSK
    int pxl_type  = GL_RED; //FRM & MSK
    if (type == OTHER) {    //everything else
        alignment = 4;
        pxl_type  = GL_RGBA;
    }

    GLuint texture = 0;

    // int dir = img_data->display_orient_num;
    // uint8_t* data = img_data->FRM_dir[dir].frame_data[0]->frame_start;
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

//TODO: delete bind_NULL_texture() - replaced by init_texture() and 
// binds the image information to FRM_texture
// and sets up PAL_texture with a NULL texture of appropriate size for editing
bool bind_NULL_texture(struct image_data* img_data, Surface* surface, img_type type)
{
    if (surface) {
        if (surface->pxls) {
            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
            //control alignment of the image (SDL auto aligned to 4-bytes) when converted to texture
            //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            //load & gen texture
            glGenTextures(1, &img_data->FRM_texture);
            glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
            //texture settings
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            uint8_t* data = img_data->FRM_dir->frame_data[0]->frame_start;

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        }
        else {
            printf("surface.pixels image didn't load...\n");
            return false;
        }
    }
    else if (type == FRM) {
        if (!img_data->ANM_dir) {
            printf("wtf?");
            return false;
        }

        int dir = img_data->display_orient_num;
        Surface* srfc = img_data->ANM_dir[dir].frame_data[0];
        glGenTextures(1, &img_data->FRM_texture);
        glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
        //texture settings
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //control alignment of the image (FRM data needs 1-byte) when converted to texture
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //bind FRM_data to FRM_texture for "indirect" editing
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, srfc->w, srfc->h, 0, GL_RED, GL_UNSIGNED_BYTE, srfc->pxls);





        // if (img_data->FRM_data) {
        //     int dir = img_data->display_orient_num;
        //     uint8_t* data = img_data->FRM_dir[dir].frame_data[0]->frame_start;
        //     glGenTextures(1, &img_data->FRM_texture);
        //     glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
        //     //texture settings
        //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //     //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //     //control alignment of the image (FRM data needs 1-byte) when converted to texture
        //     glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //     //bind FRM_data to FRM_texture for "indirect" editing
        //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_data->width, img_data->height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        // }

    }
    else if (type == MSK) {
        if (img_data->MSK_data) {

            glGenTextures(1, &img_data->MSK_texture);
            glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
            //texture settings
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
            //control alignment of the image (FRM data needs 1-byte) when converted to texture
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            //bind FRM_data to FRM_texture for "indirect" editing
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_data->width, img_data->height, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->MSK_data);
        }
        else {
            printf("MSK_texture didn't load?...\n");
            return false;
        }
    }

    //bind NULL texture to PAL_texture for "direct" editing
    glGenTextures(1, &img_data->PAL_texture);
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //set texture to NULL for drawing to
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img_data->width, img_data->height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    bool success = false;
    success = init_framebuffer(img_data);   //TODO: swap for framebuffer_init()
    if (!success) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] bind_NULL_texture()\n\n"
            "framebuffer failed to attach correctly."
        );
        printf("[ERROR] Image framebuffer failed to attach correctly?\n");
        return false;
    }
    return true;

}

bool checkbox_handler(const char* text, bool* alpha)
{
    ImGui::Checkbox(text, alpha);
    if (*alpha) {
        return false;
    }
    return true;
}