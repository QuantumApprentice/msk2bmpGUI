//https://falloutmods.fandom.com/wiki/PAL_File_Format
//https://falloutmods.fandom.com/wiki/Pal_animations#Animated_colors
#include "Palette_Cycle.h"
#include "tinyfiledialogs.h"
#include <Windows.h>

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

void update_palette_array(float* palette, double CurrentTime, bool* Palette_Update)
{
    {
        uint16_t g_dwCycleSpeedFactor = 1;

        if (CurrentTime - g_dwLastCycleSlow >= 200 * g_dwCycleSpeedFactor) {
            // Slime        ///////////////////////////////////////////////////////
            Color_Cycle(palette, &g_dwSlimeCurrent, 229, g_nSlime, 3);
            // Fire_slow    ///////////////////////////////////////////////////////
            Color_Cycle(palette, &g_dwFireSlowCurrent, 238, g_nFireSlow, 4);
            // Shoreline    ///////////////////////////////////////////////////////
            Color_Cycle(palette, &g_dwShorelineCurrent, 248, g_nShoreline, 5);

            g_dwLastCycleSlow = CurrentTime;
            *Palette_Update = true;
        }

        if (CurrentTime - g_dwLastCycleMedium >= 142 * g_dwCycleSpeedFactor) {
            // Fire_fast    ///////////////////////////////////////////////////////
            Color_Cycle(palette, &g_dwFireFastCurrent, 243, g_nFireFast, 4);

            g_dwLastCycleMedium = CurrentTime;
            *Palette_Update = true;
        }

        if (CurrentTime - g_dwLastCycleFast >= 100 * g_dwCycleSpeedFactor) {
            // Monitors     ///////////////////////////////////////////////////////
            Color_Cycle(palette, &g_dwMonitorsCurrent, 233, g_nMonitors, 4);

            g_dwLastCycleFast = CurrentTime;
            *Palette_Update = true;
        }

        if (CurrentTime - g_dwLastCycleVeryFast >= 33 * g_dwCycleSpeedFactor) {
            // Blinking red ///////////////////////////////////////////////////////
            //TODO: need to fix this color cycle...doesn't update in ImGui correctly yet
            if ((g_nBlinkingRedCurrent == 0) || (g_nBlinkingRedCurrent == 60*4))
            { g_nBlinkingRed = -g_nBlinkingRed; }

            palette[254*3+0] = (g_nBlinkingRedCurrent + g_nBlinkingRed) / 255.0f;
            palette[254*3+1] = 0;
            palette[254*3+2] = 0;

            /* color value range
              0,  16,  32,  48,
             64,  80,  96, 112,
            128, 142, 160, 176,
            192, 208, 224, 240
            */

            g_nBlinkingRedCurrent += g_nBlinkingRed;

            g_dwLastCycleVeryFast = CurrentTime;
            *Palette_Update = true;
        }
    }
}

void Color_Cycle(float* PaletteColors, int* g_dwCurrent, int pal_index, uint8_t * cycle_colors, int cycle_count)
{
    uint16_t Current_Frame = *g_dwCurrent;

    for (int i = cycle_count; i >= 0; i--) {
        PaletteColors[pal_index * 3 + i * 3 + 0] = cycle_colors[Current_Frame * 3 + 0] / 255.0; // Red
        PaletteColors[pal_index * 3 + i * 3 + 1] = cycle_colors[Current_Frame * 3 + 1] / 255.0; // Green
        PaletteColors[pal_index * 3 + i * 3 + 2] = cycle_colors[Current_Frame * 3 + 2] / 255.0; // Blue

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

bool load_palette_to_array(float* palette, char* exe_path)
{
    char path_buffer[MAX_PATH];
    snprintf(path_buffer, sizeof(path_buffer), "%s%s", exe_path, "palette/fo_color.pal");

    //file management
    FILE *File_ptr;
    errno_t error = _wfopen_s(&File_ptr, tinyfd_utf8to16(path_buffer), L"rb");
    if (error != 0) {
        printf("error %d, can't open palette", error);
        return false;
    }

    uint8_t uint8_t_data[256 * 3];

    //read in palette info
    size_t read_size = fread(uint8_t_data, 1, 768, File_ptr);
    if (read_size < 768) {
        printf("\nread_size: %I64u", read_size);
    }
    fclose(File_ptr);
    if (uint8_t_data) {
        for (int i = 0; i < 765; i++)
        {
            if (uint8_t_data[i] < 64) {
                uint8_t_data[i] *= 4;
            }
        }
        for (int i = 765; i < 768; i++)
        {
            uint8_t_data[i] = 0;
        }
        for (int i = 0; i < 256 * 3; i++)
        {
            palette[i] = uint8_t_data[i] / 255.0;
        }
        return true;
    }
    else {
        printf("Palette file opened, but couldn't read?");
        return false;
    }
}