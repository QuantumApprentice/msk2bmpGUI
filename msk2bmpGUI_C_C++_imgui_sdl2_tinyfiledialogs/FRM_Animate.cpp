//https://falloutmods.fandom.com/wiki/Pal_animations#Animated_colors
#include <stdio.h>
#include <cstdint>
#include <time.h>
#include <SDL.h>

#include "FRM_Animate.h"
#include "FRM_Convert.h"

//typedef uint8_t BYTE;
typedef uint8_t BYTE;
typedef uint16_t DWORD;

// Palette color arrays   r,   g,   b
BYTE g_nSlime[] =     {   0, 108,   0,                              // Slime
                         11, 115,   7,
                         27, 123,  15,
                         43, 131,  27 };
BYTE g_nMonitors[] =  { 107, 107, 111,                              // Monitors
                         99, 103, 127,
                         87, 107, 143,
                          0, 147, 163,
                        107, 187, 255 };
BYTE g_nFireSlow[] =  { 255,   0,   0,                              // Slow fire
                        215,   0,   0,
                        147,  43,  11,
                        255, 119,   0,
                        255,  59,   0 };
BYTE g_nFireFast[] =  {  71,   0,   0,                              // Fast fire
                        123,   0,   0,
                        179,   0,   0,
                        123,   0,   0,
                         71,   0,   0 };
BYTE g_nShoreline[] = {  83,  63,  43,                              // Shoreline
                         75,  59,  43,
                         67,  55,  39,
                         63,  51,  39,
                         55,  47,  35,
                         51,  43,  35 };
BYTE g_nBlinkingRed = { 252 };                                      // Alarm

// Current parameters of cycle
DWORD g_dwSlimeCurrent      = 0;
DWORD g_dwMonitorsCurrent   = 0;
DWORD g_dwFireSlowCurrent   = 0;
DWORD g_dwFireFastCurrent   = 0;
DWORD g_dwShorelineCurrent  = 0;
BYTE  g_nBlinkingRedCurrent = 0;

// Time of Last changinge
uint32_t g_dwLastCycleSlow     = 0;
uint32_t g_dwLastCycleMedium   = 0;
uint32_t g_dwLastCycleFast     = 0;
uint32_t g_dwLastCycleVeryFast = 0;

// Current speed factor
DWORD g_dwCycleSpeedFactor = 1;

void Image_Color_Cycle(SDL_Surface* PAL_Surface, int i, SDL_Color* PaletteColors, SDL_Surface* Final_Render)
{
    SDL_Color animated_pixel = PaletteColors[((uint8_t*)PAL_Surface->pixels)[i]];
    animated_pixel.a = 255;
    ((SDL_Color*)Final_Render->pixels)[i] = animated_pixel;
}

void Color_Cycle(SDL_Color * PaletteColors, uint16_t* g_dwCurrent, int pal_index, uint8_t * cycle_colors, int cycle_count)
{
    DWORD Current_Frame = *g_dwCurrent;
    for (int i = cycle_count; i >= 0; i--) {
        PaletteColors[pal_index + i] = SDL_Color {
                cycle_colors[Current_Frame * 3 + 0],                        // Red
                cycle_colors[Current_Frame * 3 + 1],                        // Green
                cycle_colors[Current_Frame * 3 + 2]                         // Blue
        };

        if (Current_Frame == cycle_count)
            Current_Frame = 0;
        else
            Current_Frame++;
    }

    if (*g_dwCurrent == 3)
        *g_dwCurrent = 0;
    else
        (*g_dwCurrent)++;
}

void AnimatePalette(SDL_Color * PaletteColors)
{
    bool bPaletteChanged = false;
    uint32_t dwCurrentTime = clock();//SDL_GetTicks();

    if (dwCurrentTime - g_dwLastCycleSlow >= 200 * g_dwCycleSpeedFactor) {
        // Slime    ///////////////////////////////////////////////////////
        Color_Cycle(PaletteColors, &g_dwSlimeCurrent, 229, g_nSlime, 3);
        // Shoreline    ///////////////////////////////////////////////////////
        Color_Cycle(PaletteColors, &g_dwShorelineCurrent, 248, g_nShoreline, 5);
        // Fire_slow    ///////////////////////////////////////////////////////
        Color_Cycle(PaletteColors, &g_dwFireSlowCurrent, 238, g_nFireSlow, 4);

        g_dwLastCycleSlow = dwCurrentTime;
        bPaletteChanged = true;
    }

    dwCurrentTime = clock();                //GetTickCount();

    if (dwCurrentTime - g_dwLastCycleMedium >= 142 * g_dwCycleSpeedFactor) {
        // Fire_fast    ///////////////////////////////////////////////////////
        Color_Cycle(PaletteColors, &g_dwFireFastCurrent, 243, g_nFireFast, 4);

        g_dwLastCycleMedium = dwCurrentTime;
        bPaletteChanged = true;
    }

    dwCurrentTime = clock();                //GetTickCount();

    if (dwCurrentTime - g_dwLastCycleFast >= 100 * g_dwCycleSpeedFactor) {
        // Monitors     ///////////////////////////////////////////////////////
        Color_Cycle(PaletteColors, &g_dwMonitorsCurrent, 233, g_nMonitors, 4);

        g_dwLastCycleFast = dwCurrentTime;
        bPaletteChanged = true;
    }

    dwCurrentTime = clock();                //GetTickCount();

    if (dwCurrentTime - g_dwLastCycleVeryFast >= 33 * g_dwCycleSpeedFactor) {
        // Blinking red     ///////////////////////////////////////////////////////
        if ((g_nBlinkingRedCurrent == 0) ||(g_nBlinkingRedCurrent == 60))
            g_nBlinkingRed = BYTE(-g_nBlinkingRed);

        PaletteColors[254] = SDL_Color{
                (uint8_t)(g_nBlinkingRed + g_nBlinkingRedCurrent),             // Red
                0,                                                             // Green
                0                                                              // Blue
        };

        g_nBlinkingRedCurrent = g_nBlinkingRed + g_nBlinkingRedCurrent;

        g_dwLastCycleVeryFast = dwCurrentTime;
        bPaletteChanged = true;
    }

    if (bPaletteChanged) {
    //  UpdatePalette();
    }
}