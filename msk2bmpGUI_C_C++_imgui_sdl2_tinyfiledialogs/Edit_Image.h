#pragma once
#include <SDL.h>
#include "Image2Texture.h"

void Edit_Image(variables* My_Variables, int counter, SDL_Event* event);
void Create_Map_Mask(variables* My_Variables, int counter);
void Edit_Map_Mask(variables* My_Variables, SDL_Event* event, int counter, ImVec2 Origin);
void CPU_Blend(SDL_Surface* surface1, SDL_Surface* surface2);
