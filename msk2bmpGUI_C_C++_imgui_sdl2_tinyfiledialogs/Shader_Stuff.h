#pragma once
#include <glad/glad.h>
#include "shaders/shader_class.h"

//mesh used for storing shader info
struct mesh {
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint vertexCount;
};

mesh load_giant_triangle();
void init_framebuffer(unsigned int* framebuffer, unsigned int* texture);
void shader_setup(mesh triangle, float width, float height, GLuint texture, GLuint framebuffer);
void load_palette_2D(unsigned int* texture);
void load_palette_1D(unsigned int* texture);
void init_textures(unsigned int* texture);
