#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"


//Used to convert generic images loaded with SDL
void Image2Texture(SDL_Surface* surface, GLuint* texture, bool* window)
{
    //TODO: Need to clean up the memory leaks in this function and the next
    if (surface)
    {
        if (surface->format->BitsPerPixel < 32) {
            SDL_Surface* Temp_Surface = NULL;
            Temp_Surface = Unpalettize_Image(surface);

            SDL_to_OpenGl(Temp_Surface, texture);

            SDL_FreeSurface(Temp_Surface);
        }
        else {
            SDL_to_OpenGl(surface, texture);
        }

        *window = true;
    }
    if (texture == NULL) {
        printf("Unable to optimize image! SDL Error: %s\n", SDL_GetError());
        *window = false;
    }
}

void SDL_to_OpenGl(SDL_Surface *Surface, GLuint *texture)
{
    // OpenGL conversion from surface to texture
    {
        if (!glIsTexture(*texture)) {
            glGenTextures(1, texture);
        }
        glBindTexture(GL_TEXTURE_2D, *texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        printf("glError: %d\n", glGetError());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Surface->w, Surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Surface->pixels);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    }
}

//TODO: remove when everything else has been moved to new opengl stuff
void SDL_to_OpenGL_PAL(SDL_Surface *Surface, GLuint *texture)
{
    if (!glIsTexture(*texture)) {
        glGenTextures(1, texture);
    }
    glBindTexture(GL_TEXTURE_2D, *texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        Surface->w,
        Surface->h,
        0, GL_RED, GL_UNSIGNED_BYTE,
        Surface->pixels);
    //printf("glError: %d\n", glGetError());

}

//Palettize to 8-bit FO pallet, and dither
void Prep_Image(LF* F_Prop, SDL_PixelFormat* pxlFMT_FO_Pal, int color_match, bool* window, bool alpha_off) {

    if (F_Prop->img_data.type == FRM) {
        //copy the FRM_data pointer for editing
        //TODO: maybe copy by value instead?
        F_Prop->edit_data.FRM_data = F_Prop->img_data.FRM_data;

        F_Prop->edit_data.width  = F_Prop->img_data.width;
        F_Prop->edit_data.height = F_Prop->img_data.height;
        F_Prop->edit_data.scale  = F_Prop->img_data.scale;
        F_Prop->edit_data.offset = F_Prop->img_data.offset;

        //bind edit data for editing
        bind_NULL_texture(&F_Prop->edit_data, NULL, F_Prop->img_data.type);
        //set edit window bool to true, opens edit window
        *window = true;

    }
    else if (F_Prop->img_data.type == MSK) {
        //copy the MSK_data pointer for editing
        //TODO: maybe copy by value instead?
        F_Prop->edit_data.MSK_data = F_Prop->img_data.MSK_data;

        F_Prop->edit_data.width  = F_Prop->img_data.width;
        F_Prop->edit_data.height = F_Prop->img_data.height;
        F_Prop->edit_data.scale  = F_Prop->img_data.scale;
        F_Prop->edit_data.offset = F_Prop->img_data.offset;

        //bind edit data for editing
        bind_NULL_texture(&F_Prop->edit_data, NULL, F_Prop->img_data.type);

        //set edit window bool to true, opens edit window
        *window = true;

    }
    else {
        F_Prop->edit_data.FRM_data
            = FRM_Color_Convert(F_Prop->IMG_Surface,
                                pxlFMT_FO_Pal,
                                color_match);

        int width  = F_Prop->IMG_Surface->w;
        int height = F_Prop->IMG_Surface->h;
        int size = width * height;

        F_Prop->edit_data.scale = F_Prop->img_data.scale;
        F_Prop->edit_data.offset = F_Prop->img_data.offset;

        F_Prop->img_data.width  = width;
        F_Prop->img_data.height = height;

        F_Prop->edit_data.width  = width;
        F_Prop->edit_data.height = height;

        if (alpha_off) {
            //for (int i = 0; i < size; i++)
            //{
            //    if (F_Prop->img_data.FRM_data[i] == 0) {
            //        F_Prop->img_data.FRM_data[i] = 1;
            //    }
            //}
            for (int i = 0; i < size; i++)
            {
                if (F_Prop->edit_data.FRM_data[i] == 0) {
                    F_Prop->edit_data.FRM_data[i] = 1;
                }
            }
        }
        //bind edit data for editing
        bind_NULL_texture(&F_Prop->edit_data, NULL, FRM);
        //set edit window bool to true, opens edit window
        *window = true;
    }
}

// binds image information to PAL_texture (don't remember why right now, probably remove)
//bool bind_PAL_data(SDL_Surface* surface, struct image_data* img_data)
//{
//    //load & gen texture
//    glGenTextures(1, &img_data->PAL_texture);
//    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
//    //texture settings
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//    if (surface) {
//        if (surface->pixels) {
//            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
//            //control alignment of the image (auto aligned to 4-bytes) when converted to texture
//            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RED, GL_UNSIGNED_BYTE, surface->pixels);
//            //reset alignment to 1 for rest of program
//            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//        }
//        else {
//            printf("surface.pixels image didn't load...\n");
//            return false;
//        }
//    }
//    else {
//        if (img_data->FRM_texture) {
//            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
//            //control alignment of the image (auto aligned to 4-bytes) when converted to texture
//            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img_data->width, img_data->width, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->FRM_data);
//        }
//        else {
//            printf("FRM_texture didn't load...\n");
//            return false;
//        }
//    }
//
//    bool success = false;
//    success = init_framebuffer(img_data);
//    if (!success) {
//        printf("image framebuffer failed to attach correctly?\n");
//        return false;
//    }
//    return true;
//}

// binds the image information to FRM_texture
// and sets up PAL_texture with a NULL texture of appropriate size for editing
bool bind_NULL_texture(struct image_data* img_data, SDL_Surface* surface, img_type type)
{
    if (surface) {
        if (surface->pixels) {
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

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->FRM_data);
        }
        else {
            printf("surface.pixels image didn't load...\n");
            return false;
        }
    }
    else if (type == FRM) {
        if (img_data->FRM_data) {

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
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_data->width, img_data->height, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->FRM_data);
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

bool alpha_handler(bool* alpha)
{
    ImGui::Checkbox("Alpha Enabled", alpha);
    if (*alpha) {
        return false;
    }
    else {
        return true;
    }
}