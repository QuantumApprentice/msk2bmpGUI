#include "imgui.h"
#include "imgui_internal.h"

#include "Preview_Tiles.h"
#include "display_FRM_OpenGL.h"
#include "Zoom_Pan.h"

//Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define MTILE_W      (350)
#define MTILE_H      (300)
#define MTILE_SIZE   (350*300)

void tile_me(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data* img_data);
void draw_red_squares(image_data* img_data, bool show_squares);
void draw_red_tiles(image_data* img_data, bool show_squares);

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void Prev_WMAP_Tiles(variables* My_Variables, image_data* img_data)
{
    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info* shaders = &My_Variables->shaders;

    //draw_FRM_to_framebuffer(shaders->palette,
    //                        shaders->render_FRM_shader,
    //                       &shaders->giant_triangle,
    //                        img_data);

    if (img_data->FRM_dir) {
        if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL) {
            ImGui::Text("No Image Data");
            return;
        }
        else {
            animate_FRM_to_framebuff(shaders->palette,
                shaders->render_FRM_shader,
                shaders->giant_triangle,
                img_data,
                My_Variables->CurrentTime_ms,
                My_Variables->Palette_Update);
        }
    }
    else {
        ImGui::Text("No Image Data");
        return;
    }


    float scale = img_data->scale;
    int img_width  = img_data->width;
    int img_height = img_data->height;



    tile_me(MTILE_W, MTILE_H, img_width, img_height, scale, img_data);




}

void tile_me(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data* img_data)
{

    //ImVec2 size = ImVec2((float)(img_w * scale), (float)(img_h * scale));
    ImVec2 base_top_corner = top_corner(img_data);
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    // Preview window for tiles already converted to palettized and dithered format
    ImVec2 uv_min;// = Origin;
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

            uv_max = { (uv_min.x + ((float)tile_w / img_w)),
                       (uv_min.y + ((float)tile_h / img_h)) };

            ImVec2 new_corner;
            new_corner.x = base_top_corner.x + (tile_w + pxl_border) * x * scale;
            new_corner.y = base_top_corner.y + (tile_h + pxl_border) * y * scale;

            ImVec2 new_bottom;
            new_bottom.x = new_corner.x + (tile_w * scale);
            new_bottom.y = new_corner.y + (tile_h * scale);

            //image I'm trying to pan and zoom with
            window->DrawList->AddImage(
                (ImTextureID)img_data->render_texture,
                new_corner, new_bottom,
                uv_min, uv_max,
                ImGui::GetColorU32(tint_col));

            //window->DrawList->AddImage(
            //    (ImTextureID)img_data->render_texture,
            //    new_corner, bottom_corner(size, new_corner),
            //    Top_Left, Bottom_Right,
            //    ImGui::GetColorU32(tint_col));

        }
    }
}

void masking(image_data* img_data, GLuint msk_texture, ImVec2 TLC, shader_info* shaders, uint8_t* img_buff, int tile_x, int tile_y)
{
    int width = img_data->width;
    int height = img_data->height;
    int img_size = width * height;

    //copy edited texture to buffer, combine with original image

    //create a buffer
    uint8_t texture_buffer[80 * 36];    // = (uint8_t*)malloc(80 * 36);
    uint8_t msk_texture_buff[80 * 36];  // = (uint8_t*)malloc(80 * 36);


    static GLuint framebuffer;
    if (!glIsFramebuffer(framebuffer)) {
        glGenFramebuffers(1, &framebuffer);
    }
    glBindFramebuffer(GL_TEXTURE_2D, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->FRM_texture, 0);
    if ((TLC.x > -80) && (TLC.y > -36)) {
        glReadPixels(TLC.x, TLC.y, 80, 36, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);
    }
    else { return; }
    glBindTexture(GL_TEXTURE_2D, msk_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, msk_texture_buff);

    for (int i = 0; i < 80*36; i++)
    {
        if (msk_texture_buff[i] > 0) {
            msk_texture_buff[i] = 255;
        }
    }

    //for (int y = 0; y < 36; y++)
    //{
    //    for (int x = 0; x < 80; x++)
    //    {
    //        texture_buffer[(y * 80 + x)] = img_buff[(y * 80 + x) + (tile_x * 80 + tile_y*width*36)];
    //    }
    //}


    for (int i = 0; i < 80 * 36; i++)
    {
        if (!(texture_buffer[i] && msk_texture_buff[i])) {
            texture_buffer[i] = 00;
        }
        //texture_buffer[i] &= msk_texture_buff[i];
    }




    static GLuint tile_texture;
    if (!glIsTexture(tile_texture)) {
        glGenTextures(1, &tile_texture);
    }
    glBindTexture(GL_TEXTURE_2D, tile_texture);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 80, 36, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    if (glIsTexture(img_data->PAL_texture)) {
        glDeleteTextures(1, &img_data->PAL_texture);
    }
    glGenTextures(1, &img_data->PAL_texture);
    //texture settings
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 80, 36, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_TEXTURE_2D, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->PAL_texture, 0);




    draw_FRM_to_framebuffer(shaders, 80, 36, framebuffer, tile_texture);




    ;

}

