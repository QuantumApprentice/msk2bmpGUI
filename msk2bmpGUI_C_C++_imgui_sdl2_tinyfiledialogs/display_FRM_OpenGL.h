#pragma once
#include "shaders/shader_class.h"
#include "load_FRM_OpenGL.h"
#include <SDL.h>

//draw to framebuffer and main window functions
mesh load_giant_triangle();
//void draw_to_window(struct image_data* img_data, Shader* shader, mesh* triangle);
void draw_FRM_to_framebuffer(float* palette, Shader* shader, mesh* triangle, struct image_data* img_data);
void draw_PAL_to_framebuffer(float* palette, Shader* shader, mesh* triangle, struct image_data* img_data);
void draw_MSK_to_framebuffer(float* palette, Shader* shader, mesh* triangle, struct image_data* img_data);
