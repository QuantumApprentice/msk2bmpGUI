#include "imgui.h"
#include "imgui_internal.h"

#include "Preview_Tiles.h"
#include "Save_Files.h"
#include "display_FRM_OpenGL.h"
#include "Zoom_Pan.h"

#include "load_FRM_OpenGL.h"
// #include "B_Endian.h"

// Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define MTILE_W (350)
#define MTILE_H (300)
#define MTILE_SIZE (350 * 300)

void tile_me(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data *img_data);
void draw_red_squares(image_data *img_data, bool show_squares);
void draw_red_tiles(image_data *img_data, bool show_squares);

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void Prev_WMAP_Tiles(variables *My_Variables, image_data *img_data)
{
    // handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info *shaders = &My_Variables->shaders;

    // draw_FRM_to_framebuffer(shaders->palette,
    //                         shaders->render_FRM_shader,
    //                        &shaders->giant_triangle,
    //                         img_data);

    if (img_data->FRM_dir)
    {
        if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL)
        {
            ImGui::Text("No Image Data");
            return;
        }
        else
        {
            animate_FRM_to_framebuff(shaders->palette,
                                     shaders->render_FRM_shader,
                                     shaders->giant_triangle,
                                     img_data,
                                     My_Variables->CurrentTime_ms,
                                     My_Variables->Palette_Update);
        }
    }
    else
    {
        ImGui::Text("No Image Data");
        return;
    }

    float scale = img_data->scale;
    int img_width = img_data->width;
    int img_height = img_data->height;

    tile_me(MTILE_W, MTILE_H, img_width, img_height, scale, img_data);
}

void tile_me(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data *img_data)
{

    // ImVec2 size = ImVec2((float)(img_w * scale), (float)(img_h * scale));
    ImVec2 base_top_corner = top_corner(img_data);
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    // Preview window for tiles already converted to palettized and dithered format
    ImVec2 uv_min; // = Origin;
    ImVec2 uv_max;
    int max_box_x = img_w / tile_w;
    int max_box_y = img_h / tile_h;
    int pxl_border = 3;

    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {

            uv_min.x = ((x * (float)tile_w)) / img_w;
            uv_min.y = ((y * (float)tile_h)) / img_h;

            uv_max = {(uv_min.x + ((float)tile_w / img_w)),
                      (uv_min.y + ((float)tile_h / img_h))};

            ImVec2 new_corner;
            new_corner.x = base_top_corner.x + (tile_w + pxl_border) * x * scale;
            new_corner.y = base_top_corner.y + (tile_h + pxl_border) * y * scale;

            ImVec2 new_bottom;
            new_bottom.x = new_corner.x + (tile_w * scale);
            new_bottom.y = new_corner.y + (tile_h * scale);

            // image I'm trying to pan and zoom with
            window->DrawList->AddImage(
                (ImTextureID)img_data->render_texture,
                new_corner, new_bottom,
                uv_min, uv_max,
                ImGui::GetColorU32(tint_col));

            // window->DrawList->AddImage(
            //     (ImTextureID)img_data->render_texture,
            //     new_corner, bottom_corner(size, new_corner),
            //     Top_Left, Bottom_Right,
            //     ImGui::GetColorU32(tint_col));
        }
    }
}

void masking(image_data *img_data, GLuint msk_texture, ImVec2 TLC, shader_info *shaders, uint8_t *img_buff, int tile_x, int tile_y)
{
    int width = img_data->width;
    int height = img_data->height;
    int img_size = width * height;

    // copy edited texture to buffer, combine with original image

    // create a buffer
    uint8_t texture_buffer[80 * 36];   // = (uint8_t*)malloc(80 * 36);
    uint8_t msk_texture_buff[80 * 36]; // = (uint8_t*)malloc(80 * 36);

    static GLuint framebuffer;
    if (!glIsFramebuffer(framebuffer))
    {
        glGenFramebuffers(1, &framebuffer);
    }
    glBindFramebuffer(GL_TEXTURE_2D, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->FRM_texture, 0);
    if ((TLC.x > -80) && (TLC.y > -36))
    {
        glReadPixels(TLC.x, TLC.y, 80, 36, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);
    }
    else
    {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, msk_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, msk_texture_buff);

    for (int i = 0; i < 80 * 36; i++)
    {
        if (msk_texture_buff[i] > 0)
        {
            msk_texture_buff[i] = 255;
        }
    }

    // for (int y = 0; y < 36; y++)
    //{
    //     for (int x = 0; x < 80; x++)
    //     {
    //         texture_buffer[(y * 80 + x)] = img_buff[(y * 80 + x) + (tile_x * 80 + tile_y*width*36)];
    //     }
    // }

    for (int i = 0; i < 80 * 36; i++)
    {
        if (!(texture_buffer[i] && msk_texture_buff[i]))
        {
            texture_buffer[i] = 00;
        }
        // texture_buffer[i] &= msk_texture_buff[i];
    }

    static GLuint tile_texture;
    if (!glIsTexture(tile_texture))
    {
        glGenTextures(1, &tile_texture);
    }
    glBindTexture(GL_TEXTURE_2D, tile_texture);
    // texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 80, 36, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    if (glIsTexture(img_data->PAL_texture))
    {
        glDeleteTextures(1, &img_data->PAL_texture);
    }
    glGenTextures(1, &img_data->PAL_texture);
    // texture settings
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 80, 36, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_TEXTURE_2D, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->PAL_texture, 0);

    draw_FRM_to_framebuffer(shaders, 80, 36, framebuffer, tile_texture);
}




