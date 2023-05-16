#pragma once
#include "Load_Files.h"
#include "Image2Texture.h"

void Preview_FRM_Image(variables* My_Variables, struct image_data* img_data);
void Preview_Image(variables* My_Variables, struct image_data* img_data);
void draw_red_squares(LF* F_Prop, bool wrong_size);
void show_image_stats_FRM(image_data* img_data, ImFont* font);
void show_image_stats_ANIM(image_data* img_data, ImFont* font);
