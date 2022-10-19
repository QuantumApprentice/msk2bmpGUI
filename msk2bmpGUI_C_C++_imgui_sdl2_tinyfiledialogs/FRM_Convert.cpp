#include "FRM_Convert.h"
#include "FRM_Animate.h"
#include "B_Endian.h"
#include "tinyfiledialogs.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <SDL.h>


union Pxl_info_32 {
    struct {
        uint8_t a;
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
    uint8_t arr[4];
};

union Pxl_err {
    struct {
        int a;
        int b;
        int g;
        int r;
    };
    int arr[4];
};

// Used to convert Fallout's palette colors to normal values
uint8_t convert_colors(uint8_t bytes) {
    if (bytes < 64) {
        return 4 * bytes;
    }
    else {
        return bytes;
    }
}

#define PALETTE_NUMBER 256
#define PALETTE_FLAT 228
SDL_Color PaletteColors[PALETTE_NUMBER];

SDL_PixelFormat* loadPalette(char * name)
{
    std::ifstream f("color.pal", 
        std::ios::in | std::ios::binary);
    if (!f.is_open()) {
        printf("Error opening color.pal\n");
        tinyfd_messageBox("Error:", 
                          "Missing color.pal, the default Fallout color palette.",
                          "ok", "error", 1);
        return NULL;
    }

    uint8_t r, g, b;
    //printf("Palette size: %d\n", 256);
    
    for (int i = 0; i < 256; i++)
    {
        uint8_t bytes[4];

        f.read((char*)bytes, 3);
        r = convert_colors(bytes[0]);
        g = convert_colors(bytes[1]);
        b = convert_colors(bytes[2]);
        PaletteColors[i] = SDL_Color{ r, g, b };
    }

    SDL_Palette* FO_Palette;
    SDL_PixelFormat* pxlFMT_FO_Pal;
    FO_Palette = SDL_AllocPalette(PALETTE_NUMBER);
    SDL_SetPaletteColors(FO_Palette, PaletteColors, 0, PALETTE_NUMBER);
    pxlFMT_FO_Pal = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
    SDL_SetPixelFormatPalette(pxlFMT_FO_Pal, FO_Palette);


    return pxlFMT_FO_Pal;
}


// Converts the color space to Fallout's paletted format
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface, SDL_PixelFormat* pxlFMT, bool SDL)
{
    // Convert all surfaces to 32bit RGBA8888 format for easy conversion
    SDL_Surface* Surface_8;
    SDL_PixelFormat* pxlFMT_UnPal;
    //TODO: swap SDL_PIXELFORMAT_RGBA8888 to SDL_PIXELFORMAT_ABGR8888
    //      for more consistent color handling,
    //      need to modify all the color matching stuff to match this change
    pxlFMT_UnPal = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);

    SDL_Surface* Surface_32 = SDL_ConvertSurface(surface, pxlFMT_UnPal, 0);
    if (!Surface_32) {
        printf("Error: %s\n", SDL_GetError());
    }

    // Setup for palettizing image
    //SDL_Palette* FO_Palette;
    //SDL_PixelFormat* pxlFMT_Pal;
    //FO_Palette = SDL_AllocPalette(PALETTE_NUMBER);
    //SDL_SetPaletteColors(FO_Palette, palette, 0, PALETTE_NUMBER);
    //pxlFMT_Pal = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
    //SDL_SetPixelFormatPalette(pxlFMT_Pal, FO_Palette);

    Surface_8 = SDL_ConvertSurface(surface, pxlFMT, 0);

    //TODO: Here's where the paint problem is
    //create new palette just for color matching (w/o the cycling colors)
    SDL_Palette* Temp_Palette;
    SDL_PixelFormat* pxlFMT_Temp;
    Temp_Palette = SDL_AllocPalette(PALETTE_FLAT);
    SDL_SetPaletteColors(Temp_Palette, pxlFMT->palette->colors, 0, PALETTE_FLAT);
    pxlFMT_Temp = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
    SDL_SetPixelFormatPalette(pxlFMT_Temp, Temp_Palette);

    if (!Surface_8) {
        printf("Error: %s\n", SDL_GetError());
    }

    //switch to change between euclidian and sdl color match algorithms
    if (SDL == true) {
        SDL_Color_Match(Surface_32, pxlFMT_Temp, Surface_8);
    }
    else
    {
        Euclidian_Distance_Color_Match(Surface_32, Surface_8);
    }

    SDL_FreeFormat(pxlFMT_Temp);
    SDL_FreeFormat(pxlFMT_UnPal);
    SDL_FreeSurface(Surface_32);
    return Surface_8;
}

