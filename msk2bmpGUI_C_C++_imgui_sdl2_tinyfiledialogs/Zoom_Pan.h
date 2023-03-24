#pragma once
#include "load_FRM_OpenGL.h"

void init_image_pos(image_position* img_pos, ImVec2 size);
void zoom_pan(image_data* img_data, position focus_point, position mouse_delta, ImVec2 size);
void zoom(float zoom_level, struct position focus_point, image_position* img_pos);
void panning(struct image_data* img_data, struct position offset);

