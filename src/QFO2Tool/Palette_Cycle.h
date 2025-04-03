#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>


typedef union Color {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint8_t clr[4];
    uint32_t rgba;
} Color;

//vanilla fallout palette only uses first 228 colors,
//the rest are hardcoded color cycling
//palettes can be swapped by providing a palette
//named the same as the FRM but with .PAL extension
//placed right next to each other
typedef struct Palette {
    int num_colors = 228;
    Color colors[256];
} Palette;

//color cycling
bool update_PAL_array(Palette* pal, double CurrentTime);//, bool* Palette_Update);