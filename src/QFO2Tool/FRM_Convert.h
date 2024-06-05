#pragma once
#include "Load_Files.h"

uint8_t convert_colors(uint8_t bytes);

Palette* load_palette_to_Palette(const char * name);
uint8_t* FRM_Color_Convert(Surface *surface, Palette* pxlFMT, int SDL);
Surface* Load_FRM_Image_SDL(char *File_Name, Palette* pxlFMT);
// void SDL_Color_Match(SDL_Surface* Convert, Palette* pxlFMT_Pal, SDL_Surface* Temp_Surface);
void Euclidian_Distance_Color_Match( Surface* Convert, Surface* Temp_Surface);
void clamp_dither(Surface *Surface_32, union Pxl_err *err, int pixel_idx, int factor);
void limit_dither(Surface *Surface_32, union Pxl_err *err, int **pxl_index_arr);

void Load_FRM_Image2(char *File_Name, unsigned int* texture, int* texture_width, int* texture_height);