void tile_t(image_data* img_data, shader_info* shaders, GLuint tile_texture, int img_w, int img_h, int tile_w, int tile_h)
{
    float scale = img_data->scale;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec2 base_top_corner = top_corner(img_data);

    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;


    ImVec2 Top_Left;// = Origin;
    ImVec2 Bottom_Right = { 0,0 };
    ImVec2 uv_min       = { 0,0 };
    ImVec2 uv_max       = { 1.0, 1.0 };
    //int pxl_border = 30;

    ImVec2 TLC, BRC;

    static float offset_x;
    static float offset_y;
    ImGui::SliderFloat("offset_x", &offset_x, -200, 200.0, NULL);
    ImGui::SliderFloat("offset_y", &offset_y, -200, 200.0, NULL);



    uint8_t* temp_buffer = (uint8_t*)malloc(img_data->width*img_data->height);
    //read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, temp_buffer);
    //glReadPixels(TLC.x, TLC.y, 200, 200, GL_RGB, GL_UNSIGNED_BYTE, texture_buffer);

    ImVec2 L_Corner = { 00, 00 };
    ImVec2 T_Corner = { 48,-12 };
    ImVec2 R_Corner = { 80, 12 };
    ImVec2 B_Corner = { 32, 24 };

    ImVec2 Left, Top, Bottom, Right, new_origin;

    static int offset1;
    static int offset2;
    static int offset3;
    static int offset4;
    ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
    ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
    ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    //Origin.x += offset3 * scale;




    int col_w = (80 + 48);        //128
    int row_h = (36 + 36 + 24);   // 96

    //max_box_x = MAX((img_w+47) / 48, (img_h+11) / 12);
    //max_box_y = MAX((img_w+31) / 32, (img_h+23) / 24);
    //max_box_y = ((img_w + col_w));

    int max_box_x;// = img_w / 32;// tile_w;
    int max_box_y;// = img_h / 24;// tile_h;
    int min_box_x;
    int min_box_y;

    max_box_x = +2 * ((img_w + (col_w - 1) - offset_x) / col_w);
    min_box_x = -3 * ((img_h + (row_h - 1) + offset_y) / row_h);
    max_box_y =  2 * ((img_w + (col_w - 1) - offset_y) / col_w) + 3 * ((img_h + (row_h - 1) - offset_y) / row_h);
    min_box_y = -3 * offset_y/row_h - 2 * offset_x/col_w;


    for (int y = min_box_y; y < max_box_y; y++)
    {
        //Origin.x += offset4 * scale;

        for (int x = min_box_x; x < max_box_x; x++)
        {
            new_origin.x = Origin.x + x * offset1 * scale;
            new_origin.y = Origin.y + y * offset2 * scale;

            Top_Left.x = new_origin.x + (x * 48 + y * 32) *scale;
            Top_Left.y = new_origin.y + (x *-12 + y * 24) *scale;

            Left.x   = Top_Left.x + L_Corner.x * scale;
            Left.y   = Top_Left.y + L_Corner.y * scale;

            Top.x    = Top_Left.x + T_Corner.x * scale;
            Top.y    = Top_Left.y + T_Corner.y * scale;

            Right.x  = Top_Left.x + R_Corner.x * scale;
            Right.y  = Top_Left.y + R_Corner.y * scale;

            Bottom.x = Top_Left.x + B_Corner.x * scale;
            Bottom.y = Top_Left.y + B_Corner.y * scale;

            ImVec2 uv_l, uv_t, uv_r, uv_b;
            uv_l.x = (Left.x   + offset_x - new_origin.x) / img_data->width  / scale;
            uv_l.y = (Left.y   + offset_y - new_origin.y) / img_data->height / scale;

            uv_t.x = (Top.x    + offset_x - new_origin.x) / img_data->width  / scale;
            uv_t.y = (Top.y    + offset_y - new_origin.y) / img_data->height / scale;

            uv_r.x = (Right.x  + offset_x - new_origin.x) / img_data->width  / scale;
            uv_r.y = (Right.y  + offset_y - new_origin.y) / img_data->height / scale;

            uv_b.x = (Bottom.x + offset_x - new_origin.x) / img_data->width  / scale;
            uv_b.y = (Bottom.y + offset_y - new_origin.y) / img_data->height / scale;

            glBindTexture(GL_TEXTURE_2D, img_data->render_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);



            if ( ((uv_l.x >= 0) && (uv_l.x <= 1.0) && (uv_l.y >= 0) && (uv_l.y <= 1.0))
               ||((uv_t.x >= 0) && (uv_t.x <= 1.0) && (uv_t.y >= 0) && (uv_t.y <= 1.0))
               ||((uv_r.x >= 0) && (uv_r.x <= 1.0) && (uv_r.y >= 0) && (uv_r.y <= 1.0))
               ||((uv_b.x >= 0) && (uv_b.x <= 1.0) && (uv_b.y >= 0) && (uv_b.y <= 1.0)) ) {

                window->DrawList->AddImageQuad(
                    (ImTextureID)img_data->render_texture,
                    Left, Top, Right, Bottom,
                    uv_l, uv_t, uv_r, uv_b);
            }

            //Top_Left.x = Origin.x + x * (pxl_border_x)*scale;
            //Top_Left.y = Origin.y + y * (pxl_border_y)*scale;
            //Bottom_Right = { (float)(Top_Left.x + tile_w * scale), (float)(Top_Left.y + tile_h * scale) };

            TLC.x = (float)(x * 48.0 + y * 32.0);
            TLC.y = (float)(x *-12.0 + y * 24.0);

            //uv_min.x = TLC.x/ img_w;
            //uv_min.y = TLC.y/ img_h;

            //uv_max.x = uv_min.x + ((float)tile_w / img_w);
            //uv_max.y = uv_min.y + ((float)tile_h / img_h);

            //masking(img_data, tile_texture, TLC, shaders, temp_buffer, x, y);




            //window->DrawList->AddImage(
            //    (ImTextureID)img_data->PAL_texture,
            //    Top_Left, Bottom_Right,
            //    ImVec2(0, 0), ImVec2(1.0, 1.0),
            //    ImGui::GetColorU32(tint_col));
        }
    }

    free(temp_buffer);

}

