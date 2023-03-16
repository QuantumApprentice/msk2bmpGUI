#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"

bool bind_NULL_texture_test(struct image_data* img_data, SDL_Surface* surface);


void Image2Texture(SDL_Surface* surface, GLuint* texture, bool* window)
{
    //TODO: Need to work on a zoom feature
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
    // OpenGL conversion from surface to texture - needs to be own function
    {
        if (!glIsTexture(*texture)) {
            glGenTextures(1, texture);
        }
        glBindTexture(GL_TEXTURE_2D, *texture);

        //shader->use();

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            Surface->w,
            Surface->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            Surface->pixels);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        //printf("glError: %d\n", glGetError());

    }
}

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

//TODO: need to simplify My_Variables to accept F_Prop[counter] instead
    //Palettize to 8-bit FO pallet, and dithered
void Prep_Image(LF* F_Prop, SDL_PixelFormat* pxlFMT_FO_Pal, bool color_match, bool* window) {
    SDL_FreeSurface(F_Prop->Pal_Surface);
    SDL_FreeSurface(F_Prop->Final_Render);

    if (F_Prop->type == FRM) {
        //rebind the FRM for editing
        F_Prop->edit_data.FRM_data = F_Prop->img_data.FRM_data;

        F_Prop->edit_data.width  = F_Prop->img_data.width;
        F_Prop->edit_data.height = F_Prop->img_data.height;
        //bind edit data for editing
        bind_NULL_texture_test(&F_Prop->edit_data, NULL);
        //set edit window bool to true, opens edit window
        *window = true;

    }
    else if (F_Prop->type == MSK) {
        ////TODO: Handle MSK file types
        F_Prop->Pal_Surface
            = FRM_Color_Convert(F_Prop->image,
                                pxlFMT_FO_Pal,
                                color_match);
        printf(SDL_GetError());

        F_Prop->Final_Render = Unpalettize_Image(F_Prop->Pal_Surface);

        //F_Prop->Final_Render =
        //    SDL_CreateRGBSurface(0, F_Prop->Map_Mask->w, F_Prop->Map_Mask->h, 32, 0, 0, 0, 0);
        //SDL_BlitSurface(F_Prop->Map_Mask, NULL, F_Prop->Final_Render, NULL);
        //SDL_BlitSurface(F_Prop->image, NULL, F_Prop->Final_Render, NULL);

    //Converts unpalettized image to texture for display, sets window bool to true
        Image2Texture(F_Prop->Final_Render,
            &F_Prop->Optimized_Render_Texture,
            window);
        printf(SDL_GetError());
    }
    else {
        F_Prop->Pal_Surface
            = FRM_Color_Convert(F_Prop->image,
                                pxlFMT_FO_Pal,
                                color_match);
        F_Prop->edit_data.width  = F_Prop->Pal_Surface->w;
        F_Prop->edit_data.height = F_Prop->Pal_Surface->h;
        //bind edit data for editing
        bind_NULL_texture_test(&F_Prop->edit_data, F_Prop->Pal_Surface);
        //set edit window bool to true, opens edit window
        *window = true;
    }
}

bool bind_PAL_data(SDL_Surface* surface, struct image_data* img_data)
{
    //load & gen texture
    glGenTextures(1, &img_data->PAL_texture);
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (surface) {
        if (surface->pixels) {
            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
            //control alignment of the image (auto aligned to 4-bytes) when converted to texture
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RED, GL_UNSIGNED_BYTE, surface->pixels);
            //reset alignment to 1 for rest of program
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else {
            printf("surface.pixels image didn't load...\n");
            return false;
        }
    }
    else {
        if (img_data->FRM_texture) {
            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
            //control alignment of the image (auto aligned to 4-bytes) when converted to texture
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img_data->width, img_data->width, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->FRM_data);
        }
        else {
            printf("FRM_texture didn't load...\n");
            return false;
        }
    }

    bool success = false;
    success = init_framebuffer(img_data);
    if (!success) {
        printf("image framebuffer failed to attach correctly?\n");
        return false;
    }
    return true;
}

bool bind_NULL_texture_test(struct image_data* img_data, SDL_Surface* surface)
{
    if (surface) {
        if (surface->pixels) {
            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
            //control alignment of the image (auto aligned to 4-bytes) when converted to texture
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            //load & gen texture
            glGenTextures(1, &img_data->FRM_texture);
            glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
            //texture settings
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RED, GL_UNSIGNED_BYTE, surface->pixels);
            //reset alignment to 1 for rest of program
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else {
            printf("surface.pixels image didn't load...\n");
            return false;
        }
    }
    else {
        if (img_data->FRM_data) {

            glGenTextures(1, &img_data->FRM_texture);
            glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
            //texture settings
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
            //control alignment of the image (auto aligned to 4-bytes) when converted to texture
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            //bind FRM_data to FRM_texture for "indirect" editing
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_data->width, img_data->height, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->FRM_data);
        }
        else {
            printf("FRM_texture didn't load?...\n");
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