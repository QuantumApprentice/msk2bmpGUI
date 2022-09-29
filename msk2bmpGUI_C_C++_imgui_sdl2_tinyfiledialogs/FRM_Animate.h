#pragma once
void Color_Cycle(SDL_Color * PaletteColors, 
                    uint16_t* g_dwCurrent, 
                    int pal_num, 
                    uint8_t * colors, 
                    int count);
void Cycle_Palette(SDL_Color * PaletteColors, bool* Palette_Update, uint32_t CurrentTime);
void Image_Color_Cycle(SDL_Surface* PAL_Surface, int i, SDL_Color* PaletteColors, SDL_Surface* Final_Render);
void Image_Color_Cycle2(uint8_t pal_color, int x, int y, SDL_Color* PaletteColors, SDL_Surface* Final_Render);