//#define TMAP_W          (80)
//#define TMAP_H          (36)
#define TMAP_W          (80 + 48)
#define TMAP_H          (36 + 24)

void Prev_TMAP_Tiles(variables* My_Variables, image_data* img_data)
{
    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info* shaders = &My_Variables->shaders;

    if (img_data->FRM_dir) {
        if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL) {
            ImGui::Text("No Image Data");
            return;
        }
        else {
            animate_FRM_to_framebuff(shaders->palette,
                shaders->render_FRM_shader,
                shaders->giant_triangle,
                img_data,
                My_Variables->CurrentTime_ms,
                My_Variables->Palette_Update);
        }
    }
    else {
        ImGui::Text("No Image Data");
        return;
    }

    ImVec2 uv_min = { 0,0 };
    ImVec2 uv_max = { 1.0, 1.0 };
    int width   = img_data->width;
    int height  = img_data->height;
    float scale = img_data->scale;

    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    static bool image_toggle = true;
    checkbox_handler("toggle image", &image_toggle);

    if (image_toggle) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        window->DrawList->AddImage(
            (ImTextureID)img_data->render_texture,
            top_corner(img_data), bottom_corner(size, top_corner(img_data)),
            uv_min, uv_max,
            ImGui::GetColorU32(My_Variables->tint_col));
    }

    tile_t(img_data, &My_Variables->shaders, My_Variables->tile_texture_rend, width, height, TMAP_W, TMAP_H);


}


