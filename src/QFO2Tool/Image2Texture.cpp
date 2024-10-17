// #include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"

//Used to convert generic image surfaces to textures
bool Image2Texture(Surface* src, GLuint* texture)
{
    if (src) {
        if (src->channels < 4) {
            Surface* Temp_Surface = NULL;
            Temp_Surface = Convert_Surface_to_RGBA(src);
            // Temp_Surface = Unpalettize_Image(surface);
            Surface_to_OpenGl(Temp_Surface, texture);
            FreeSurface(Temp_Surface);
        } else {
            Surface_to_OpenGl(src, texture);
        }
        return true;
    }
    if (texture == NULL) {
        printf("Error: Unable to optimize image!\n");
        return false;
    }
    return false;
}

void Surface_to_OpenGl(Surface *Surface, GLuint *texture)
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surface->w, Surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Surface->pxls);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    printf("glError: %d\n", glGetError());
}

void init_FRM(image_data* edit_data)
{
    int width  = edit_data->width;
    int height = edit_data->height;
    int size   = width * height;

    edit_data->FRM_hdr = (FRM_Header*)malloc(sizeof(FRM_Header));
    new(edit_data->FRM_hdr) FRM_Header;

    edit_data->FRM_hdr->Frame_Area = size + sizeof(FRM_Frame);
    edit_data->FRM_dir = (FRM_Dir*)malloc(sizeof(FRM_Dir) * 6);
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

//Palettize to 8-bit FO pallet, and dither
// void Prep_Image(LF* F_Prop, SDL_PixelFormat* pxlFMT_FO_Pal, int color_match, bool* window, bool alpha_off) {
void Prep_Image(LF* F_Prop, Palette* palette, int color_match_algo, bool* window, bool alpha_off) {

    if (F_Prop->img_data.type == FRM) {
        //copy the FRM_data pointer for editing
        //TODO: need to allocate and copy by entire FRM_data
        //      need to allocate and assign FRM_dir and FRM_frame pointers

        //F_Prop->edit_data.FRM_data = F_Prop->img_data.FRM_data;

        F_Prop->edit_data.FRM_size = F_Prop->img_data.FRM_size;
        F_Prop->edit_data.width    = F_Prop->img_data.width;
        F_Prop->edit_data.height   = F_Prop->img_data.height;
        F_Prop->edit_data.scale    = F_Prop->img_data.scale;
        F_Prop->edit_data.offset   = F_Prop->img_data.offset;

        copy_it_all(&F_Prop->img_data, &F_Prop->edit_data);

        F_Prop->edit_data.type = FRM;

        //bind edit data for editing
        bind_NULL_texture(&F_Prop->edit_data, NULL, F_Prop->img_data.type);
        //set edit window bool to true, opens edit window
        *window = true;

    }
    else if (F_Prop->img_data.type == MSK) {
        //copy the MSK_data pointer for editing
        //TODO: need to copy by value instead
        F_Prop->edit_data.MSK_data = F_Prop->img_data.MSK_data;

        F_Prop->edit_data.width    = F_Prop->img_data.width;
        F_Prop->edit_data.height   = F_Prop->img_data.height;
        F_Prop->edit_data.scale    = F_Prop->img_data.scale;
        F_Prop->edit_data.offset   = F_Prop->img_data.offset;

        //bind edit data for editing
        bind_NULL_texture(&F_Prop->edit_data, NULL, F_Prop->img_data.type);

        //set edit window bool to true, opens edit window
        *window = true;

    }
    else {
        F_Prop->edit_data.FRM_data
            = FRM_Color_Convert(F_Prop->img_data.ANM_dir->frame_data->frame_start,
                                palette, color_match_algo);
        F_Prop->edit_data.type = FRM;
        int width  = F_Prop->img_data.ANM_dir->frame_data->frame_start->w;
        int height = F_Prop->img_data.ANM_dir->frame_data->frame_start->h;
        int size   = width * height;

        F_Prop->edit_data.scale  = F_Prop->img_data.scale;
        F_Prop->edit_data.offset = F_Prop->img_data.offset;

        F_Prop->edit_data.width  = width;
        F_Prop->edit_data.height = height;
        //TODO: need to allocate header? info etc?
        //      need to assign FRM_dir[] pointers into FRM_data
        init_FRM(&F_Prop->edit_data);
        F_Prop->edit_data.FRM_dir[0].num_frames = 1;


        if (alpha_off) {
            for (int i = 0; i < size; i++)
            {
                if (F_Prop->edit_data.FRM_dir->frame_data[0]->frame_start[i] == 0) {
                    F_Prop->edit_data.FRM_dir->frame_data[0]->frame_start[i] = 1;
                }
            }
        }
        //bind edit data for editing
        bind_NULL_texture(&F_Prop->edit_data, NULL, FRM);
        //set edit window bool to true, opens edit window
        *window = true;
    }
}

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
        if (img_data->FRM_data) {
            int dir = img_data->display_orient_num;
            uint8_t* data = img_data->FRM_dir[dir].frame_data[0]->frame_start;
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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_data->width, img_data->height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        }
        else {
            printf("FRM_texture didn't load?...\n");
            return false;
        }
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
    success = init_framebuffer(img_data);
    if (!success) {
        printf("image framebuffer failed to attach correctly?\n");
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
    else {
        return true;
    }
}