#include "display_FRM_OpenGL.h"
#include "B_Endian.h"
#include "Load_Files.h"

#define ms_PER_sec                    (1000)

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

//TODO: this is still being used for regular images, refactor
void render_OTHER_OpenGL(image_data* img_data, int width, int height)
{
    int orient = img_data->display_orient_num; //(img_data->ANIM_hdr->Frame_0_Offset[1] > 0) ? img_data->display_orient_num : 0;
    int frame_num = img_data->display_frame_num;
    int max_frm = img_data->ANM_dir[orient].num_frames;
    ANM_Dir* anm_dir = img_data->ANM_dir;

    int x_offset = anm_dir[orient].frame_box->x1 - img_data->ANM_bounding_box[orient].x1;
    int y_offset = anm_dir[orient].frame_box->y1 - img_data->ANM_bounding_box[orient].y1;

    uint8_t* pxls;
    if (anm_dir[orient].frame_data[frame_num] == NULL) {
        width  = 0;
        height = 0;
        pxls = NULL;
    } else {
        pxls = anm_dir[orient].frame_data[frame_num]->pxls;
    }
    //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
    //FRM's are aligned to 1-byte, SDL_Surfaces are typically 4-byte
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    //bind data to FRM_texture for display
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pxls);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

//TODO: this is still being used, maybe refactor?
void animate_OTHER_to_framebuff(Shader* shader, mesh* triangle, image_data* img_data, uint64_t current_time)
{
    float constexpr static playback_speeds[5] = { 0.0f, .25f, 0.5f, 1.0f, 2.0f };

    float fps = 10 * playback_speeds[img_data->playback_speed];

    int dir = img_data->display_orient_num;
    int num = img_data->display_frame_num;
    ANM_Dir* anm_dir = img_data->ANM_dir;

    int img_width  = 0;
    int img_height = 0;
    //TODO: probably want to make the image display
    //      more flexible, manage the case where
    //      frame_data[num] doesn't have a surface
    //      but we can add one with a button if we
    //      want, also can add a "num" entry at the
    //      end (or possibly in between frames?)
    if (anm_dir[dir].frame_data[num]) {
        img_width  = img_data->ANM_dir[dir].frame_data[num]->w;
        img_height = img_data->ANM_dir[dir].frame_data[num]->h;
    } else {
        //TODO: for now this assigns frame[0]'s w/h
        //      to force glViewport() to be full image size
        //      for clearing, in case current frame is NULL
        img_width  = img_data->ANM_dir[dir].frame_data[0]->w;
        img_height = img_data->ANM_dir[dir].frame_data[0]->h;
    }

    glViewport(0, 0, img_width, img_height);
    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

    static uint64_t last_time = 0;
    static int last_frame;
    static int last_orient;

    if ((fps != 0) && ((float)(current_time - last_time) / ms_PER_sec > 1 / fps)) {
        last_time = current_time;

        img_data->display_frame_num += 1;
        if (img_data->display_frame_num >= img_data->ANM_dir[dir].num_frames) {
            img_data->display_frame_num = 0;
        }
        render_OTHER_OpenGL(img_data, img_width, img_height);
    }
    else if ((last_frame != img_data->display_frame_num) || (last_orient != img_data->display_orient_num)) {
        last_frame  = img_data->display_frame_num;
        last_orient = img_data->display_orient_num;
        render_OTHER_OpenGL(img_data, img_width, img_height);
    }

    //shader
    shader->use();
    //draw image to framebuffer
    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SURFACE_to_texture(Surface* src, GLuint texture,
                        int width, int height, int alignment)
{
    if (!src) {
        return;
    }

    // //print surface out as ascii hex
    // for (int i = 0; i < src->h; i++) {
    //     for (int j = 0; j < src->w; j++) {
    //         printf("%02x", src->pxls[i*src->pitch + j]);
    //     }
    //     printf("\n");
    // }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
    //MSK & FRM are aligned to 1-byte
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

    int pxl_type = GL_RED;
    if (alignment == 3) {
        pxl_type = GL_RGB;
    }
    else if (alignment == 4) {
        pxl_type = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, pxl_type, GL_UNSIGNED_BYTE, src->pxls);

}

void PAL_SURFACE_to_sub_texture(uint8_t* pxls, GLuint texture,
                        int x_offset, int y_offset,
                        int frm_width, int frm_height,
                        int total_width, int total_height)
{
    GLuint alignment = 1;
    int pxl_type = GL_RED;
    // if (type == OTHER) {
    //     alignment = 4;
    //     pxl_type = GL_RGBA;
    // }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
    //FRM/MSK are aligned to 1-byte
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    //bind blank background to FRM_texture for display, then paint data onto texture
    uint8_t * blank = (uint8_t*)calloc(1, total_width*total_height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, total_width, total_height, 0, pxl_type, GL_UNSIGNED_BYTE, blank);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, frm_width, frm_height, GL_RED, GL_UNSIGNED_BYTE, pxls);
    free(blank);
}

