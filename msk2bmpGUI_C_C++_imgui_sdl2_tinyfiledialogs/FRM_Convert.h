#pragma once
#include <SDL.h>
#include "Load_Files.h"

uint8_t convert_colors(uint8_t bytes);

SDL_PixelFormat* loadPalette(char * name);
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface, SDL_PixelFormat* pxlFMT, bool SDL);
SDL_Surface* Load_FRM_Image(char *File_Name, SDL_PixelFormat* pxlFMT);
SDL_Surface* Unpalettize_Image(SDL_Surface* Render_Surface);
void SDL_Color_Match(SDL_Surface* Convert,
	SDL_PixelFormat* pxlFMT_Pal,
	SDL_Surface* Temp_Surface);
void Euclidian_Distance_Color_Match(
	SDL_Surface* Convert,
	SDL_Surface* Temp_Surface);
void clamp_dither(SDL_Surface *Surface_32,
	union Pxl_err *err,
	int pixel_idx,
	int factor);
void limit_dither(SDL_Surface *Surface_32,
	union Pxl_err *err,
	int **pxl_index_arr);