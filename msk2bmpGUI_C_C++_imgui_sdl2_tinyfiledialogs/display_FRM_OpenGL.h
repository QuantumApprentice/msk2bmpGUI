#pragma once
#include "shaders/shader_class.h"
#include "load_FRM_OpenGL.h"
#include <SDL.h>
#include <time.h>

//draw to framebuffer and main window functions
mesh load_giant_triangle();
//void draw_to_window(struct image_data* img_data, Shader* shader, mesh* triangle);
void animate_FRM_to_framebuff(float* palette, Shader* shader, mesh* triangle, image_data* img_data, clock_t time, bool palette_update);

void draw_FRM_to_framebuffer(float* palette, Shader* shader, mesh* triangle, struct image_data* img_data);
void draw_PAL_to_framebuffer(float* palette, Shader* shader, mesh* triangle, struct image_data* img_data);
void draw_MSK_to_framebuffer(float* palette, Shader* shader, mesh* triangle, struct image_data* img_data);
