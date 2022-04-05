#pragma once
#include <SDL.h>
#include "Load_Files.h"

SDL_Color* loadPalette(char * name);
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface);
SDL_Surface* BMP_Color_Convert(LF *F_Prop);