ImVec2 L_Corner = {00, 00};
ImVec2 T_Corner = {48,-12};
ImVec2 R_Corner = {80, 12};
ImVec2 B_Corner = {32, 24};
void draw_TMAP_tiles(user_info* usr_nfo, image_data *img_data,
                     shader_info *shaders, GLuint tile_texture)
{
    float scale = img_data->scale;
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    // ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    // ImVec2 base_top_corner = top_corner(img_data);

    int img_w = img_data->width;
    int img_h = img_data->height;

    ImVec2 Top_Left; // = Origin;
    // ImVec2 Bottom_Right = {  0,   0};
    // ImVec2 uv_min =       {  0,   0};
    // ImVec2 uv_max =       {1.0, 1.0};
    static int offset_x;
    static int offset_y;

    //Save tiles button
    if (ImGui::Button("Export Tiles")) {
        char* new_tiles = export_TMAP_tiles(usr_nfo, usr_nfo->exe_directory, img_data, offset_x, offset_y);
    }

    ImGui::SliderInt("Tile Offset X", &offset_x, -400, 400, NULL);
    ImGui::SliderInt("Tile Offset Y", &offset_y, -400, 400, NULL);

    // read pixels into buffer
    uint8_t *temp_buffer = (uint8_t *)malloc(img_data->width * img_data->height);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, temp_buffer);

    static int spacing_x;
    static int spacing_y;
    // static int offset3;
    // static int offset4;
    ImGui::SliderInt("Tile Spacing X", &spacing_x, 0, 80, NULL);
    ImGui::SliderInt("Tile Spacing Y", &spacing_y, 0, 80, NULL);
    // ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    // ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

    int col_w = (80 + 48);      // 128
    int row_h = (36 + 36 + 24); // 96
    int max_box_x = +2 * ((img_w + (col_w - 1) - offset_x) / col_w);
    int min_box_x = -3 * ((img_h + (row_h - 1) + offset_y) / row_h);
    int max_box_y = +2 * ((img_w + (col_w - 1) - offset_y) / col_w) + 3 * ((img_h + (row_h - 1) - offset_y) / row_h);
    int min_box_y = -3 * offset_y / row_h - 2  * offset_x  / col_w;

    ImVec2 Left, Top, Bottom, Right, new_origin;
    for (int y = min_box_y; y < max_box_y; y++)
    {
        for (int x = min_box_x; x < max_box_x; x++)
        {
            new_origin.x = Origin.x + x * spacing_x * scale;
            new_origin.y = Origin.y + y * spacing_y * scale;

            Top_Left.x = new_origin.x + (x *  48 + y * 32) * scale;
            Top_Left.y = new_origin.y + (x * -12 + y * 24) * scale;

            Left.x = Top_Left.x + L_Corner.x * scale;
            Left.y = Top_Left.y + L_Corner.y * scale;

            Top.x = Top_Left.x + T_Corner.x * scale;
            Top.y = Top_Left.y + T_Corner.y * scale;

            Right.x = Top_Left.x + R_Corner.x * scale;
            Right.y = Top_Left.y + R_Corner.y * scale;

            Bottom.x = Top_Left.x + B_Corner.x * scale;
            Bottom.y = Top_Left.y + B_Corner.y * scale;

            ImVec2 uv_l, uv_t, uv_r, uv_b;
            uv_l.x = (Left.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_l.y = (Left.y + offset_y - new_origin.y) / img_data->height / scale;

            uv_t.x = (Top.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_t.y = (Top.y + offset_y - new_origin.y) / img_data->height / scale;

            uv_r.x = (Right.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_r.y = (Right.y + offset_y - new_origin.y) / img_data->height / scale;

            uv_b.x = (Bottom.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_b.y = (Bottom.y + offset_y - new_origin.y) / img_data->height / scale;

            glBindTexture(GL_TEXTURE_2D, img_data->render_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            if (((uv_l.x >= 0) && (uv_l.x <= 1.0) && (uv_l.y >= 0) && (uv_l.y <= 1.0)) || ((uv_t.x >= 0) && (uv_t.x <= 1.0) && (uv_t.y >= 0) && (uv_t.y <= 1.0)) || ((uv_r.x >= 0) && (uv_r.x <= 1.0) && (uv_r.y >= 0) && (uv_r.y <= 1.0)) || ((uv_b.x >= 0) && (uv_b.x <= 1.0) && (uv_b.y >= 0) && (uv_b.y <= 1.0)))
            {

                window->DrawList->AddImageQuad(
                    (ImTextureID)img_data->render_texture,
                    Left, Top, Right, Bottom,
                    uv_l, uv_t, uv_r, uv_b);

                // ImGui::ShowMetricsWindow();

                // printf("position: %d,%d\n", Left.x, Left.y);
            }

        }
    }

    free(temp_buffer);
}

// #define TMAP_W          (80)
// #define TMAP_H          (36)
#define TMAP_W (80 + 48)
#define TMAP_H (36 + 24)
void Prev_TMAP_Tiles(user_info* usr_info, variables *My_Variables, image_data *img_data)
{
    // handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);
    shader_info *shaders = &My_Variables->shaders;
    ImGui::PushItemWidth(100);
    ImGui::DragFloat("##Zoom", &img_data->scale, 0.1f, 0.0f, 10.0f, "Zoom: %%%.2fx", 0);
    ImGui::PopItemWidth();

    if (!img_data->FRM_dir)
    {
        ImGui::Text("No Image Data");
        return;
    }
    if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL)
    {
        ImGui::Text("No Image Data");
        return;
    }


    animate_FRM_to_framebuff(shaders->palette,
                        shaders->render_FRM_shader,
                        shaders->giant_triangle,
                        img_data,
                        My_Variables->CurrentTime_ms,
                        My_Variables->Palette_Update);

    static bool image_toggle = false;
    checkbox_handler("toggle image", &image_toggle);
    if (image_toggle)
    {
        ImVec2 uv_min = {0, 0};
        ImVec2 uv_max = {1.0, 1.0};
        int width   = img_data->width;
        int height  = img_data->height;
        float scale = img_data->scale;
        ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

        ImGuiWindow *window = ImGui::GetCurrentWindow();
        window->DrawList->AddImage(
            (ImTextureID)img_data->render_texture,
            top_corner(img_data), bottom_corner(size, top_corner(img_data)),
            uv_min, uv_max,
            ImGui::GetColorU32(My_Variables->tint_col));
    }

    draw_TMAP_tiles(usr_info, img_data, shaders,
                    My_Variables->tile_texture_rend);
}

void draw_red_squares(image_data *img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (show_squares)
    {
        ImDrawList *Draw_List = ImGui::GetWindowDrawList();
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        ImVec2 Top_Left;
        ImVec2 Bottom_Right = {0, 0};
        int max_box_x = img_data->width  / 350;
        int max_box_y = img_data->height / 300;

        for (int j = 0; j < max_box_y; j++)
        {
            for (int i = 0; i < max_box_x; i++)
            {
                Top_Left.x = Origin.x + (i * 350) * scale;
                Top_Left.y = Origin.y + (j * 300) * scale;
                Bottom_Right = {(float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale)};
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
    }
}

// //this function not used right now
// void draw_quad(ImVec2 Image_Corner, ImVec2 Top_Left, float scale)
// {
//     ImDrawList *Draw_List = ImGui::GetWindowDrawList();
//     ImVec2 Left, Top, Bottom, Right, new_origin;
//     ImVec2 Bottom_Right = { 0, 0 };
//     ImVec2 L_Offset = { 00, 00 };
//     ImVec2 T_Offset = { 48,-12 };
//     ImVec2 R_Offset = { 80, 12 };
//     ImVec2 B_Offset = { 32, 24 };
//     // Bottom_Right = { (float)(Top_Left.x + TMAP_W * scale), (float)(Top_Left.y + TMAP_H * scale) };
//     //Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
//     Left.x   = Top_Left.x + L_Offset.x * scale;
//     Left.y   = Top_Left.y + L_Offset.y * scale;
//     Top.x    = Top_Left.x + T_Offset.x * scale;
//     Top.y    = Top_Left.y + T_Offset.y * scale;
//     Right.x  = Top_Left.x + R_Offset.x * scale;
//     Right.y  = Top_Left.y + R_Offset.y * scale;
//     Bottom.x = Top_Left.x + B_Offset.x * scale;
//     Bottom.y = Top_Left.y + B_Offset.y * scale;
//     if (Bottom.y < (Image_Corner.y-1)) {
//         return;
//     }
//     //Bottom.y = Origin.y + (i*TMAP_W) - 36;
//     Draw_List->AddQuad(Left, Bottom, Right, Top, 0xff0000ff, 1.0f);
// }

// struct to hold the 4 points for the quadrilateral (tile shape or image shape)
struct outline
{
    ImVec2 Top,
        Rgt,
        Btm,
        Lft;
};

// add offset to each point of square
void add_offset(ImVec2 offset, outline *square)
{
    square->Top.x += offset.x;
    square->Top.y += offset.y;
    square->Rgt.x += offset.x;
    square->Rgt.y += offset.y;
    square->Btm.x += offset.x;
    square->Btm.y += offset.y;
    square->Lft.x += offset.x;
    square->Lft.y += offset.y;
}

// draw tiles for the preview screen when checking the tiles box
void draw_red_tiles(image_data *img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (!show_squares)
    {
        return;
    }

    ImDrawList *Draw_List = ImGui::GetWindowDrawList();
    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

    ImVec2 Top_Left;
    outline tile_offsets;

    tile_offsets.Top = {48 * scale, -12 * scale};
    tile_offsets.Rgt = {80 * scale,  12 * scale};
    tile_offsets.Btm = {32 * scale,  24 * scale};
    tile_offsets.Lft = {00 * scale,  00 * scale};

    int max_box_x = img_data->width / TMAP_W;
    int max_box_y = img_data->height / TMAP_H;

    static int offset1;
    static int offset2;
    static int offset3;
    static int offset4;
    ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
    ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
    ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    Origin.x += offset3 * scale;

    ImVec2 offset =     { 48 * scale + scale * offset1,
                         -12 * scale + scale * offset2};
    ImVec2 row_offset = {-16 * scale + scale * offset4,
                          36 * scale + scale * offset4};

    Top_Left.x = Origin.x - 32 * scale;
    Top_Left.y = Origin.y;

    outline row_start;                 // used when incrementing rows
    outline new_square;                // stores tile position for each tile
    new_square = tile_offsets;         // copy default dimensions
    add_offset(Top_Left, &new_square); // move first tile position to image corner
    row_start = new_square;            // copy this position for re-use

    int img_right = Origin.x + img_data->width * scale;
    int img_bottom = Origin.y + img_data->height * scale;
    bool drew_row = true;
    int count = 0;

    while (true)
    {
        new_square = row_start;

        // when you switch to the next row, after doing the current addition,
        // you could "advance" the start of the row by regular tile offset
        //(h_offset and v_offset) until it's inside the image,
        // or past the end of it
        // to test it's working correctly, you could comment out the if
        // condition wrapping the draw quad
        // And you'd break if you couldn't "find" the start of the row
        //(i.e. the left point is past the right edge of the image)

        if (!drew_row)
        {
            printf("count: %d\n", count);
            break;
        }
        drew_row = false;

        while (true)
        {
            if ((new_square.Top.y < img_bottom) && // crop bottom
                (new_square.Rgt.x > Origin.x))
            { // crop left
                Draw_List->AddQuad(new_square.Lft,
                                   new_square.Btm,
                                   new_square.Rgt,
                                   new_square.Top, 0xff0000ff, 1.0f);
                drew_row = true;
                count++;
            }

            add_offset(offset, &new_square);

            if (new_square.Btm.y < (Origin.y))
            { // crop top
                break;
            }
            if (new_square.Lft.x > (img_right))
            { // crop right
                break;
            }
        }

        add_offset(row_offset, &row_start);
    }
}

void draw_tiles_OpenGL(image_data *img_data, shader_info *shader, GLuint *texture, bool draw_tiles)
{
    float scale = img_data->scale;

    printf("draw_tiles_OpenGL is being called...\n");

    if (draw_tiles)
    {
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        glViewport(0, 0, img_data->width, img_data->height);
        glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
        glBindVertexArray(shader->giant_triangle.VAO);
        glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 96, GL_RED, GL_UNSIGNED_BYTE, texture);

        glDrawArrays(GL_TRIANGLES, 0, shader->giant_triangle.vertexCount);

        // bind framebuffer back to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void draw_tiles_ImGui(image_data *img_data, ImVec2 Top_Corner, ImVec2 Bottom_Corner)
{
    // float scale = img_data->scale;
    // int tile_w  = img_data->width/128;
    // int tile_h  = img_data->height/96;
    // for (int x = 0; x < tile_w; x++)
    //{
    //     for (int y = 0; y < tile_h; y++)
    //     {
    // Top_Left.x = new_origin.x + (x * 48 + y * 32) *scale;
    // Top_Left.y = new_origin.y + (x *-12 + y * 24) *scale;
    // window->DrawList->AddImage(
    //     (ImTextureID)texture,
    //     new_corner, new_bottom,
    //     Top_Left, Bottom_Right,
    //     )
    //    }
    //}
}