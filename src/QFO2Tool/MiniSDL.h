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

typedef struct Palette {
    int num_colors = 228;   //vanilla fallout palette only uses first 228 colors, rest are hardcoded color cycling
    Color colors[256];
} Palette;

typedef struct Surface {
    int w;
    int h;
    int channels;       //number of color channels, 1 for indexed/palletized, 3/4 for RGB/RGBA
    int pitch;          //== w*channels
    Palette* palette;   //store a palette per image?
    uint8_t* pxls;      //actual pixel data
} Surface;


struct Edit_Surface{
    Surface** edit_frame;
};


void FreeSurface(Surface* src);
Surface* Create_8Bit_Surface(int width, int height, Palette* palette);
Surface* Create_RGBA_Surface(int width, int height);
Surface* Load_File_to_RGBA(const char* filename);
Surface* Convert_Surface_to_RGBA(Surface* src);
void BlitSurface(Surface* src, Rect src_rect, Surface* dst, Rect dst_rect);
void PaintSurface(Surface* dst, Rect dst_rect, uint8_t color);