void draw_red_squares(image_data* img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (show_squares) {
        ImDrawList *Draw_List = ImGui::GetWindowDrawList();
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        ImVec2 Top_Left;
        ImVec2 Bottom_Right = { 0, 0 };
        int max_box_x = img_data->width / 350;
        int max_box_y = img_data->height / 300;

        for (int j = 0; j < max_box_y; j++)
        {
            for (int i = 0; i < max_box_x; i++)
            {
                Top_Left.x = Origin.x + (i * 350)*scale;
                Top_Left.y = Origin.y + (j * 300)*scale;
                Bottom_Right = { (float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale) };
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
    }
}

void draw_quad(ImVec2 Image_Corner, ImVec2 Top_Left, float scale)
{
    ImDrawList *Draw_List = ImGui::GetWindowDrawList();
    ImVec2 Left, Top, Bottom, Right, new_origin;
    ImVec2 Bottom_Right = { 0, 0 };
    ImVec2 L_Offset = { 00, 00 };
    ImVec2 T_Offset = { 48,-12 };
    ImVec2 R_Offset = { 80, 12 };
    ImVec2 B_Offset = { 32, 24 };

    // Bottom_Right = { (float)(Top_Left.x + TMAP_W * scale), (float)(Top_Left.y + TMAP_H * scale) };
    //Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);

    Left.x   = Top_Left.x + L_Offset.x * scale;
    Left.y   = Top_Left.y + L_Offset.y * scale;

    Top.x    = Top_Left.x + T_Offset.x * scale;
    Top.y    = Top_Left.y + T_Offset.y * scale;

    Right.x  = Top_Left.x + R_Offset.x * scale;
    Right.y  = Top_Left.y + R_Offset.y * scale;

    Bottom.x = Top_Left.x + B_Offset.x * scale;
    Bottom.y = Top_Left.y + B_Offset.y * scale;

    if (Bottom.y < (Image_Corner.y-1)) {
        return;
    }

    //Bottom.y = Origin.y + (i*TMAP_W) - 36;
    Draw_List->AddQuad(Left, Bottom, Right, Top, 0xff0000ff, 1.0f);
}

void add_offsets(ImVec2* top, ImVec2* left, ImVec2* right, ImVec2* bottom)
{
    
}

void draw_red_tiles(image_data* img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (!show_squares) {
        return;
    }

    ImDrawList *Draw_List = ImGui::GetWindowDrawList();
    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

    ImVec2 Top_Left;
    ImVec2 Btm_Rght;
    ImVec2 L_Offset = { 00*scale, 00*scale };
    ImVec2 T_Offset = { 48*scale,-12*scale };
    ImVec2 R_Offset = { 80*scale, 12*scale };
    ImVec2 B_Offset = { 32*scale, 24*scale };

    int max_box_x = img_data->width  / TMAP_W;
    int max_box_y = img_data->height / TMAP_H;

    ImVec2 Left, Top, Bottom, Right, new_origin;

    static int offset1;
    static int offset2;
    static int offset3;
    static int offset4;
    ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
    ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
    ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    Origin.x += offset3 * scale;
    // offset1 = offset1*scale;
    // offset2 = offset2*scale;

    float h_offset =  48*scale + scale*offset1;
    float v_offset = -12*scale + scale*offset2;
    float row_h_offset = -16*scale;
    float row_v_offset =  36*scale;


    Top_Left.x = Origin.x-32*scale;
    Top_Left.y = Origin.y;

    // Left.x   = Top_Left.x + L_Offset.x;
    // Left.y   = Top_Left.y + L_Offset.y;
    // Top.x    = Top_Left.x + T_Offset.x;
    // Top.y    = Top_Left.y + T_Offset.y;
    // Right.x  = Top_Left.x + R_Offset.x;
    // Right.y  = Top_Left.y + R_Offset.y;
    // Bottom.x = Top_Left.x + B_Offset.x;
    // Bottom.y = Top_Left.y + B_Offset.y;

    ImVec2 Row_Left, Row_Top, Row_Right, Row_Bottom;
    Row_Left.x      = Top_Left.x + L_Offset.x;
    Row_Left.y      = Top_Left.y + L_Offset.y;
    Row_Top.x       = Top_Left.x + T_Offset.x;
    Row_Top.y       = Top_Left.y + T_Offset.y;
    Row_Bottom.x    = Top_Left.x + R_Offset.x;
    Row_Bottom.y    = Top_Left.y + R_Offset.y;
    Row_Right.x     = Top_Left.x + B_Offset.x;
    Row_Right.y     = Top_Left.y + B_Offset.y;

    int img_right  = Origin.x + img_data->width;
    int img_bottom = Origin.y + img_data->height;


    for (int i = 0; i < 50; i++)
    // int i = 0;
    // int j = 0;
    // while (Left.y <= img_bottom)
    {
        // int x = -16*scale*i;// + (-16) + offset4*scale;
        // int y =  36*scale*i     ;


        Left.x   = Row_Left.x    ;
        Left.y   = Row_Left.y    ;
        Top.x    = Row_Top.x     ;
        Top.y    = Row_Top.y     ;
        Right.x  = Row_Bottom.x  ;
        Right.y  = Row_Bottom.y  ;
        Bottom.x = Row_Right.x   ;
        Bottom.y = Row_Right.y   ;


        //when you switch to the next row, after doing the current addition,
        //you could "advance" the start of the row by refular tile offset 
        //(h_offset and v_offset) until it's inside the image, 
        //or past the end of it
        bool drew_a_tile = true;
        //to test it's working correctly, you could comment out the if
        //condition wrapping the draw quad
        //And you'd break if you couldn't "find" the start of the row 
        //(i.e. the left point is past the right edge of the image)

        for (int j = 0; j < 50; j++)
        {
            if ((Top.y < img_bottom) &&
                (Right.x > Origin.x)) {
                Draw_List->AddQuad(Left, Bottom, Right, Top, 0xff0000ff, 1.0f);
            }

            Left.x   += h_offset;
            Left.y   += v_offset;
            Top.x    += h_offset;
            Top.y    += v_offset;
            Right.x  += h_offset;
            Right.y  += v_offset;
            Bottom.x += h_offset;
            Bottom.y += v_offset;

            if (Bottom.y < (Origin.y)) {    //crop top
                break;
            }
            if (Left.x > (img_right)) {     //crop right
                break;
            }
        }

        Row_Left.x   += row_h_offset;
        Row_Left.y   += row_v_offset;
        Row_Top.x    += row_h_offset;
        Row_Top.y    += row_v_offset;
        Row_Bottom.x += row_h_offset;
        Row_Bottom.y += row_v_offset;
        Row_Right.x  += row_h_offset;
        Row_Right.y  += row_v_offset;
    }



    // for (int j = 0; j < 5; j++)
    // {
    //     new_origin.x = Origin.x + j*offset1 * scale -32*scale;
    //     new_origin.y = Origin.y + j*offset2 * scale;

    //     Top_Left.x = new_origin.x + (j*32) *scale;
    //     Top_Left.y = new_origin.y;// + (12)*scale + (-12 + 24) *scale;

    //     draw_quad(Top_Left, scale);
//-----------------------------------------------------------
    //     for (int i = 0; i < max_box_x*3; i++)
    //     {
    //         Top_Left.x = new_origin.x - (j * 48)*scale + (i* 48 + j*32) *scale;
    //         if (Top_Left.x < (Origin.x - TMAP_W)) {
    //             new_origin.x = Origin.x;
    //         }
    //         Top_Left.y = new_origin.y + (j * 12)*scale + (i*-12 + j*24) *scale;
    //         draw_quad(Top_Left, scale);
    //     }
    // }
}

void draw_tiles_OpenGL(image_data* img_data, shader_info* shader, GLuint* texture, bool draw_tiles)
{
    float scale = img_data->scale;
    if (draw_tiles) {
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        glViewport(0, 0, img_data->width, img_data->height);
        glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
        glBindVertexArray(shader->giant_triangle.VAO);
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 96, GL_RED, GL_UNSIGNED_BYTE, texture);

        glDrawArrays(GL_TRIANGLES, 0, shader->giant_triangle.vertexCount);

        //bind framebuffer back to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}


void draw_tiles_ImGui(image_data* img_data, ImVec2 Top_Corner, ImVec2 Bottom_Corner)
{
    //float scale = img_data->scale;
    //int tile_w  = img_data->width/128;
    //int tile_h  = img_data->height/96;

    //for (int x = 0; x < tile_w; x++)
    //{
    //    for (int y = 0; y < tile_h; y++)
    //    {

            //Top_Left.x = new_origin.x + (x * 48 + y * 32) *scale;
            //Top_Left.y = new_origin.y + (x *-12 + y * 24) *scale;

            //window->DrawList->AddImage(
            //    (ImTextureID)texture,
            //    new_corner, new_bottom,
            //    Top_Left, Bottom_Right,
            //    )



    //    }
    //}

}