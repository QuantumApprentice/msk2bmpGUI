#include <string.h>
#include <stdlib.h>
#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "MiniSDL.h"

//create blank 4-byte per pixel surface (RGBA?)
Surface* Create_RGBA_Surface(int width, int height)
{
    int size = sizeof(Surface) + width*height*4;
    Surface* surface = (Surface*)malloc(size);
    surface->w = width;
    surface->h = height;
    surface->channels = 4;
    surface->pitch    = 4*width;
    surface->pxls     = ((uint8_t*)surface) + sizeof(Surface);

    return surface;
}

Surface* Create_8Bit_Surface(int width, int height, Palette* palette)
{
    int size = sizeof(Surface) + width*height;
    Surface* surface = (Surface*)malloc(size);
    surface->w = width;
    surface->h = height;
    surface->channels = 1;
    surface->pitch    = width;
    surface->palette  = palette;
    surface->pxls     = ((uint8_t*)surface) + sizeof(Surface);

    return surface;
}

void FreeSurface(Surface* src)
{
    uint8_t* pxls_ptr = (uint8_t*)src + sizeof(Surface);
    if (src->pxls != pxls_ptr) {
        free(src->pxls);
    }
    free(src);
}

Surface* Convert_Surface_to_RGBA(Surface* src)
{
    Surface* RGBA_surface = Create_RGBA_Surface(src->w, src->h);

    Color* dst_pxl = (Color*)RGBA_surface->pxls;
    uint8_t* src_pxl = src->pxls;
    int total_pxls = src->w * src->h;
    if (src->channels == 1) {
        //convert from paletted to 32bit
        for (int i = 0; i < total_pxls; i++)
        {
            *dst_pxl = src->palette->colors[*src_pxl];
            src_pxl++;
            dst_pxl++;
        }
    }
    else if (src->channels == 3) {
        //convert from 24bit to 32bit
        uint8_t r,g,b;
        for (int i = 0; i < total_pxls; i++)
        {
            r = src_pxl[0];
            g = src_pxl[1];
            b = src_pxl[2];

            dst_pxl->r = r;
            dst_pxl->g = g;
            dst_pxl->b = b;
            dst_pxl->a = 255;
        }
    }
    else if (src->channels == 4) {
        //just copy using same format
        memcpy(RGBA_surface->pxls, src->pxls, src->h*src->pitch);
    }
    return RGBA_surface;
}

Surface* Load_File_to_RGBA(const char* filename)
{
    int w, h, channels;
    uint8_t* pxls = (uint8_t*)stbi_load(filename, &w, &h, &channels, 4);
    if (!pxls) {return nullptr;}

    Surface* surface = (Surface*)malloc(sizeof(Surface));
    surface->w        = w;
    surface->h        = h;
    surface->channels = 4; //why not =channels?
    surface->pitch    = w*4;
    surface->pxls     = pxls;

    return surface;
}

//src, src_rect, dst, dst_rect
//both surfaces must be same pixel width (RGB/RGBA etc)
//src_rect.w & h must be == dst_rect
void BlitSurface(Surface* src, Rect src_rect, Surface* dst, Rect dst_rect)
{
    assert(src_rect.w == dst_rect.w);
    assert(src_rect.h == dst_rect.h);
    //set starting position for top left corner of rectangle to copy
    uint8_t* src_pxls = &src->pxls[src_rect.y*src->pitch + src_rect.x*src->channels];
    uint8_t* dst_pxls = &dst->pxls[dst_rect.y*dst->pitch + dst_rect.x*dst->channels];

    //copy each row of src rectangle to dst surface
    for (int row = 0; row < src_rect.h; row++) {
        memcpy(dst_pxls, src_pxls, dst->pitch);
        src_pxls += src->pitch;
        dst_pxls += dst->pitch;
    }
}

//sets dst->pxls to 'color' palette index
//brush_rect determines section to color
void PaintSurface(Surface* dst, Rect brush_rect, uint8_t color)
{
    if (dst == nullptr) {
        return;
    }
    uint8_t* dst_pxls = &dst->pxls[brush_rect.y*dst->pitch + brush_rect.x*dst->channels];
    //copy each row of src rectangle to dst surface
    for (int row = 0; row < brush_rect.h; row++) {
        memset(dst_pxls, color, brush_rect.w);
        dst_pxls += dst->pitch;
    }
}

void ClearSurface(Surface* dst)
{
    if (dst == nullptr) {
        return;
    }
    uint8_t* pxls = dst->pxls;
    for (int i = 0; i < dst->h; i++)
    {
        memset(pxls, 0, dst->pitch);
        pxls += dst->pitch;
    }
}