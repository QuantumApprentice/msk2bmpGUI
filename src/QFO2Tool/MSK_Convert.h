#pragma once
#include "MiniSDL.h"
#include "load_FRM_OpenGL.h"
#include "Load_Files.h"

#define MAX_LINES 300
#define writelines(arg1, arg2) fwrite(arg2, MAX_LINES * 44, 1, arg1)

typedef uint8_t line_array_t[MAX_LINES][44];

bool IsBMPFile(FILE *infile);
bool ReadBmpLines(FILE *file, line_array_t vOutput);            // same as below
void Read_MSK_Tile(FILE *file, uint8_t vOutput[MAX_LINES][44]);
int BytesToInt(char *C, int numBytes);

bool Load_MSK_Tile_SURFACE(char* FileName, image_data* img_data);

void Convert_SURFACE_to_MSK(Surface* surface, image_data* img_data, int cutoff);
