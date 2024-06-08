#include "MiniSDL.h"

#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef STB_IMAGE_WRITE_IMPLEMENTATION

Surface* CreateRGBASurface(int width, int height)
{
  size_t size = sizeof(Surface) + width*height*4;
  void* ptr = malloc(size);
  Surface* surface = (Surface*)ptr;
  surface->w = width;
  surface->h = height;
  surface->channels = 4;
  surface->pitch = width*4;
  surface->pixels = ((uint8_t*)ptr) + sizeof(Surface);
  return surface;
}

Surface* Create8BitSurface(int width, int height, Palette* palette)
{
  size_t size = sizeof(Surface) + width*height;
  void* ptr = malloc(size);
  Surface* surface = (Surface*)ptr;
  surface->w = width;
  surface->h = height;
  surface->channels = 1;
  surface->pitch = width;
  surface->palette = palette;
  surface->pixels = ((uint8_t*)ptr) + sizeof(Surface);
  return surface;
}

void FreeSurface(Surface* surface)
{
  uint8_t* ptr = (uint8_t*)surface;
  ptr += sizeof(Surface);
  if (surface->pixels != ptr) {
    free(surface->pixels);
  }
  free(surface);
}

Surface* ConvertSurfaceToRGBA(Surface* src)
{
  Surface* surface = CreateRGBASurface(src->w, src->h);
  if (src->channels == 1) {
    Color* dst_pixel = (Color*)surface->pixels;
    uint8_t* src_pixel = src->pixels;
    int total_pixels = src->w * src->h;
    for (int i = 0; i < total_pixels; i++)
    {
      *dst_pixel = src->palette->colors[*src_pixel];
      src_pixel++;
      dst_pixel++;
    }
  } else if (src->channels == 3) {
    Color* dst_pixel = (Color*)surface->pixels;
    uint8_t* src_pixel = src->pixels;
    uint8_t r, g, b;
    int total_pixels = src->w * src->h;
    for (int i = 0; i < total_pixels; i++)
    {
      r = src_pixel[0];
      g = src_pixel[1];
      b = src_pixel[2];
      dst_pixel->r = r;
      dst_pixel->g = g;
      dst_pixel->b = b;
      dst_pixel->a = 255;
    }
  } else if (src->channels == 4) {
    memcpy(surface->pixels, src->pixels, src->w * src->pitch);
  }
  return surface;
}

Surface* LoadFileAsRGBASurface(const char* filename)
{
  int w, h, channels_in_file;
  uint8_t* pixels = (uint8_t*)stbi_load(filename, &w, &h, &channels_in_file, 4);
  if (!pixels) return NULL;
  Surface* surface = (Surface*)malloc(sizeof(Surface));
  surface->w = w;
  surface->h = h;
  surface->channels = 4;
  surface->pitch = w*4;
  surface->pixels = pixels;
  return surface;
}

bool SaveSurfaceAsBMP(Surface* surface, const char* filename)
{
  int result = stbi_write_bmp(filename, surface->w, surface->h, surface->channels, surface->pixels);
  return result != 0;
}

bool SaveSurfaceAsPNG(Surface* surface, const char* filename)
{
  int result = stbi_write_png(filename, surface->w, surface->h, surface->channels, surface->pixels, surface->pitch);
  return result != 0;
}

void BlitSurface(Surface* src, Rect src_rect, Surface* dst, Rect dst_rect)
{
  assert(src_rect.w == dst_rect.w);
  assert(src_rect.h == dst_rect.h);
  uint8_t* src_pixels = &src->pixels[src_rect.h * src->pitch + src_rect.x];
  uint8_t* dst_pixels = &dst->pixels[dst_rect.h * dst->pitch + dst_rect.x];
  for (int row = 0; row < src_rect.h; row++)
  {
    memcpy(dst, src, src_rect.w);
    src_pixels += src->pitch;
    dst_pixels += dst->pitch;
  }
}
