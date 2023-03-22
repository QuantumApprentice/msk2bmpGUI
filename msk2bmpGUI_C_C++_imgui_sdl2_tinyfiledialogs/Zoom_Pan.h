#pragma once
#include "load_FRM_OpenGL.h"

void zoom(float zoom_level, struct position focus_point, float* old_zoom, float* new_zoom, ImVec2* corner_pos);
void zoom_wrap(float zoom_level, image_data* img_data);
struct position mouse_pos_to_texture_coord(struct position pos, float new_zoom, int frame_width, int frame_height, float* bottom_left_pos);
void panning(struct image_data* img_data, struct position offset);

