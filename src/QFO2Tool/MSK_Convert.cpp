#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "MSK_Convert.h"
#include "Image2Texture.h"


#include "tinyfiledialogs.h"


#include "ImGui_Warning.h"


// Windows BITMAPINFOHEADER format, for historical reasons
uint8_t bmpHeader[62] = {
    // Offset 0x00000000 to 0x00000061
    0x42, 0x4D,             // 'BM'
    0xCE, 0x33, 0x00, 0x00, //Size in bytes
    0x00, 0x00, 0x00, 0x00,
    0x3E, 0x00, 0x00, 0x00, // Starting Address of pixels
    // Windows BITMAPINFOHEADER as per original
    0x28, 0x00, 0x00, 0x00, // Size of header
    0x5E, 0x01, 0x00, 0x00, // Bitmap Width  (350px)
    0x2C, 0x01, 0x00, 0x00, // Bitmap Height (300px)
    0x01, 0x00,             // Number of color planes
    0x01, 0x00,             // Bits per pixel
    0x00, 0x00, 0x00, 0x00, // Compression/ (1 = none)
    0x90, 0x33, 0x00, 0x00, // Image size (raw bitmap data [300x44])
    0xC4, 0x0E, 0x00, 0x00, // Horizotal Resolution
    0xC4, 0x0E, 0x00, 0x00, // Vertical Resolution
    0x00, 0x00, 0x00, 0x00, // Number of colors (0 = 2^n)
    0x00, 0x00, 0x00, 0x00, // Number of important colors used
    // Color Table, RGBA
    0x00, 0x00, 0x00, 0x00, // 0
    0xFF, 0xFF, 0xFF, 0x00  // 1
};

bool load_MSK_SURFACE(char* FileName, image_data* img_data, int width, int height);


// Lines of bits. Essentially the raw MSK file.
int i = 0;
FILE *infile;
line_array_t inputLines;

void Read_MSK_Tile(FILE *file, uint8_t vOutput[MAX_LINES][44])
{
    // Each line in an MSK file is 44 bytes. 
    // Obviously, this isn't flexible if a 
    // different use case is needed. 
    // Although it is important to note that 
    // the MSK file doesn't have any header 
    // information and so any line-by-line 
    // processing will have to figure out 
    // (or be told) the length of the line.

    //TODO: may need alternate function
    //      44*8 = 352, not sure if it works
    //      with the SDL surface the same way
    fseek(file, 0, SEEK_SET);
    fread(vOutput, MAX_LINES * 44, 1, file);
}

//TODO: need to handle switching between dropped
//      bmp files and SDL surface files
bool IsBMPFile(FILE *infile)
{
    // Super lazy - is the first character a "B"?
    // False positives for MSK files that have this as the first set of bits
    int firstChar;
    firstChar = fgetc(infile);
    ungetc(firstChar, infile);
    printf("File is a %c\n", firstChar);
    return ('B' == firstChar); // True only if "B"
}

bool ReadBmpLines(FILE *file, line_array_t vOutput)
{
    // Note that this is a simple parsing. For more complicated work, maybe 
    // Hook into a proper image parsing library.
    char bmpHeader[18];
    fread(bmpHeader, sizeof(char), 18, file);
    if ('B' != bmpHeader[0] || 'M' != bmpHeader[1])
    {
        printf("[ERROR] Input file Lacks Bitmap Signature");
        return false;
    }
    int ImageDataOffset = BytesToInt(bmpHeader + 0x0A, 4);

    int HeaderSize = BytesToInt(bmpHeader + 0x0E, 4);
    char bmpSubHeader[128];
    fread(bmpSubHeader, sizeof(char), ((HeaderSize > 132) ? 128 : (HeaderSize - 4)), file);
    int BitmapWidth, BitmapHeight, BitsPerPixel;
    int Compression = 0;
    if (HeaderSize == 12)
    {
        BitmapWidth = BytesToInt(bmpSubHeader, 2);
        BitmapHeight = BytesToInt(bmpSubHeader + 2, 2);
        BitsPerPixel = BytesToInt(bmpSubHeader + 6, 2);
    }
    else
    {
        BitmapWidth = BytesToInt(bmpSubHeader, 4);
        BitmapHeight = BytesToInt(bmpSubHeader + 4, 4);
        BitsPerPixel = BytesToInt(bmpSubHeader + 10, 2);
        Compression = BytesToInt(bmpSubHeader + 12, 4);
    }
    // Sanity Checks - All of these could be dealt with either
    // With better logic or with an image library.
    if (300 != BitmapHeight || 350 != BitmapWidth)
    {
        printf("[ERROR] Expecting 350x300 Bitmap (%dx%d seen)", BitmapWidth, BitmapHeight);
        return false;
    }
    if (0 != Compression && 3 != Compression && 0x0B != Compression)
    {
        printf("[ERROR] Only uncompressed Bitmaps supported");
        return false;
    }
    if (1 != BitsPerPixel)
    {
        printf("[ERROR] Only 2-color bitmaps supported");
        return false;
    }
    fseek(file, ImageDataOffset, SEEK_SET);
    fread(vOutput, MAX_LINES * 44, 1, file);

    return true;
}

