#include "FRM_Convert.h"
#include "B_Endian.h"
#include "tinyfiledialogs.h"
#include "Load_Files.h"
#include "MiniSDL.h"

#include <cstdint>
// #include <iostream>      //old c++ way of doing this
#include <vector>

#include <limits.h>

#ifdef QFO2_WINDOWS
#include <direct.h>

#elif defined(QFO2_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

union Pxl_err {
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

#define PALETTE_NUMBER 256
#define PALETTE_FLAT 228
Color PaletteColors[PALETTE_NUMBER];

//TODO: fix this to use float palette from My_Variables?
Palette* load_palette_to_Palette(const char * name)
{

#ifdef QFO2_WINDOWS
    FILE* file_ptr = fopen(name, "rb");
    if (file_ptr == NULL) {              //uncomment when compiling for windows
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
    int file_ptr = open(name, O_RDONLY);
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

    uint8_t r, g, b;
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
        PaletteColors[i] = Color{ r, g, b, 255 };
    }
    PaletteColors[0].a = 0;

    Palette* pxlFMT_FO_Pal = (Palette*)malloc(sizeof(Palette));
    memcpy(pxlFMT_FO_Pal, &PaletteColors, sizeof(PaletteColors));

    //TODO: need to free FO_Palette? (pxlFMT_FO_Pal might point to FO_Palette?)

    return pxlFMT_FO_Pal;
}


// Converts the color space to Fallout's paletted format
uint8_t* FRM_Color_Convert(Surface *surface, Palette* pxlFMT, int color_match_type)
{
    // Convert all surfaces to 32bit RGBA8888 format for easy conversion
    Surface* Surface_8;
    Surface* Surface_32 = ConvertSurfaceToRGBA(surface);

    // Setup for palettizing image
    Surface_8 = Create8BitSurface(surface->w, surface->h, pxlFMT);
    //Force Surface_8 to use the global palette instead of allowing SDL to use a copy

    // TODO cleanup: no need to have different color match algorithms as SDL used euclidian distance matching internally anyway
    // see https://github.com/libsdl-org/SDL/blob/e5101ebae68b62453930b94e19d62ae04e0df1f1/src/video/SDL_pixels.c#L1162
    //switch to change between euclidian and sdl color match algorithms
    // if (color_match_type == 0) {
    //     Color_Match(Surface_32, pxlFMT_Temp, Surface_8);
    // }
    // else if (color_match_type == 1)
    // {
        Euclidian_Distance_Color_Match(Surface_32, Surface_8);
    // }

    FreeSurface(Surface_32);

    int width  = Surface_8->w;
    int height = Surface_8->h;
    int size   = width * height;

    uint8_t* data = (uint8_t*)malloc(size + sizeof(FRM_Frame) + sizeof(FRM_Header));
    FRM_Frame* data_ptr = (FRM_Frame*)(data + sizeof(FRM_Header));

    int pixel_pointer = 0;
    for (int y = 0; y < height; y++)
    {
        //write out one row of pixels in each loop
        memcpy(data_ptr->frame_start + (width*y), ((uint8_t*)Surface_8->pixels + pixel_pointer), width);

        pixel_pointer += Surface_8->pitch;
    }
    FreeSurface(Surface_8);

    return data;
}

void Euclidian_Distance_Color_Match(
                Surface* Surface_32,
                Surface* Surface_8)
{
    uint8_t w_PaletteColor;
    Color rgba;
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
            int i = (Surface_32->pitch * y) + x * (sizeof(Color));
            w_smallest = INT_MAX;
            memcpy(&rgba, (uint8_t*)Surface_32->pixels + i, sizeof(Color));

            if (rgba.a < 255) {
                ((uint8_t*)Surface_8->pixels)[(Surface_8->pitch*y) + x] = 0;
            }
            else {
                for (int j = 1; j < PALETTE_FLAT; j++)
                {
                    s = rgba.r - PaletteColors[j].r;
                    t = rgba.g - PaletteColors[j].g;
                    u = rgba.b - PaletteColors[j].b;
                    v = rgba.a - PaletteColors[j].a;

                    s *= s;
                    t *= t;
                    u *= u;
                    v *= v;
                    // TODO no need to sqrt here since we're just doing a comparison and not using w for anything else
                    w = sqrt(s + t + u + v);

                    if (w < w_smallest) {
                        w_smallest = w;
                        w_PaletteColor = j;
                    }
                    // TODO if w == 0 here we've found an exact match and shouldn't bother searching the rest of the palette

                    if (j == PALETTE_FLAT - 1)
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

            ((uint8_t*)Surface_8->pixels)[(Surface_8->pitch*y) + x] = w_PaletteColor;
        }
    }
}

void limit_dither(Surface* Surface_32,
    union Pxl_err *err,
    int x, int y)
{
    int pixel_index[4];
    //if it crashes again on dithering, try removing the '=' sign from pixel_index[0] 
    //and pixel_index[2] and maybe pixel_index[1]?
    // this should probably use pitch instead of width for the multiplication
    pixel_index[0] = (y + 0 < Surface_32->h && x + 1 <  Surface_32->w) ? (Surface_32->w * (y + 0)) + (x + 1) : -1;
    pixel_index[1] = (y + 1 < Surface_32->h && x - 1 >= 0            ) ? (Surface_32->w * (y + 1)) + (x - 1) : -1;
    pixel_index[2] = (y + 1 < Surface_32->h && x + 0 <  Surface_32->w) ? (Surface_32->w * (y + 1)) + (x + 0) : -1;
    pixel_index[3] = (y + 1 < Surface_32->h && x + 1 <  Surface_32->w) ? (Surface_32->w * (y + 1)) + (x + 1) : -1;

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
                    union Pxl_err *err,
                    int pixel_idx,
                    int factor)
{
    // pointer arrays so I can run a loop through them dependably
    uint8_t* pxl_color [4];
    Color rgba;
    memcpy(&rgba, (Color*)Surface_32->pixels + pixel_idx, sizeof(Color));

    pxl_color[0] = &(((Color*)Surface_32->pixels + (pixel_idx))->r);
    pxl_color[1] = &(((Color*)Surface_32->pixels + (pixel_idx))->g);
    pxl_color[2] = &(((Color*)Surface_32->pixels + (pixel_idx))->b);
    pxl_color[3] = &(((Color*)Surface_32->pixels + (pixel_idx))->a);

    // take palettized error and clamp values to max/min, then add error to appropriate pixel
    for (int i = 0; i < 4; i++)
    {
        // clamp 255 or 0
        int total_error = (rgba.arr[i] + err->arr[i] * factor / 16);

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
Surface* Load_FRM_Image_SDL(char *File_Name, Palette* pxlFMT)
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

    //8 bit palleted stuff here
    Surface* Pal_Surface = Create8BitSurface(frame_width, frame_height, pxlFMT);

    // Seek to starting pixel in file
    fseek(File_ptr, 0x4A, SEEK_SET);

    // Read in the pixels from the FRM file to the surface
    // Have to read-in one line at a time to account for
    // SDL adding a buffer to the pitch
    uint8_t* data = (uint8_t*)Pal_Surface->pixels;

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

// Convert FRM color to standard 32bit
//opengl version -- being replaced by new code today
void Load_FRM_Image2(char *File_Name, unsigned int* texture, int* texture_width, int* texture_height)
{
    glDeleteTextures(1, texture);
    // load and generate texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // texture filtering settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //fixing alignment issues
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


    // File open stuff
    FILE *File_ptr = fopen(File_Name, "rb");
    if (File_ptr == NULL) {
        printf("File not opened?");
    }

    fseek(File_ptr, 0x3E, SEEK_SET);
    uint16_t frm_width = 0;
    fread(&frm_width, 2, 1, File_ptr);
    frm_width = B_Endian::write_u16(frm_width);
    uint16_t frm_height = 0;
    fread(&frm_height, 2, 1, File_ptr);
    frm_height = B_Endian::write_u16(frm_height);
    uint32_t frm_size = 0;
    fread(&frm_size, 4, 1, File_ptr);
    frm_size = B_Endian::write_u32(frm_size);

    *texture_width  = frm_width;
    *texture_height = frm_height;

    // Seek to starting pixel in file
    fseek(File_ptr, 0x4A, SEEK_SET);

    // Read in the pixels from the FRM file to the surface
    uint8_t* data = (uint8_t*)malloc(frm_size);
    fread(data, 1, frm_size, File_ptr);
    fclose(File_ptr);

    // Might have to read-in one line at a time to account for
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frm_width, frm_height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_2D);
        //TODO: control alignment of the image (auto aligned to 4-bytes) when converted to texture
        //GL_UNPACK_ALIGNMENT
        //TODO: control alignment of the image (auto aligned to 4-bytes) when read from texture
        //GL_PACK_ALIGNMENT
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
    }
    else {
        std::cout << "Failure to communicate..." << std::endl;
    }
}
