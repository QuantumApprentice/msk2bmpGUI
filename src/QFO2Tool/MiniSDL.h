#pragma once
#include <stdint.h>
#include "Palette_Cycle.h"

typedef struct Rect {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct Surface {
    uint16_t w;
    uint16_t h;
    int16_t  x;
    int16_t  y;
    int channels;       //number of color channels, 1 for indexed/palletized, 3/4 for RGB/RGBA
    int pitch;          //== w*channels
    Palette* palette;   //store a palette per image?
    uint8_t* pxls;      //actual pixel data
} Surface;

void FreeSurface(Surface* src);
Surface* Create_8Bit_Surface(int width, int height, Palette* palette);
Surface* Create_RGBA_Surface(int width, int height);
Surface* Load_File_to_RGBA(const char* filename);
Surface* Convert_Surface_to_RGBA(Surface* src);
Surface* Copy8BitSurface(Surface* src);

void BlitSurface(Surface* src, Rect src_rect, Surface* dst, Rect dst_rect);
void PaintSurface(Surface* dst, Rect dst_rect, uint8_t color);
void ClearSurface(Surface* dst);

void print_SURFACE_pxls(Surface* src);