#pragma once
// #include <SDL.h>
#include "Image2Texture.h"


void Edit_Image(variables* My_Variables, ImVec2 img_pos, image_data* edit_data, ANM_Dir* edit_srfc, Surface* edit_MSK_srfc, bool edit_MSK, bool Palette_Update, uint8_t* Color_Pick);
void brush_size_handler(variables* My_Variables);
ImVec2 display_img_ImGUI(variables* My_Variables, image_data* edit_data);

