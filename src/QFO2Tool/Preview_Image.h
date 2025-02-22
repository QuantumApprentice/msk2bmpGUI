#pragma once
#include "Load_Files.h"
#include "Image2Texture.h"

void Preview_FRM_Image(variables* My_Variables, struct image_data* img_data, bool show_stats);
void Preview_MSK_Image(variables* My_Variables, struct image_data* img_data, bool show_stats);
void Preview_Image(variables* My_Variables, struct image_data* img_data, bool show_stats);
void draw_red_squares(image_data* img_data, bool wrong_size);
void draw_red_tiles(image_data* img_data, bool wrong_size);


void show_image_stats_FRM_SURFACE(image_data* img_data, ImFont* font);
void show_image_stats_FRM(image_data* img_data, ImFont* font);



void show_image_stats_ANM(image_data* img_data, ImFont* font);
void show_image_stats_MSK(image_data* img_data, ImFont* font);
void preview_FRM_SURFACE(variables* My_Variables, struct image_data* img_data, bool show_stats);