//https://falloutmods.fandom.com/wiki/PAL_File_Format
//https://falloutmods.fandom.com/wiki/Pal_animations#Animated_colors
#include "Palette_Cycle.h"
#include "tinyfiledialogs.h"
#include "ImGui_Warning.h"
#ifdef QFO2_WINDOWS
    #include <Windows.h>
#elif defined(QFO2_LINUX)
    #include "Load_Settings.h"
#endif

//color cycling stuff
// Palette color arrays      r,   g,   b
uint8_t g_nSlime[] =     {   0, 108,   0,       // Slime
                            11, 115,   7,
                            27, 123,  15,
                            43, 131,  27 };
uint8_t g_nMonitors[] =  { 107, 107, 111,       // Monitors
                            99, 103, 127,
                            87, 107, 143,
                             0, 147, 163,
                           107, 187, 255 };
uint8_t g_nFireSlow[] =  { 255,   0,   0,       // Slow fire
                           215,   0,   0,
                           147,  43,  11,
                           255, 119,   0,
                           255,  59,   0 };
uint8_t g_nFireFast[] =  {  71,   0,   0,       // Fast fire
                           123,   0,   0,
                           179,   0,   0,
                           123,   0,   0,
                            71,   0,   0 };
uint8_t g_nShoreline[] = {  83,  63,  43,       // Shoreline
                            75,  59,  43,
                            67,  55,  39,
                            63,  51,  39,
                            55,  47,  35,
                            51,  43,  35 };
int g_nBlinkingRed = { -4*4 };

struct cycle {
    // Current parameters of cycle
    int g_dwSlimeCurrent = 0;
    int g_dwMonitorsCurrent = 0;
    int g_dwFireSlowCurrent = 0;
    int g_dwFireFastCurrent = 0;
    int g_dwShorelineCurrent = 0;
    uint8_t g_nBlinkingRedCurrent = 0;

    // Time of last cycle
    double g_dwLastCycleSlow = 0;
    double g_dwLastCycleMedium = 0;
    double g_dwLastCycleFast = 0;
    double g_dwLastCycleVeryFast = 0;
} cycle_vals;

void color_cycle_PAL(Palette* pal, int* g_dwCurrent, int pal_index, uint8_t * cycle_colors, int cycle_count)
{
    uint16_t Current_Frame = *g_dwCurrent;

    for (int i = cycle_count; i >= 0; i--) {
        pal->colors[pal_index + i].r = cycle_colors[Current_Frame * 3 + 0];
        pal->colors[pal_index + i].g = cycle_colors[Current_Frame * 3 + 1];
        pal->colors[pal_index + i].b = cycle_colors[Current_Frame * 3 + 2];
        //all cycle colors have alpha of 255
        pal->colors[pal_index + i].a = 255;

        if (Current_Frame == cycle_count)
            Current_Frame = 0;
        else
            Current_Frame++;
    }

    if (*g_dwCurrent == cycle_count)
        *g_dwCurrent = 0;
    else
        (*g_dwCurrent)++;
}

//returns true if Palette is updated
//used to indicate the FRM needs to be re-rendered
bool update_PAL_array(Palette* pal, double CurrentTime)//, bool* Palette_Update)
{
    bool update = false;
    uint16_t g_dwCycleSpeedFactor = 1;

    if (CurrentTime - cycle_vals.g_dwLastCycleSlow >= 200 * g_dwCycleSpeedFactor) {
        // Slime        ///////////////////////////////////////////////////////
        color_cycle_PAL(pal, &cycle_vals.g_dwSlimeCurrent, 229, g_nSlime, 3);
        // Fire_slow    ///////////////////////////////////////////////////////
        color_cycle_PAL(pal, &cycle_vals.g_dwFireSlowCurrent, 238, g_nFireSlow, 4);
        // Shoreline    ///////////////////////////////////////////////////////
        color_cycle_PAL(pal, &cycle_vals.g_dwShorelineCurrent, 248, g_nShoreline, 5);

        cycle_vals.g_dwLastCycleSlow = CurrentTime;
        update = true;
    }

    if (CurrentTime - cycle_vals.g_dwLastCycleMedium >= 142 * g_dwCycleSpeedFactor) {
        // Fire_fast    ///////////////////////////////////////////////////////
        color_cycle_PAL(pal, &cycle_vals.g_dwFireFastCurrent, 243, g_nFireFast, 4);

        cycle_vals.g_dwLastCycleMedium = CurrentTime;
        update = true;
    }

    if (CurrentTime - cycle_vals.g_dwLastCycleFast >= 100 * g_dwCycleSpeedFactor) {
        // Monitors     ///////////////////////////////////////////////////////
        color_cycle_PAL(pal, &cycle_vals.g_dwMonitorsCurrent, 233, g_nMonitors, 4);

        cycle_vals.g_dwLastCycleFast = CurrentTime;
        update = true;
    }

    if (CurrentTime - cycle_vals.g_dwLastCycleVeryFast >= 33 * g_dwCycleSpeedFactor) {
        // Blinking red ///////////////////////////////////////////////////////
        //TODO: need to fix this color cycle...doesn't update in ImGui correctly yet
        if ((cycle_vals.g_nBlinkingRedCurrent == 0) || (cycle_vals.g_nBlinkingRedCurrent == 60*4))
        { g_nBlinkingRed = -g_nBlinkingRed; }

        pal->colors[254].r = (cycle_vals.g_nBlinkingRedCurrent + g_nBlinkingRed);
        pal->colors[254].g = 0;
        pal->colors[254].b = 0;
        pal->colors[254].a = 255;

        /* color value range
          0,  16,  32,  48,
         64,  80,  96, 112,
        128, 142, 160, 176,
        192, 208, 224, 240
        */

        cycle_vals.g_nBlinkingRedCurrent += g_nBlinkingRed;

        cycle_vals.g_dwLastCycleVeryFast = CurrentTime;
        update = true;
    }
    return update;
}