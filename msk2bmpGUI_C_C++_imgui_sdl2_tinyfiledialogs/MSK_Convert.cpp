#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <memory.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "MSK_Convert.h"
#include "tinyfiledialogs.h"

// Windows BITMAPINFOHEADER format, for historical reasons
char bmpHeader[62] = {
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



// Lines of bits. Essentially the raw MSK file.
line_array_t inputLines;
int i = 0;
FILE *infile;

//TODO: allow drag and drop .MSK files to be passed into here
int MSK_Convert(char* File_Name, const char ** argv)
{
    bool bFlipVertical = true;
    const char* FileName = argv[1];

    fopen_s(&infile, File_Name, "rb");

    if (infile == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to open file.");
        return 1;
    }

    // Extension for output file.
    char *TargetExtension = { "" };

    // Load Input File
    //TODO: modify IsBMPFile() check to switch between
    //      files dropped on here and surfaces passed
    //      in from the converter
    bool bBMP2MSK = false;

    if (bBMP2MSK)
    {
        if (!ReadBmpLines(infile, inputLines))
        {
            printf("[ERROR] Unable to read BMP file");
            fclose(infile);
            return 1;
        }
        TargetExtension = "MSK";
    }
    else {
        ReadMskLines(infile, inputLines);
        TargetExtension = "BMP";
    }
    fclose(infile);

    // Change format if needed
    // MSK files are top-left origin. 
    // BMP files are bottom-left origin.
    // (Some BMP files are top-left origin. 
    // These ones will end up flipping the 
    // image until they are addressed in ReadBmpLines)

    //TODO: is flipping necessary for binary SDL surface?
    if (bFlipVertical)
    {
        for (int i = 0; i < MAX_LINES / 2; i++)
        {
            char buffer[44];
            memcpy(buffer, inputLines[i], 44);
            memcpy(inputLines[i], inputLines[MAX_LINES - 1 - i], 44);
            memcpy(inputLines[MAX_LINES - 1 - i], buffer, 44);
        }
    }

    // Open Output File
    char *sOutputFileName;
    int y = strlen(argv[1]) + 1;
    sOutputFileName = (char *)calloc(y, sizeof(char));
    memcpy(sOutputFileName, FileName, y);

    strncpy(&sOutputFileName[y - 4], TargetExtension, 3);

    // Write Output File
    //TODO: this needs a little cleanup
    //      and probably refactor into a helper function
    FILE *fb;
    fb = fopen(sOutputFileName, "wb");
    FILE *outfile = fb;
    if (bBMP2MSK)
    {
        writelines(outfile, inputLines);
    }
    else
    {
        fwrite(bmpHeader, 1, 62, outfile);
        writelines(outfile, inputLines);
    }
    fclose(fb);
}

void ReadMskLines(FILE *file, uint8_t vOutput[MAX_LINES][44])
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

SDL_Surface* Load_MSK_Image(char* FileName)
{
    //open the file & error checking
    fopen_s(&infile, FileName, "rb");
    if (infile == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to open file.");
        return NULL;
    }
    //read the binary lines in
    ReadMskLines(infile, inputLines);

    ///*this section was used to convert the pixels using SDL_ConvertSurface...
    ///*works, but needs palette to get correct coloring
    ////Create the binary_bitmap surface
    //SDL_Surface* binary_bitmap;
    //binary_bitmap = SDL_CreateRGBSurface(0, 350, 300, 1, 0, 0, 0, 0);
    ////copy inputLines to binary_bitmap surface
    //memcpy(binary_bitmap->pixels, inputLines, MAX_LINES * 44);
    ////convert to regular 32bit surface
    //SDL_PixelFormat* pxlFMT_32 = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    //SDL_Surface*temp_surface = SDL_ConvertSurfaceFormat(binary_bitmap, SDL_PIXELFORMAT_RGBA8888, 0);

    SDL_Surface* Mask_Surface = SDL_CreateRGBSurface(0, 350, 300, 32, 0, 0, 0, 0);
    //TODO: refactor this and make sure the inputLines buffer
    //      matches the other buffer for exporting
    uint8_t *bin_ptr = (uint8_t*)inputLines;
    //int shift = 0;
    uint8_t bitmask = 128;
    uint8_t buff = 0;
    bool mask_1_or_0;
    SDL_Color white = { 255,255,255 };

    for (int pxl_y = 0; pxl_y < 300; pxl_y++)
    {
        for (int pxl_x = 0; pxl_x < 350; pxl_x++)
        {
            buff = *bin_ptr;

            mask_1_or_0 = (buff & bitmask);
            if (mask_1_or_0) {
                *((SDL_Color*)Mask_Surface->pixels + (pxl_y * Mask_Surface->pitch/4) + pxl_x) = white;
            }

            bitmask >>= 1;

            if (bitmask == 0)
            {
                ++bin_ptr;
                bitmask = 128;
            }
        }
        //bitmask <<= 2 /* final shift */;
        ++bin_ptr;
        bitmask = 128;
    }

    printf(SDL_GetError());

    return Mask_Surface;


}

void Save_MSK_Image(SDL_Surface* surface, FILE* File_ptr, int x, int y)
{
    uint8_t out_buffer[13200] /*= { 0 }/* ceil(350/8) * 300 */;
    uint8_t *outp = out_buffer;

    int shift = 0;
    uint8_t bitmask = 0;
    bool mask_1_or_0;

    int pixel_pointer = surface->pitch * y * 300 + x * 350;
    //don't need to flip for the MSK (maybe need to flip for bitmaps)
    for (int pxl_y = 0; pxl_y < 300; pxl_y++)
    {
        for (int pxl_x = 0; pxl_x < 350; pxl_x++)
        {
            bitmask <<= 1;
            mask_1_or_0 =
                *((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) > 0;
            //*((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) & 1;
            //*((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) > 0 ? 1 : 0;
            bitmask |= mask_1_or_0;
            if (++shift == 8)
            {
                *outp = bitmask;
                ++outp;
                shift = 0;
                bitmask = 0;
            }
        }
        bitmask <<= 2 /* final shift */;
        *outp = bitmask;
        ++outp;
        shift = 0;
        bitmask = 0;
    }
    writelines(File_ptr, out_buffer);
    fclose(File_ptr);
}