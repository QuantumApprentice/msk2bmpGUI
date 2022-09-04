#pragma once
void Color_Cycle(SDL_Color * PaletteColors, 
                    uint16_t* g_dwCurrent, 
                    int pal_num, 
                    uint8_t * colors, 
                    int count);
void AnimatePalette(SDL_Color * PaletteColors);
