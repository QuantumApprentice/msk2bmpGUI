#include "FRM_Convert.h"
#include "B_Endian.h"
#include "tinyfiledialogs.h"
#include "Load_Files.h"

#include <cstdint>
#include <vector>
#include <math.h>
#include <limits.h>

#ifdef QFO2_WINDOWS
#include <direct.h>

#elif defined(QFO2_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

union Pxl_Err {
    struct {
        int r;
        int g;
        int b;
        int a;
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

//TODO: fix this to use float palette from My_Variables?
Palette* load_palette_from_path(const char* path)
{
#ifdef QFO2_WINDOWS
    FILE* file_ptr = fopen(path, "rb");
    if (file_ptr == NULL) {
//TODO: replace color.pal w/name
//      (possibly modify messagebox for user provided palette)
        printf("Error opening color.pal \n%d: %s\n", errno, strerror(errno));
        printf("Current Working Directory: %s\n", _getcwd(NULL, 0));
        tinyfd_messageBox("Error:",
            "Missing color.pal, the default Fallout color palette.",
            "ok", "error", 1);
        return NULL;
    }
#elif defined(QFO2_LINUX)
    int file_ptr = open(path, O_RDONLY);
    if (file_ptr < 0) {
//TODO: replace color.pal w/name
//      (possibly modify messagebox for user provided palette)
        printf("Error opening color.pal \n%d: %s\n", errno, strerror(errno));
        printf("Current Working Directory: %s\n", getcwd(NULL, 0));
        tinyfd_messageBox("Error:",
            "Missing color.pal, the default Fallout color palette.",
            "ok", "error", 1);
        return NULL;
    }
#endif

    Palette* path_palette = (Palette*)malloc(sizeof(Palette));

    uint8_t r, g, b;
    Color* PaletteColors = path_palette->colors;
    for (int i = 0; i < 256; i++)
    {
        uint8_t bytes[4];

#ifdef QFO2_WINDOWS
        fread(bytes, 3, 1, file_ptr);
#elif defined(QFO2_LINUX)
        read(file_ptr, bytes, 3);
#endif

        r = convert_colors(bytes[0]);
        g = convert_colors(bytes[1]);
        b = convert_colors(bytes[2]);
        PaletteColors[i] = Color{ r, g, b };
    }

    path_palette->num_colors = 256;

    return path_palette;
}

// Converts the color space to Fallout's paletted format
uint8_t* FRM_Color_Convert(Surface *src, Palette* pxlFMT, int color_match_algo)
{
    // Convert input surface to 32bit format for easy palettization
    Surface* Surface_32 = Convert_Surface_to_RGBA(src);
    if (!Surface_32) {
        printf("Error: Unable to allocate 32-bit surface\n");
    }

    // Setup for palettizing image
    Surface* Surface_8 = Create_8Bit_Surface(src->w, src->h, pxlFMT);
    if (!Surface_8) {
        printf("Error: Unable to allocate 8-bit palette\n");
    }
    //switch to change between euclidian and sdl color match algorithms
    if (color_match_algo == 0) {
        //TODO: test difference between SDL_Color_Match() and Euclidian w/optimizations
        // SDL_Color_Match(Surface_32, pxlFMT_Temp, Surface_8);
        Euclidian_Distance_Color_Match(Surface_32, Surface_8);
    } else if (color_match_algo == 1) {
        //TODO: get a new color match algorithm
        Euclidian_Distance_Color_Match(Surface_32, Surface_8);
    }
    FreeSurface(Surface_32);

    int width  = Surface_8->w;
    int height = Surface_8->h;
    int size   = width * height;
    uint8_t* data = (uint8_t*)malloc(size + sizeof(FRM_Header) + sizeof(FRM_Frame));
    FRM_Frame* data_ptr = (FRM_Frame*)(data + sizeof(FRM_Header));
    int pxl_ptr = 0;
    for (int y = 0; y < height; y++)
    {
        //write out one row of pixels in each loop
        memcpy(data_ptr->frame_start + (width*y), ((uint8_t*)Surface_8->pxls + pxl_ptr), width);
        pxl_ptr += Surface_8->pitch;
    }
    //TODO: should I return the surface as well?
    //      surface contains palette information
    //      palette is lost if not returned here
    FreeSurface(Surface_8);
    return data;
}

void Euclidian_Distance_Color_Match(
                Surface* Surface_32,
                Surface* Surface_8)
{
    uint8_t w_PaletteColor;
    Color rgba;
    Pxl_Err err;

    int w_smallest;

    int s;
    int t;
    int u;
    int v;
    int w;

    int c = 100;    //TODO: remove this counter
    int pixel_idx = 0;
    Color* PaletteColors = Surface_8->palette->colors;

    for (int y = 0; y < Surface_32->h; y++) {
        for (int x = 0; x < Surface_32->w; x++) {

            w_smallest = INT_MAX;
            int i = (Surface_32->pitch * y) + x * (sizeof(Color));
            memcpy(&rgba, (uint8_t*)Surface_32->pxls + i, sizeof(Color));

            if (rgba.a < 255) {
                ((uint8_t*)Surface_8->pxls)[(Surface_8->pitch*y) + x] = 0;
            } else {
                for (int j = 0; j < Surface_8->palette->num_colors; j++)
                {
                    s = rgba.r - PaletteColors[j].r;
                    t = rgba.g - PaletteColors[j].g;
                    u = rgba.b - PaletteColors[j].b;
                    v = rgba.a - PaletteColors[j].a;

                    s *= s;
                    t *= t;
                    u *= u;
                    v *= v;
                    //TODO: get rid of this sqrt() to make it faster
                    w = sqrt(s + t + u + v);

                    if (w < w_smallest) {
                        w_smallest = w;
                        w_PaletteColor = j;
                    }
                // TODO: if w == 0 here
                // we've found an exact match
                // and shouldn't bother searching
                // the rest of the palette
                    if (j == Surface_8->palette->num_colors - 1)
                    {
                        err.r = rgba.r - PaletteColors[w_PaletteColor].r;
                        err.g = rgba.g - PaletteColors[w_PaletteColor].g;
                        err.b = rgba.b - PaletteColors[w_PaletteColor].b;
                        err.a = rgba.a - PaletteColors[w_PaletteColor].a;

                        limit_dither(Surface_32, &err, x, y);
                    }
                }
            }
            //TODO: need to clean this up
            //      used to keep track of palettization
            if (i == c) {
                printf("Euclidian color match loop #: %d\n", i);
                c *= 10;
            }

            ((uint8_t*)Surface_8->pxls)[(Surface_8->pitch*y) + x] = w_PaletteColor;
        }
    }
}

void limit_dither(Surface* Surface_32,
                  union Pxl_Err *err,
                  int x, int y)
{
    int pixel_index[4];
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

void clamp_dither(Surface *Surface_32,
                    union Pxl_Err *err,
                    int pixel_idx,
                    int factor)
{
    // pointer arrays so I can run a loop through them dependably
    uint8_t* pxl_color [4];
    Color rgba;
    memcpy(&rgba, (Color*)Surface_32->pxls + pixel_idx, sizeof(Color));

    pxl_color[0] = &(((Color*)Surface_32->pxls + (pixel_idx))->r);
    pxl_color[1] = &(((Color*)Surface_32->pxls + (pixel_idx))->g);
    pxl_color[2] = &(((Color*)Surface_32->pxls + (pixel_idx))->b);
    pxl_color[3] = &(((Color*)Surface_32->pxls + (pixel_idx))->a);

    // take palettized error and clamp values to max/min, then add error to appropriate pixel
    for (int i = 0; i < 4; i++)
    {
        // clamp 255 or 0
        int total_error = (rgba.clr[i] + err->arr[i] * factor / 16);

        if (total_error > 255)
        { *pxl_color[i] = 255; }
        else
        if (total_error < 0)
        { *pxl_color[i] = 0; }
        // if not clamp, then store error info
        else
          *pxl_color[i] = total_error;
    }
}