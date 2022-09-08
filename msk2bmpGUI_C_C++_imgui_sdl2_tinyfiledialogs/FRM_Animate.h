#pragma once
void Color_Cycle(SDL_Color * PaletteColors, 
                    uint16_t* g_dwCurrent, 
                    int pal_num, 
                    uint8_t * colors, 
                    int count);
void AnimatePalette(SDL_Color * PaletteColors);
void Image_Color_Cycle(SDL_Surface* PAL_Surface, int i, SDL_Color* PaletteColors, SDL_Surface* Final_Render);