// Ensure Little Endian Interpretation - I think there is a built-in for this,
// But too lazy to look it up.
int BytesToInt(char *C, int numBytes)
{
    // if (!numBytes) numBytes = 4;
    int ReturnVal = 0;
    for (int rOff = 1; rOff <= numBytes; ++rOff)
    {
        ReturnVal *= 256;
        unsigned char subC = C[numBytes - rOff];
        ReturnVal += subC;
    }
    return ReturnVal;
}

//Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define TILE_W      (350)
#define TILE_H      (300)
#define TILE_SIZE   (350*300)

bool Load_MSK_Tile_SURFACE(char* FileName, image_data* img_data)
{
    // if (img_data->FRM_data == NULL) {
        // loading to an empty? slot
        if (load_MSK_SURFACE(FileName, img_data, TILE_W, TILE_H)) {
            return true;
        }
    // }
    // else {
    //     //TODO: change this to load a new file type,
    //     //      different extension,
    //     //      include width/height in the header
    //     if (load_MSK_SURFACE(FileName, img_data, img_data->width, img_data->height)) {
    //         return true;
    //     }
    // }
    return false;
}

//load MSK FileName to uint8_t binary buffer,
//then parse this buffer into another uint8_t 8-bit Surface
//before assigning the binary buffer to img_data->MSK_data
//and assigning the 8-bit Surface to img_data->MSK_srfc
bool load_MSK_SURFACE(char* FileName, image_data* img_data, int width, int height)
{
    //TODO: refactor this and make sure the inputLines/file_buffer
    //      matches the other buffer for exporting

    FILE* File_ptr;

    int buffsize = (width + 7) / 8 * height;
    uint8_t* MSK_buffer = (uint8_t*)malloc(buffsize);
    if (!MSK_buffer) {
        set_popup_warning(
            "[ERROR] load_MSK_SURFACE\n\n"
            "Failed to allocate memory for MSK_buffer.\n"
        );
        printf("[ERROR] Failed to allocate memory for MSK_buffer.\n");
        printf("MSK image didn't load...\n");
        return false;
    }

    //open the file & error checking
#ifdef QFO2_WINDOWS
    fopen_s(&File_ptr, FileName, "rb");
#elif defined(QFO2_LINUX)
    File_ptr = fopen(FileName, "rb");
#endif

    if (File_ptr == NULL)
    {
        set_popup_warning(
            "[ERROR] load_MSK_SURFACE\n\n"
            "Unable to open file.\n"
        );
        //TODO: fprintf? or printf? (stderr or stdout)?
        fprintf(stderr, "[ERROR] Unable to open file: %s.\n", FileName);
        return false;
    }

    //read the binary lines in
    fread(MSK_buffer, buffsize, 1, File_ptr);
    fclose(File_ptr);

    uint8_t white     = 1;
    uint8_t bitmask   = 128;
    uint8_t* bin_ptr  = MSK_buffer;
    Surface* MSK_srfc = Create_8Bit_Surface(width, height, nullptr);

    if (!MSK_srfc) {
        set_popup_warning(
            "[ERROR] load_MSK_SURFACE\n\n"
            "Failed to allocate memory for MSK surface.\n"
        );
        printf("[ERROR] Failed to allocate memory for MSK surface.\n");
        printf("MSK image didn't load...\n");
        return false;
    }

    //parse binary MSK data and turn into 8-bit surface->pxls
    for (int pxl_y = 0; pxl_y < height; pxl_y++) {
        for (int pxl_x = 0; pxl_x < width; pxl_x++) {

            uint8_t buff = *bin_ptr;
            bool mask_1_or_0 = (buff & bitmask);
            if (mask_1_or_0) {
                *(MSK_srfc->pxls + (pxl_y * width) + pxl_x) = white;
            }
            bitmask >>= 1;

            if (bitmask == 0) {
                ++bin_ptr;
                bitmask = 128;
            }
        }
        if (bitmask < 128) {
            ++bin_ptr;
            bitmask = 128;
        }
    }

    img_data->MSK_texture = init_texture(
                                MSK_srfc,
                                MSK_srfc->w,
                                MSK_srfc->h,
                                MSK
                            );

    if (img_data->MSK_srfc) {
        free(img_data->MSK_srfc);
    }
    if (img_data->MSK_data) {
        free(img_data->MSK_data);
    }


    img_data->MSK_data = MSK_buffer;
    img_data->MSK_srfc = MSK_srfc;
    img_data->width    = width;
    img_data->height   = height;

    return true;
}

void Convert_SURFACE_to_MSK(Surface* surface, image_data* img_data, int cutoff)
{
    int width  = surface->w;
    int height = surface->h;
    int size   = width * height;
    uint8_t* data = (uint8_t*)calloc(1, size);

    Surface* Surface_32 = Convert_Surface_to_RGBA(surface, NULL);
    if (!Surface_32) {
        set_popup_warning(
            "[ERROR] Convert_SURFACE_to_MSK()\n\n"
            "Unable to allocate surface for MSK"
        );
        printf("[ERROR] Unable to allocate surface for MSK\n");
        return;
    }

    Color rgba;
    int white = 1;
    int i;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            i = (Surface_32->pitch * y) + x * (sizeof(Color));
            memcpy(&rgba, (uint8_t*)Surface_32->pxls + i, sizeof(Color));

            if (rgba.r > cutoff || rgba.g > cutoff || rgba.b > cutoff) {
                data[y*width + x] = white;
            }
        }
    }
    img_data->MSK_data = data;
    img_data->type = MSK;
}