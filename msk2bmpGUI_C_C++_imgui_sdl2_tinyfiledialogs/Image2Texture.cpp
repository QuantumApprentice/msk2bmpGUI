#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

#include "Image2Texture.h"

void Image2Texture(SDL_Surface* surface, GLuint* texture, bool* window)
{
    //TODO: Need to work on a zoom feature
    //TODO: Need to clean up the memory leaks in this function and the next
    SDL_Surface* Temp_Surface = NULL;
    if (surface)
    {

        if (surface->format->BitsPerPixel < 32) {
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
        //int bpp = 0;
        //Uint32 Rmask, Gmask, Bmask, Amask;
        //SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp,
        //    &Rmask, &Gmask, &Bmask, &Amask );
        //
        ///* Create surface that will hold pixels passed into OpenGL. */
        //SDL_Surface *img_abgr8888 = SDL_CreateRGBSurface(0,
        //    Surface->w,
        //    Surface->h,
        //    bpp,
        //    Rmask, Gmask, Bmask, Amask);

        ////SDL_SetSurfaceAlphaMod(My_Variables->Temp_Surface, 0xFF);
        ////SDL_SetSurfaceBlendMode(Temp_Surface, SDL_BLENDMODE_NONE);
        //SDL_BlitSurface(Surface, NULL, img_abgr8888, NULL);

        //printf("glError: %d\n", glGetError());

        if (!glIsTexture(*texture)) {
            glGenTextures(1, texture);
        }
        glBindTexture(GL_TEXTURE_2D, *texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        //    img_abgr8888->w,
        //    img_abgr8888->h ,
        //    0, GL_RGBA, GL_UNSIGNED_BYTE,
        //    img_abgr8888->pixels);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            Surface->w,
            Surface->h,
            0, GL_RGBA, GL_UNSIGNED_BYTE,
            Surface->pixels);


        //SDL_FreeSurface(img_abgr8888);
    }
}

//TODO: need to simplify My_Variables to accept F_Prop[counter] instead
void Prep_Image(variables* My_Variables, int counter, bool color_match, bool* preview_type) {
    //Palettize to 8-bit FO pallet, and dithered
    SDL_FreeSurface(My_Variables->F_Prop[counter].Pal_Surface);

    if (My_Variables->F_Prop[counter].type == FRM) {

        My_Variables->F_Prop[counter].Pal_Surface
            = SDL_ConvertSurface(My_Variables->F_Prop[counter].image,
                                 My_Variables->F_Prop[counter].image->format,
                                 0);
        SDL_SetPixelFormatPalette(My_Variables->F_Prop[counter].Pal_Surface->format,
                                  My_Variables->pxlFMT_FO_Pal->palette);
            //SDL_BlitSurface
            //SDL_DuplicateSurface(My_Variables->F_Prop[counter].image);
    }
    else {
        My_Variables->F_Prop[counter].Pal_Surface
            = FRM_Color_Convert(My_Variables->F_Prop[counter].image,
                                //My_Variables->PaletteColors,
                                My_Variables->pxlFMT_FO_Pal,
                                color_match);
    }

    //Unpalettize image to new surface for display
    SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);

    My_Variables->F_Prop[counter].Final_Render
        = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);

    //Converts unpalettized image to texture for display, sets window bool to true
    Image2Texture(My_Variables->F_Prop[counter].Final_Render,
        &My_Variables->F_Prop[counter].Optimized_Render_Texture,
        preview_type);
}