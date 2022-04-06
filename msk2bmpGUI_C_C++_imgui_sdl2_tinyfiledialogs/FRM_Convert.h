#pragma once
#include <SDL.h>
#include "Load_Files.h"

SDL_Color* loadPalette(char * name);
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface);
SDL_Surface* Load_Pal_Image(char *File_Name);