//TODO: handle the current_time break outside this code
//      to prevent this from being called when not directly
//      changing either a frame or a palette color-cycle
void animate_SURFACE_to_sub_texture(
    image_data* img_data, Surface* edit_srfc,
    uint64_t current_time)
{
    if (!edit_srfc) {
        return;
    }
    //TODO: maybe handle single image FRM's slightly differently with dropdown?
    //int orient = (img_data->FRM_hdr->Frame_0_Offset[1] > 0) ? img_data->display_orient_num : 0;
    int frame_num = img_data->display_frame_num;
    int dir     = img_data->display_orient_num;

    int total_w = img_data->ANM_bounding_box[dir].x2 - img_data->ANM_bounding_box[dir].x1;
    int total_h = img_data->ANM_bounding_box[dir].y2 - img_data->ANM_bounding_box[dir].y1;

    // int frame_w = img_data->ANM_dir[dir].frame_data[frame_num]->w;
    // int frame_h = img_data->ANM_dir[dir].frame_data[frame_num]->h;
    int frame_w = edit_srfc->w;
    int frame_h = edit_srfc->h;

    int x_offset = img_data->ANM_dir[dir].frame_box[frame_num].x1 - img_data->ANM_bounding_box[dir].x1;
    int y_offset = img_data->ANM_dir[dir].frame_box[frame_num].y1 - img_data->ANM_bounding_box[dir].y1;

    float constexpr static playback_speeds[5] = { 0.0f, .25f, 0.5f, 1.0f, 2.0f };
    int FRM_fps = (img_data->FRM_hdr->FPS == 0 && img_data->ANM_dir[dir].num_frames > 1) ? 10 : img_data->FRM_hdr->FPS;
    float fps   = FRM_fps * playback_speeds[img_data->playback_speed];

    static uint64_t last_time = 0;
    if ((fps != 0) && ((float)(current_time - last_time)/ms_PER_sec > 1 / fps)) {
        last_time = current_time;

        img_data->display_frame_num     += 1;
        if (img_data->display_frame_num >= img_data->FRM_hdr->Frames_Per_Orient) {
            img_data->display_frame_num  = 0;
        }
    }
    PAL_SURFACE_to_sub_texture(edit_srfc->pxls, img_data->FRM_texture,
                        x_offset, y_offset, frame_w, frame_h,
                        total_w, total_h);
}

//TODO: delete? replaced by draw_PAL_to_framebuffer()?
void draw_FRM_to_framebuffer(shader_info* shader_i, int width, int height,
                             GLuint framebuffer, GLuint texture)
{
    glViewport(0, 0, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindVertexArray(shader_i->giant_triangle.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    //shader
    shader_i->render_FRM_shader->use();
    uint32_t ID = shader_i->render_FRM_shader->ID;

    GLint t = glGetUniformLocation(shader_i->render_FRM_shader->ID, "ColorPaletteUINT");
    glUniform1uiv(t, 256, (GLuint*)shader_i->FO_pal->colors);

    shader_i->render_FRM_shader->setInt("Indexed_FRM", 0);

    int err = glGetError();
    if (err) {
        printf("draw_FRM_to_framebuffer() glGetError: %d\n", err);
    }
    glDrawArrays(GL_TRIANGLES, 0, shader_i->giant_triangle.vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//TODO: rename this? draw_TEXTURES_to_Framebuffer()?
//      it used to work just for PAL images
//      but now it just takes 3 textures
//      and combines them into one in the framebuffer
//      while applying the palette cycle
//TODO: maybe restructure this to take an array of textures?
//      textures ordered by index?
//      passed in framebuffer to draw to?
void draw_PAL_to_framebuffer(Palette* pal, Shader* shader,
    mesh* triangle, struct image_data* img_data)
{
    glViewport(0, 0, img_data->width, img_data->height);

    //bind framebuffer to draw to
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
    uint32_t ID = shader->ID;
    glUniform1uiv(glGetUniformLocation(ID, "ColorPaletteUINT"), 256, (GLuint*)pal->colors);

    shader->setInt("Indexed_FRM", 0);
    shader->setInt("Indexed_PAL", 1);
    shader->setInt("Indexed_MSK", 2);

    int err = glGetError();
    if (err) {
        printf("draw_PAL_to_framebuffer() glGetError: %d\n", err);
    }


    //draw to framebuffer
    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//TODO: this should be replaced by draw_PAL_to_framebuffer()?
void draw_texture_to_framebuffer(Palette* pal, Shader* shader, mesh* triangle,
    GLuint framebuffer, GLuint texture, int w, int h)
{
    glViewport(0, 0, w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    shader->use();


    GLint t = glGetUniformLocation(shader->ID, "ColorPaletteUINT");
    glUniform1uiv(t, 256, (GLuint*)pal->colors);

    GLenum err = glGetError();
    if (err) {
        printf("draw_texture_to_framebuffer() glGetError: %d\n", err);
        // GL_INVALID_OPERATION
    }

    shader->setInt("Indexed_FRM", 0);

    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//TODO: delete? this should be replaced by draw_PAL_to_framebuffer()
void draw_MSK_to_framebuffer(Palette* pal, Shader* shader, mesh* triangle,
                             struct image_data* img_data)
{
    glViewport(0, 0, img_data->width, img_data->height);

    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
    glBindVertexArray(triangle->VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);

    //shader
    shader->use();
    glUniform1uiv(glGetUniformLocation(shader->ID, "ColorPaletteUINT"), 256, (GLuint*)pal->colors);
    shader->setInt("Indexed_FRM", 0);

    glDrawArrays(GL_TRIANGLES, 0, triangle->vertexCount);

    //bind framebuffer back to default
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLenum err = glGetError();
    if (err) {
        printf("draw_MSK_to_framebuffer() glGetError: %d\n", err);
        // GL_INVALID_OPERATION
    }
}