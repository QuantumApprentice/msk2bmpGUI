#pragma once

#include <stdint.h>

typedef struct Rect {
  int x, y, w, h;
} Rect;

typedef union Color {
  struct {
    uint8_t r, g, b, a;
  };
  uint8_t arr[4];
  uint32_t abgr;
} Color;

typedef struct Palette {
  Color colors[256];
} Palette;

typedef struct Surface {
  int w;
  int h;
  int channels; // either 1 for indexed/palletized image or 3/4 for RGB/RGBA
  int pitch; // will be equal to w*channels, provided here mostly for compatibility with existing code
  Palette* palette;
  uint8_t* pixels;
} Surface;

Surface* CreateRGBASurface(int width, int height);
Surface* Create8BitSurface(int width, int height, Palette* palette);
void FreeSurface(Surface* surface);

Surface* ConvertSurfaceToRGBA(Surface* src);
Surface* LoadFileAsRGBASurface(const char* filename);
bool SaveSurfaceAsBMP(Surface* surface, const char* filename);

void BlitSurface(Surface* src, Rect src_rect, Surface* dst, Rect dst_rect);
