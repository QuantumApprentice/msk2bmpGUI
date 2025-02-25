#pragma once
#include "load_FRM_OpenGL.h"
#include "MiniSDL.h"

bool Crop_Animation(image_data* img_data, image_data* edit_data, Palette* FO_Palette);
bool crop_animation_SURFACE(image_data* src, image_data* dst, Palette* pal, int algo, shader_info* shaders);
