#pragma once
#include <SDL.h>
#include "Load_Files.h"

uint8_t convert_colors(uint8_t bytes);

SDL_PixelFormat* load_palette_to_SDL_PixelFormat(const char * name);
uint8_t* FRM_Color_Convert(SDL_Surface *surface, SDL_PixelFormat* pxlFMT, int SDL);
SDL_Surface* Load_FRM_Image_SDL(char *File_Name, SDL_PixelFormat* pxlFMT);
SDL_Surface* Unpalettize_Image(SDL_Surface* Render_Surface);
void SDL_Color_Match(SDL_Surface* Convert, SDL_PixelFormat* pxlFMT_Pal, SDL_Surface* Temp_Surface);
void Euclidian_Distance_Color_Match( SDL_Surface* Convert, SDL_Surface* Temp_Surface);
void clamp_dither(SDL_Surface *Surface_32, union Pxl_err *err, int pixel_idx, int factor);
void limit_dither(SDL_Surface *Surface_32, union Pxl_err *err, int **pxl_index_arr);

void Load_FRM_Image2(char *File_Name, unsigned int* texture, int* texture_width, int* texture_height);
