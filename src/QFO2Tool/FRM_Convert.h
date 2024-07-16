#pragma once
#include "Load_Files.h"

uint8_t convert_colors(uint8_t bytes);

Palette* load_palette_from_path(const char* path);
uint8_t* FRM_Color_Convert(Surface *surface, Palette* pxlFMT, int color_match_algo);
void Euclidian_Distance_Color_Match(Surface* Convert, Surface* Temp_Surface);
void clamp_dither(Surface *Surface_32, union Pxl_Err *err, int pixel_idx, int factor);
void limit_dither(Surface *Surface_32, union Pxl_Err *err, int x, int y);
