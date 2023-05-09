#pragma once
#include <glad/glad.h>
#include <time.h>

#include "shader_class.h"

//mesh used for storing shader info
struct mesh {
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;
    GLuint vertexCount = 0;
};

struct position
{
    double x;
    double y;
};

mesh load_giant_triangle();
void init_framebuffer(unsigned int* framebuffer, unsigned int* texture, int width, int height);
void shader_setup(mesh triangle, float width, float height, GLuint texture, GLuint framebuffer, Shader shader);
void load_palette_2D(unsigned int* texture);
void load_palette_1D(unsigned int* texture);
void init_textures(GLuint* texture, int frame_width, int frame_height);
void draw_to_framebuffer(unsigned int* framebuffer1, unsigned int* framebuffer2, unsigned int* texture1, unsigned int* texture2, Shader* ourShader, mesh* giant_triangle, float new_zoom, float* bottom_left_pos, int frm_width, int frm_height, clock_t time);