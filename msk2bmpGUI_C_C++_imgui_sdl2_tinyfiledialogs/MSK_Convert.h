#pragma once
#include "SDL.h"

#define MAX_LINES 300
#define writelines(arg1, arg2) fwrite(arg2, MAX_LINES * 44, 1, arg1)

typedef uint8_t line_array_t[MAX_LINES][44];

int MSK_Convert(char* File_Name, const char ** argv);
bool IsBMPFile(FILE *infile);
bool ReadBmpLines(FILE *file, line_array_t vOutput);            // same as below
void ReadMskLines(FILE *file, uint8_t vOutput[MAX_LINES][44]);
int BytesToInt(char *C, int numBytes);
SDL_Surface* Load_MSK_Image(char* FileName);
void Save_MSK_Image(SDL_Surface* surface, FILE* File_ptr, int x, int y);
