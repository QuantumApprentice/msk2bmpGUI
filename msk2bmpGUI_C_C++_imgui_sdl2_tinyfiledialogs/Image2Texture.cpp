#include "Image2Texture.h"
#include <SDL.h>
#include <stdio.h>
#include <glad/glad.h>

void Image2Texture(SDL_Surface* surface, GLuint* texture, bool* window)
{
	SDL_Surface* Temp_Surface = NULL;
	if (surface)
	{
		//SDL_FreeSurface(Temp_Surface);
		Temp_Surface = 
			SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);

		SDL_to_OpenGl(Temp_Surface, texture);
		*window = true;
	}
	if (texture == NULL) {
		printf("Unable to optimize image! SDL Error: %s\n", SDL_GetError());
		window = false;
	}
}

void SDL_to_OpenGl(SDL_Surface *Temp_Surface, GLuint *texture)
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
		
		printf("glError: %d\n", glGetError());

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);

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