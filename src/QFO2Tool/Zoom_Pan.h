#pragma once
#include "load_FRM_OpenGL.h"

// ImVec2 top_corner(image_data* img_data);
ImVec2 top_corner(ImVec2 offset);
ImVec2 bottom_corner(ImVec2 size, ImVec2 corner_pos);
void zoom_pan(image_data* img_data, ImVec2 focus_point, ImVec2 mouse_delta);
void zoom(float zoom_level, ImVec2 focus_point, image_data* img_data);
void panning(struct image_data* img_data, ImVec2 offset);
void viewport_boundary(image_data* img_data, ImVec2 size);
