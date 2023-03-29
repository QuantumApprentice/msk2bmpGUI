#pragma once
#include "load_FRM_OpenGL.h"

ImVec2 top_corner(image_data* img_data);
ImVec2 bottom_corner(ImVec2 size, ImVec2 corner_pos);
void zoom_pan(image_data* img_data, position focus_point, position mouse_delta);
void zoom(float zoom_level, struct position focus_point, image_data* img_data, ImVec2* corner_pos);
void panning(struct image_data* img_data, struct position offset);
void viewport_boundary(image_data* img_data, ImVec2 size);
