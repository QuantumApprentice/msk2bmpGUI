#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

//color cycling
bool load_palette_to_array(float* data);
void update_palette_array(float* palette, double CurrentTime, bool* Palette_Update);
void Color_Cycle(float* PaletteColors, int* g_dwCurrent, int pal_index, uint8_t * cycle_colors, int cycle_count);