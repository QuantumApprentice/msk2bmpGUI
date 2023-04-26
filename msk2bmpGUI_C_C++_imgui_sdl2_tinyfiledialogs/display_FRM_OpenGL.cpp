#include "display_FRM_OpenGL.h"
#include "B_Endian.h"

mesh load_giant_triangle()
{
    float vertices[] = {
        //giant triangle     uv coordinates?
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         3.0f, -1.0f, 0.0f,  2.0f, 0.0f,
        -1.0f,  3.0f, 0.0f,  0.0f, 2.0f
    };

    mesh triangle;
    triangle.vertexCount = 3;

    glGenVertexArrays(1, &triangle.VAO);
    glBindVertexArray(triangle.VAO);

    glGenBuffers(1, &triangle.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, triangle.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return triangle;
}

void animate_FRM_to_framebuff(float* palette, Shader* shader, mesh* triangle,
                              image_data* img_data, clock_t time, int* q)
{
    int frm_width  = img_data->Frame[*q].frame_info->Frame_Width;
    int frm_height = img_data->Frame[*q].frame_info->Frame_Height;
    uint32_t size  = img_data->Frame[*q].frame_info->Frame_Size;
    uint8_t* data  = (uint8_t*)(img_data->Frame[*q].frame_info + 1);

    glViewport(0, 0, 51, 79);

    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

    static int b = 0;
    if (b != *q) {

        b = *q;

        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //FRM's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //bind data to FRM_texture for display
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frm_width, frm_height, 0, GL_RED, GL_UNSIGNED_BYTE, data);



        ////Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        ////FRM's are aligned to 1-byte
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        ////bind data to FRM_texture for display
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RED, GL_UNSIGNED_BYTE, data + pxl_ptr);

    }
    else {
        glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    }

    //shader
    shader->use();
    glUniform3fv(glGetUniformLocation(shader->ID, "ColorPalette"), 256, palette);
    shader->setInt("Indexed_FRM", 0);
    //draw image to framebuffer
    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_FRM_to_framebuffer(float* palette,
                             Shader* shader, mesh* triangle,
                             image_data* img_data)
{
    glViewport(0, 0, img_data->width, img_data->height);

    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

    //shader
    shader->use();
    glUniform3fv(glGetUniformLocation(shader->ID, "ColorPalette"), 256, palette);
    shader->setInt("Indexed_FRM", 0);

    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_PAL_to_framebuffer(float* palette, Shader* shader,
                            mesh* triangle, struct image_data* img_data)
{
    glViewport(0, 0, img_data->width, img_data->height);

    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);

    //shader
    shader->use();
    glUniform3fv(glGetUniformLocation(shader->ID, "ColorPalette"), 256, palette);
    shader->setInt("Indexed_FRM", 0);
    shader->setInt("Indexed_PAL", 1);
    shader->setInt("Indexed_MSK", 2);

    //printf("glGetError: %d\n", glGetError());

    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);


    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void draw_MSK_to_framebuffer(float* palette,
                             Shader* shader, mesh* triangle,
                             struct image_data* img_data)
{
    glViewport(0, 0, img_data->width, img_data->height);

    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);

    //shader
    shader->use();
    glUniform3fv(glGetUniformLocation(shader->ID, "ColorPalette"), 256, palette);
    shader->setInt("Indexed_FRM", 0);

    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}