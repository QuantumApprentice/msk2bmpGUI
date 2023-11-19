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



// Lines of bits. Essentially the raw MSK file.
int i = 0;
FILE *infile;
line_array_t inputLines;

//TODO: allow drag and drop .MSK files to be passed into here
//also TODO: re-write this entire thing to handle errors and wide character files
int MSK_Convert(char* File_Name, const char ** argv)
{
    bool bFlipVertical = true;
    const char* FileName = argv[1];

#ifdef QFO2_WINDOWS
    fopen_s(&infile, File_Name, "rb");
#elif defined(QFO2_LINUX)
    infile = fopen(File_Name, "rb");
#endif

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
        Read_MSK_Tile(infile, inputLines);
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
//TODO: might want to move this entire function to Save_Files.cpp
//      and swap out ReadMskLines for something more generic
bool Load_MSK_Tile_OpenGL(char* FileName, image_data* img_data)
{
    if (img_data->FRM_data == NULL) {
        if (Load_MSK_File_OpenGL(FileName, img_data, TILE_W, TILE_H)) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        //TODO: change this to load a new file type, different extension, include width/height in the header
        if (Load_MSK_File_OpenGL(FileName, img_data, img_data->width, img_data->height)) {
            return true;
        }
        else {
            return false;
        }
    }
}

bool Load_MSK_File_OpenGL(char* FileName, image_data* img_data, int width, int height)
{
    //TODO: refactor this and make sure the inputLines/file_buffer buffer
    //      matches the other buffer for exporting

    FILE* File_ptr;

    int buffsize = (width + 7) / 8 * height;
    uint8_t* MSK_buffer = (uint8_t*)malloc(buffsize);

    //open the file & error checking
#ifdef QFO2_WINDOWS
    fopen_s(&File_ptr, FileName, "rb");
#elif defined(QFO2_LINUX)
    File_ptr = fopen(FileName, "rb");
#endif

    if (File_ptr == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to open file.");
        return false;
    }

    //read the binary lines in
    fread(MSK_buffer, buffsize, 1, File_ptr);
    fclose(File_ptr);

    uint8_t bitmask = 128;
    uint8_t buff = 0;
    bool mask_1_or_0 = false;
    uint8_t white    = 1;
    uint8_t* bin_ptr = MSK_buffer;
    uint8_t* data    = (uint8_t*)calloc(1, width*height);

    if (data) {
        for (int pxl_y = 0; pxl_y < height; pxl_y++)
        {
            for (int pxl_x = 0; pxl_x < width; pxl_x++)
            {
                buff = *bin_ptr;

                mask_1_or_0 = (buff & bitmask);
                if (mask_1_or_0) {
                    *(data + (pxl_y * width) + pxl_x) = white;
                }
                bitmask >>= 1;

                if (bitmask == 0)
                {
                    ++bin_ptr;
                    bitmask = 128;
                }
            }
            if (bitmask < 128) {
                ++bin_ptr;
                bitmask = 128;
            }
        }

        img_data->width  = width;
        img_data->height = height;

        //load & gen texture
        glGenTextures(1, &img_data->MSK_texture);
        glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
        //texture settings
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //MSK's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //bind data to FRM_texture for display
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        img_data->MSK_data = data;

        free(MSK_buffer);
        return true;
    }
    else {
        printf("MSK image didn't load...\n"); 
        return false;
    }
}

//bool GetBit(this byte b, int bitNumber)
//{
//    return (b & (1 << bitNumber)) != 0;
//}

union Pxl_info_32 {
    struct {
        uint8_t a;
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
    uint8_t arr[4];
};

void Convert_SDL_Surface_to_MSK(SDL_Surface* surface, LF* F_Prop, image_data* img_data)
{
    int width  = surface->w;
    int height = surface->h;
    int size   = width * height;
    uint8_t* data = (uint8_t*)calloc(1, size);

    SDL_PixelFormat* pxlFMT_UnPal = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
    SDL_Surface* Surface_32 = SDL_ConvertSurface(surface, pxlFMT_UnPal, 0);
    if (!Surface_32) {
        printf("Error: %s\n", SDL_GetError());
    }

    Pxl_info_32 rgba;
    int white = 1;
    int i;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            i = (Surface_32->pitch * y) + x * (sizeof(Pxl_info_32));
            memcpy(&rgba, (uint8_t*)Surface_32->pixels + i, sizeof(Pxl_info_32));

            if (rgba.r > 0 || rgba.g > 0 || rgba.b > 0) {
                data[y*width + x] = white;
            }
        }
    }
    img_data->MSK_data = data;
    F_Prop->img_data.type = MSK;
}

SDL_Surface* Load_MSK_Tile_SDL(char* FileName)
{
    //open the file & error checking
#ifdef QFO2_WINDOWS
    fopen_s(&infile, FileName, "rb");
#elif defined(QFO2_LINUX)
    infile = fopen(FileName, "rb");
#endif

    if (infile == NULL)
    {
        fprintf(stderr, "[ERROR] Unable to open file.");
        return NULL;
    }
    //read the binary lines in
    Read_MSK_Tile(infile, inputLines);

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
    printf(SDL_GetError());

    SDL_Surface* Mask_Surface = SDL_CreateRGBSurface(0, TILE_W, TILE_H, 32, 0, 0, 0, 0);
    printf(SDL_GetError());
    //TODO: refactor this and make sure the inputLines buffer
    //      matches the other buffer for exporting
    uint8_t *bin_ptr = (uint8_t*)inputLines;
    //int shift = 0;
    uint8_t bitmask = 128;
    uint8_t buff = 0;
    bool mask_1_or_0;
    SDL_Color white = { 255,255,255,128 };

    for (int pxl_y = 0; pxl_y < TILE_H; pxl_y++)
    {
        for (int pxl_x = 0; pxl_x < TILE_W; pxl_x++)
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

