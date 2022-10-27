#pragma once
#include <SDL.h>
#include "Image2Texture.h"

void Edit_Image(LF* F_Prop, bool Palette_Update, SDL_Event* event, uint8_t* Color_Pick);
void Create_Map_Mask(LF* F_Prop);
void Edit_Map_Mask(LF* F_Prop, SDL_Event* event, bool* Palette_Update, ImVec2 Origin);
void CPU_Blend(SDL_Surface* surface1, SDL_Surface* surface2);
void Update_Palette(struct LF* files, bool blend);
//void Update_Palette2(struct LF* files, SDL_PixelFormat* pxlFMT);
void Update_Palette2(SDL_Surface* surface, GLuint* texture, SDL_PixelFormat* pxlFMT);
//void Update_Palette(variables* My_Variables, int counter, bool blend);
