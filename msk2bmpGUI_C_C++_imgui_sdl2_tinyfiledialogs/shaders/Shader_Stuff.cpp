#pragma once
#include "Shader_Stuff.h"

mesh load_giant_triangle()
{
    float vertices[] = {
        //giant triangle     uv coordinates?
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         3.0f, -1.0f, 0.0f,  2.0f, 0.0f,
        -1.0f,  3.0f, 0.0f,  0.0f, 2.0f
    };

    mesh tri_mesh;
    tri_mesh.vertexCount = 3;

    glGenBuffers(1, &tri_mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, tri_mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Vertex Attrib Object
    glGenVertexArrays(1, &tri_mesh.VAO);
    glBindVertexArray(tri_mesh.VAO);

    //Vertex Attrib Pointer
        //Triangle vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //UV position (texture)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //Unbind buffers in case other buffers need to be used?
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return tri_mesh;
}

void init_framebuffer(unsigned int framebuffer, unsigned int texture, int width, int height)
{
    //random data to start///////////////////////////////////////////////////////////
    int w = width;
    int h = height;
    uint32_t* initialData = (uint32_t*)malloc(w * h * sizeof(uint32_t));
    memset(initialData, 0, sizeof(uint32_t)*w * h);

    //random starting stuff for both textures
    for (int i = 0; i < (w*h); i++)
    {
        initialData[i] = (rand() % 16 == 0)*(0xFFFFFFFF);
    }

    //start of Framebuffer stuff///////////////////////////////////////////////////////////
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(    1, &texture);
    glGenTextures(       1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //allocate memory for the texture?
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        w, h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, initialData);
    //framebuffer stuff
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    //attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    //unbind texture after setting values
    glBindTexture(GL_TEXTURE_2D, 0);

    //check if framebuffer works?
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //end of Framebuffer stuff/////////////////////////////////////////////////////////////
}

void init_textures(GLuint* texture, int frame_width, int frame_height)
{
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //std::cout << glGetError();
//allocate memory for the texture?
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        //this size is supposed to be different from display?
        frame_width, frame_height,
        0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

void shader_setup(mesh triangle, float width, float height,
                  GLuint texture, GLuint framebuffer, Shader shader)
{
    float dimensions[2] = { width, height };
    glBindVertexArray(triangle.VAO);
    glActiveTexture(GL_TEXTURE0);

    glViewport(0, 0, width, height);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    shader.use();
    glUniform2fv(glGetUniformLocation(shader.ID, "scale"), 1, dimensions);
    glDrawArrays(GL_TRIANGLES, 0, triangle.vertexCount);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void load_palette_1D(unsigned int* texture)
{
    // load and generate texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_1D, *texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // texture filtering settings
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    FILE *File_ptr;
    fopen_s(&File_ptr, "images/color.pal", "rb");
    if (!File_ptr) {
        printf("error, can't open file");
    }

    unsigned char* data = (unsigned char*)malloc(768);
    // Read in the pixels from the FRM file to the texture
    fread(data, 1, 768, File_ptr);
    fclose(File_ptr);
    if (data) {
        for (int i = 0; i < 768; i++)
        {
            if (data[i] < 64) {
                data[i] *= 4;
            }
        }
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        //glGenerateMipmap(GL_TEXTURE_1D);
    }
    else {
        std::cout << "Failure to communicate..." << std::endl;
    }
}

void load_palette_2D(unsigned int* texture)
{
    // load and generate texture
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // texture filtering settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    FILE *File_ptr;
    fopen_s(&File_ptr, "images/color.pal", "rb");
    if (!File_ptr) {
        printf("error, can't open file");
    }

    unsigned char* data = (unsigned char*)malloc(768 * 6);
    // Read in the pixels from the FRM file to the texture
    fread(data, 1, 768, File_ptr);
    fclose(File_ptr);
    if (data) {
        for (int i = 0; i < 768; i++)
        {
            if (data[i] < 64) {
                data[i] *= 4;
            }
        }
        {//color cycling stuff
            uint8_t g_nSlime[] =        { 0,   108,   0,   11, 115,   7,     27, 123,  15,     43, 131,  27 };                                  // Slime
            uint8_t g_nMonitors[] =     { 107, 107, 111,   99, 103, 127,     87, 107, 143,      0, 147, 163,    107, 187, 255 };                // Monitors
            uint8_t g_nFireSlow[] =     { 255,   0,   0,  215,   0,   0,    147,  43,  11,    255, 119,   0,    255,  59,   0 };                // Slow fire
            uint8_t g_nFireFast[] =     { 71,    0,   0,  123,   0,   0,    179,   0,   0,    123,   0,   0,     71,   0,   0 };                // Fast fire
            uint8_t g_nShoreline[] =    { 83,   63,  43,   75,  59,  43,     67,  55,  39,     63,  51,  39,     55,  47,  35,   51, 43, 35 };  // Shoreline
            uint8_t g_nBlinkingRed[] =  { 252,   0,   0,  168,   0,   0,     84,   0,   0,      0,   0,   0,     84,   0,   0,   168, 0,  0 };

            int size = 0;
            uint8_t* ptr = data + 229 * 3;

            //first row
            size = sizeof(g_nSlime);
            memcpy(ptr, g_nSlime, size);
            ptr += size;
            size = sizeof(g_nMonitors);
            memcpy(ptr, g_nMonitors, size);
            ptr += size;
            size = sizeof(g_nFireSlow);
            memcpy(ptr, g_nFireSlow, size);
            ptr += size;
            size = sizeof(g_nFireFast);
            memcpy(ptr, g_nFireFast, size);
            ptr += size;
            size = sizeof(g_nShoreline);
            memcpy(ptr, g_nShoreline, size);
            ptr += size;
            size = sizeof(g_nBlinkingRed) / 6;
            memcpy(ptr, g_nBlinkingRed, size);
            //2nd row

            uint8_t* last_row = data;


            //copy last color in cycle from previous row to first position of current row
            for (int i = 0; i < 5; i++)
            {
                uint8_t* current_row = last_row + 256 * 3;
                //copy first 229 colors
                memcpy(current_row, data, 229 * 3);
                //copy rest of colors but shifted over 1 (ie 229 becomes 230)
                memcpy(current_row + 230 * 3, last_row + 229 * 3, (255 - 229) * 3);

                ptr = current_row + 229 * 3;
                size = sizeof(g_nSlime);
                memcpy(ptr, ptr - 256 * 3 + size - 3, 3);
                ptr += size;
                size = sizeof(g_nMonitors);
                memcpy(ptr, ptr - 256 * 3 + size - 3, 3);
                ptr += size;
                size = sizeof(g_nFireSlow);
                memcpy(ptr, ptr - 256 * 3 + size - 3, 3);
                ptr += size;
                size = sizeof(g_nFireFast);
                memcpy(ptr, ptr - 256 * 3 + size - 3, 3);
                ptr += size;
                size = sizeof(g_nShoreline);
                memcpy(ptr, ptr - 256 * 3 + size - 3, 3);
                ptr += size;
                //special code for the blinking red color
                size = 3;
                memcpy(ptr, &g_nBlinkingRed[3 * (i + 1)], 3);
                ptr += size;
                //set last color to 255
                uint8_t arr[3] = { 255, 255, 255 };
                memcpy(ptr, arr, 3);

                last_row = current_row;

            }

        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 6, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Failure to communicate..." << std::endl;
    }
}