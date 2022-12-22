#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"

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

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            Surface->w,
            Surface->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            Surface->pixels);
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

        F_Prop->Pal_Surface
            = SDL_ConvertSurface(F_Prop->image,
                                 F_Prop->image->format,
                                 0);
        SDL_SetPixelFormatPalette(F_Prop->Pal_Surface->format,
                                  pxlFMT_FO_Pal->palette);
        //Unpalettize image to new surface for display
        F_Prop->Final_Render = Unpalettize_Image(F_Prop->Pal_Surface);
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

        printf(SDL_GetError());
    }
    else {
        F_Prop->Pal_Surface
            = FRM_Color_Convert(F_Prop->image,
                                pxlFMT_FO_Pal,
                                color_match);
        //Unpalettize image to new surface for display
        F_Prop->Final_Render = Unpalettize_Image(F_Prop->Pal_Surface);
    }




    //Converts unpalettized image to texture for display, sets window bool to true
    Image2Texture(F_Prop->Final_Render,
                 &F_Prop->Optimized_Render_Texture,
                  window);
}