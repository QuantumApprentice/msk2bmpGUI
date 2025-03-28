#pragma once
#include <stdint.h>

typedef struct Rect {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef union Color {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint8_t clr[4];
    uint32_t rgba;
} Color;

//vanilla fallout palette only uses first 228 colors,
//the rest are hardcoded color cycling
//palettes can be swapped by providing a palette
//named the same as the FRM but with .PAL extension
//placed right next to each other
typedef struct Palette {
    int num_colors = 228;
    Color colors[256];
} Palette;

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
Surface* Convert_Surface_to_RGBA(Surface* src, float* FO_pal);
Surface* Copy8BitSurface(Surface* src);

void BlitSurface(Surface* src, Rect src_rect, Surface* dst, Rect dst_rect);
void PaintSurface(Surface* dst, Rect dst_rect, uint8_t color);
void ClearSurface(Surface* dst);

void print_SURFACE_pxls(Surface* src);