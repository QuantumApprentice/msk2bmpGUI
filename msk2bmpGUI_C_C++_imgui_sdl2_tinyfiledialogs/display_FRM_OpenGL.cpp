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

void render_FRM_OpenGL(image_data* img_data, int width, int height)
{
    int frame = img_data->display_frame_num;
    int orient = img_data->display_orient_num;
    int max_frm = img_data->FRM_Info.Frames_Per_Orient;
    int display_frame = orient * max_frm + frame;

    int frm_width = img_data->Frame[display_frame].frame_info->Frame_Width;
    int frm_height = img_data->Frame[display_frame].frame_info->Frame_Height;

    int x_offset = img_data->Frame[display_frame].bounding_box.x1 - img_data->FRM_bounding_box[orient].x1;
    int y_offset = img_data->Frame[display_frame].bounding_box.y1 - img_data->FRM_bounding_box[orient].y1;

    uint8_t* data = img_data->Frame[display_frame].frame_info->frame_start;

    //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
    //FRM's are aligned to 1-byte
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //bind data to FRM_texture for display
    uint8_t * blank = (uint8_t*)calloc(1, width*height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, blank);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, frm_width, frm_height, GL_RED, GL_UNSIGNED_BYTE, data);
    free(blank);
}

void animate_FRM_to_framebuff(float* palette, Shader* shader, mesh* triangle,
                              image_data* img_data, clock_t current_time, bool palette_update)
{
    float constexpr static playback_speeds[5] = { 0.0f, .25f, 0.5f, 1.0f, 2.0f };
    float fps = img_data->FRM_Info.FPS * playback_speeds[img_data->playback_speed];

    int orient      = img_data->display_orient_num;
    int width       = img_data->FRM_bounding_box[orient].x2 - img_data->FRM_bounding_box[orient].x1;
    int height      = img_data->FRM_bounding_box[orient].y2 - img_data->FRM_bounding_box[orient].y1;

    glViewport(0, 0, img_data->width, img_data->height);
    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

    static clock_t last_time = 0;
    //static clock_t accumulated_delta_time = current_time - last_time;
    //last_time = current_time;

    if ((fps != 0) && ((float)(current_time - last_time)/CLOCKS_PER_SEC > 1 / fps)) {
        last_time = current_time;

        img_data->display_frame_num += 1;
        if (img_data->display_frame_num >= img_data->FRM_Info.Frames_Per_Orient) {
            img_data->display_frame_num = 0;
        }
        render_FRM_OpenGL(img_data, width, height);
    }
    else if (palette_update) {
        render_FRM_OpenGL(img_data, width, height);
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