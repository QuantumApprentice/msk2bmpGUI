#include "Image2Texture.h"
#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>


void Image2Texture(variables* My_Variables, int counter) {
	if (My_Variables->F_Prop[counter].file_open_window) {
		if (!My_Variables->F_Prop[counter].Optimized_Texture)
		{
			SDL_FreeSurface(My_Variables->Temp_Surface);
			My_Variables->Temp_Surface = SDL_ConvertSurfaceFormat(
				My_Variables->F_Prop[counter].image,
				SDL_PIXELFORMAT_RGBA8888, 0);

			SDL_to_OpenGl(My_Variables->Temp_Surface,
				&My_Variables->F_Prop[counter].Optimized_Texture);

		}
		if (My_Variables->F_Prop[counter].Optimized_Texture == NULL) {
			printf("Unable to optimize image %s! SDL Error: %s\n",
				My_Variables->F_Prop[counter].Opened_File, SDL_GetError());
			My_Variables->F_Prop[counter].file_open_window = false;
		}
		else
		{
			My_Variables->F_Prop[counter].texture_width = My_Variables->Temp_Surface->w;
			My_Variables->F_Prop[counter].texture_height = My_Variables->Temp_Surface->h;
		}
	}
}

void SDL_to_OpenGl(SDL_Surface *Temp_Surface, GLuint *Optimized_Texture)
{
	// OpenGL conversion from surface to texture - needs to be own function
	{
		int bpp;
		Uint32 Rmask, Gmask, Bmask, Amask;
		SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp,
			&Rmask, &Gmask, &Bmask, &Amask
		);
		/* Create surface that will hold pixels passed into OpenGL. */
		SDL_Surface *img_rgba8888 = SDL_CreateRGBSurface(0,
			Temp_Surface->w,
			Temp_Surface->h,
			bpp, Rmask, Gmask, Bmask, Amask
		);

		//SDL_SetSurfaceAlphaMod(My_Variables->Temp_Surface, 0xFF);
		//SDL_SetSurfaceBlendMode(My_Variables->Temp_Surface, SDL_BLENDMODE_NONE);

		SDL_BlitSurface(Temp_Surface, NULL, img_rgba8888, NULL);

		glGenTextures(1, Optimized_Texture);
		glBindTexture(GL_TEXTURE_2D, *Optimized_Texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			Temp_Surface->w,
			Temp_Surface->h,
			0, GL_RGBA, GL_UNSIGNED_BYTE,
			img_rgba8888->pixels);
	}
}