void SDL_Color_Match(SDL_Surface* Surface_32,
                    SDL_PixelFormat* pxlFMT_Pal,
                    SDL_Surface* Surface_8)
{
    uint8_t w_PaletteColor;
    Pxl_info_32 abgr;
    Pxl_err err;
    int c = 100;
    // Convert image color to indexed palette
    for (int y = 0; y < Surface_32->h; y++)
    {
        for (int x = 0; x < Surface_32->w; x++)
        {
            int i = (Surface_32->pitch * y) + x * (sizeof(Pxl_info_32));

            memcpy(&abgr, (uint8_t*)Surface_32->pixels + i, sizeof(Pxl_info_32));
            w_PaletteColor = SDL_MapRGBA(pxlFMT_Pal, abgr.r, abgr.g, abgr.b, abgr.a);

            err.a = abgr.a - PaletteColors[w_PaletteColor].a;
            err.b = abgr.b - PaletteColors[w_PaletteColor].b;
            err.g = abgr.g - PaletteColors[w_PaletteColor].g;
            err.r = abgr.r - PaletteColors[w_PaletteColor].r;

            int* pxl_index_arr[2];
            pxl_index_arr[0] = &x;
            pxl_index_arr[1] = &y;

            limit_dither(Surface_32, &err, pxl_index_arr);

            //TODO: need to clean this up
            if (i == c) { printf("SDL color match loop #: %d\n", i);
                          c *= 10; }

            ((uint8_t*)Surface_8->pixels)[(Surface_8->pitch*y)+x] = w_PaletteColor;
        }
    }
}

void Euclidian_Distance_Color_Match(
                SDL_Surface* Surface_32,
                SDL_Surface* Surface_8)
{
    uint8_t w_PaletteColor;
    Pxl_info_32 abgr;
    Pxl_err err;

    int w_smallest;

    int s;
    int t;
    int u;
    int v;
    int w;

    int pixel_idx = 0;
    int c = 100;

    for (int y = 0; y < Surface_32->h; y++)
    {
        for (int x = 0; x < Surface_32->w; x++)
        {
            //int i = (Surface_32->w * y) + x;
            //w_smallest = INT_MAX;
            //memcpy(&abgr, (Pxl_info_32*)Surface_32->pixels + i, sizeof(Pxl_info_32));
            int i = (Surface_32->pitch * y) + x * (sizeof(Pxl_info_32));
            w_smallest = INT_MAX;
            memcpy(&abgr, (uint8_t*)Surface_32->pixels + i, sizeof(Pxl_info_32));

            for (int j = 0; j < PALETTE_FLAT; j++)
            {
                s = abgr.r - PaletteColors[j].r;
                t = abgr.g - PaletteColors[j].g;
                u = abgr.b - PaletteColors[j].b;
                v = abgr.a - PaletteColors[j].a;

                s *= s;
                t *= t;
                u *= u;
                v *= v;
                w = sqrt(s + t + u + v);

                if (w < w_smallest) {
                    w_smallest = w;
                    w_PaletteColor = j;
                }

                if (j == PALETTE_FLAT - 1)
                {
                    err.r = abgr.r - PaletteColors[w_PaletteColor].r;
                    err.g = abgr.g - PaletteColors[w_PaletteColor].g;
                    err.b = abgr.b - PaletteColors[w_PaletteColor].b;
                    err.a = abgr.a - PaletteColors[w_PaletteColor].a;

                    int* pxl_index_arr[2];
                    pxl_index_arr[0] = &x;
                    pxl_index_arr[1] = &y;

                    limit_dither(Surface_32, &err, pxl_index_arr);
                }
            }
            //TODO: need to clean this up
            if (i == c) {
                printf("SDL color match loop #: %d\n", i);
                c *= 10;
            }

            ((uint8_t*)Surface_8->pixels)[(Surface_8->pitch*y) + x] = w_PaletteColor;
        }
    }
}

void limit_dither(SDL_Surface* Surface_32,
    union Pxl_err *err,
    int **pxl_index_arr)
{
    int x = *pxl_index_arr[0];
    int y = *pxl_index_arr[1];

    //if ((x - 1 <= 0) || (x + 1 >= Surface_32->w)) return;
    //if (y + 1 >= Surface_32->h) return;

    int pixel_index[4];
    //if it crashes again on dithering, try removing the '=' sign from pixel_index[0] 
    //and pixel_index[2] and maybe pixel_index[1]?
    pixel_index[0] = (y + 0 <  Surface_32->h && x + 1 <  Surface_32->w) ? (Surface_32->w * (y + 0)) + (x + 1) : -1;
    pixel_index[1] = (y + 1 <  Surface_32->h && x - 1 >= 0            ) ? (Surface_32->w * (y + 1)) + (x - 1) : -1;
    pixel_index[2] = (y + 1 <  Surface_32->h && x + 0 <  Surface_32->w) ? (Surface_32->w * (y + 1)) + (x + 0) : -1;
    pixel_index[3] = (y + 1 <  Surface_32->h && x + 1 <  Surface_32->w) ? (Surface_32->w * (y + 1)) + (x + 1) : -1;

    int factor[4];
    factor[0] = 7;
    factor[1] = 3;
    factor[2] = 5;
    factor[3] = 1;

    for (int i = 0; i < 4; i++)
    {
        if (pixel_index[i] >= 0) {
            clamp_dither(Surface_32, err, pixel_index[i], factor[i]);
        }
    }
}

void clamp_dither(SDL_Surface *Surface_32,
                    union Pxl_err *err,
                    int pixel_idx,
                    int factor)
{
    // pointer arrays so I can run a loop through them dependably
    uint8_t* pxl_color [4];
    union Pxl_info_32 abgr;
    memcpy(&abgr, (Pxl_info_32*)Surface_32->pixels + pixel_idx, sizeof(Pxl_info_32));

    pxl_color[0] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->a);
    pxl_color[1] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->b);
    pxl_color[2] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->g);
    pxl_color[3] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->r);

    // take palettized error and clamp values to max/min, then add error to appropriate pixel
    for (int i = 0; i < 4; i++)
    {
        // clamp 255 or 0
        int total_error = (abgr.arr[i] + err->arr[i] * factor / 16);

        if (total_error > 255)
        {	*pxl_color[i] = 255;	}
        else
        if (total_error < 0)
        {   *pxl_color[i] = 0;	}
        // if not clamp, then store error info
        else
            *pxl_color[i] = total_error;
    }
}

// Convert FRM color to standard 32bit
SDL_Surface* Load_FRM_Image(char *File_Name, SDL_PixelFormat* pxlFMT)
{
    // File open stuff
    FILE *File_ptr = fopen(File_Name, "rb");
    fseek(File_ptr, 0x3E, SEEK_SET);

        uint16_t frame_width = 0;
        fread(&frame_width, 2, 1, File_ptr);
    frame_width = B_Endian::write_u16(frame_width);
        uint16_t frame_height = 0;
        fread(&frame_height, 2, 1, File_ptr);
    frame_height = B_Endian::write_u16(frame_height);
        uint32_t frame_size = 0;
        fread(&frame_size, 4, 1, File_ptr);
    frame_size = B_Endian::write_u32(frame_size);

    // 8 bit palleted stuff here
    //SDL_PixelFormat* pxlFMT_Pal;
    //SDL_Palette* FO_Palette;
    //FO_Palette = SDL_AllocPalette(PALETTE_NUMBER);
    //SDL_SetPaletteColors(FO_Palette, palette, 0, PALETTE_NUMBER);
    //pxlFMT_Pal = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
    //SDL_SetPixelFormatPalette(pxlFMT_Pal, FO_Palette);

    //TODO: Can I make a new surface with a palette without using SDL_CreateRGBSurface AND SDL_ConvertSurface()?
    SDL_Surface* Surface_8   = SDL_CreateRGBSurface(0, frame_width, frame_height, 8, 0,0,0,0);
    SDL_Surface* Pal_Surface = SDL_ConvertSurface(Surface_8, pxlFMT, 0);
    SDL_FreeSurface(Surface_8);

    if (!Pal_Surface) {
        printf("Error: %s\n", SDL_GetError());
    }

    // Seek to starting pixel in file
    fseek(File_ptr, 0x4A, SEEK_SET);

    // Read in the pixels from the FRM file to the surface
    // Have to read-in one line at a time to account for
    // SDL adding a buffer to the pitch
    uint8_t* data = (uint8_t*)Pal_Surface->pixels;

    //Pal_Surface->pitch = Pal_Surface->w;
    //fread(data, frame_height, frame_width, File_ptr);

    int pitch = Pal_Surface->pitch;
    for (int row = 0; row < frame_height; row++)
    {
        //int total;
        //total =
            fread(data, 1, frame_width, File_ptr);
            data += pitch;
        //printf("%d\n", total);
    }

    return Pal_Surface;
}

SDL_Surface* Unpalettize_Image(SDL_Surface* Surface)
{
    SDL_PixelFormat* pxlFMT_UnPal;
    pxlFMT_UnPal = SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);

    SDL_Surface* Output_Surface;
    Output_Surface = SDL_ConvertSurface(Surface, pxlFMT_UnPal, 0);
    //SDL_FreeFormat(pxlFMT_UnPal);

    if (!Output_Surface) {
        printf("\nError: %s", SDL_GetError());
    }
    else {
        return Output_Surface;
    }
}

//void OpenGL_Palette_to_Texture()
//{
//    char buffer[512];
//    FILE *palette_test;
//    palette_test = fopen("Palette_Shader.vert", "rb");
//
//    fread(buffer, sizeof(palette_test), 1, palette_test);
//}