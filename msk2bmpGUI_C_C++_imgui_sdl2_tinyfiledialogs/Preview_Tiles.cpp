#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_internal.h"

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
                My_Variables->CurrentTime,
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
    ImVec2 Top_Left;// = Origin;
    ImVec2 Bottom_Right = { 0, 0 };
    int max_box_x = img_w / tile_w;
    int max_box_y = img_h / tile_h;
    int pxl_border = 3;


    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {
            Top_Left.x = ((x * (float)tile_w)) / img_w;
            Top_Left.y = ((y * (float)tile_h)) / img_h;

            ImVec2 Bottom_Right = { (Top_Left.x + ((float)tile_w / img_w)),
                                    (Top_Left.y + ((float)tile_h / img_h)) };

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
                Top_Left, Bottom_Right,
                ImGui::GetColorU32(tint_col));

            //window->DrawList->AddImage(
            //    (ImTextureID)img_data->render_texture,
            //    new_corner, bottom_corner(size, new_corner),
            //    Top_Left, Bottom_Right,
            //    ImGui::GetColorU32(tint_col));

        }
    }
}

#define TMAP_W          (80)
#define TMAP_H          (36)

void Prev_TMAP_Tiles(variables* My_Variables, image_data* img_data)
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
                My_Variables->CurrentTime,
                My_Variables->Palette_Update);
        }
    }
    else {
        ImGui::Text("No Image Data");
        return;
    }


    float scale = img_data->scale;
    int img_width = img_data->width;
    int img_height = img_data->height;

    tile_me(TMAP_W, TMAP_H, img_width, img_height, scale, img_data);


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

        for (int i = 0; i < max_box_x; i++)
        {
            for (int j = 0; j < max_box_y; j++)
            {
                Top_Left.x = Origin.x + (i * 350)*scale;
                Top_Left.y = Origin.y + (j * 300)*scale;
                Bottom_Right = { (float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale) };
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
    }
}

#define TMAP_W          (80 + 48)
#define TMAP_H          (36 + 24)
void draw_red_tiles(image_data* img_data, bool show_squares)
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
        int max_box_x = img_data->width / TMAP_W;
        int max_box_y = img_data->height / TMAP_H;

        ImVec2 L_Offset = { 00, 00 };
        ImVec2 T_Offset = { 48, -12 };
        ImVec2 R_Offset = { 80, 12 };
        ImVec2 B_Offset = { 32, 24 };

        ImVec2 Left, Top, Bottom, Right, new_origin;

        static int offset1;
        static int offset2;
        static int offset3;
        ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
        ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
        ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);

        Origin.x += offset3 * scale;

        for (int j = 0; j < max_box_y * 3; j++)
        {
            new_origin.x = Origin.x + j*offset1 * scale;
            new_origin.y = Origin.y + j*offset2 * scale;

            for (int i = 0; i < max_box_x * 3+1; i++)
            {


                Top_Left.x = new_origin.x + /*(i * TMAP_W)*scale +*/ (i* 48 + j*32) *scale;
                Top_Left.y = new_origin.y + /*(j * TMAP_H)*scale +*/ (i*-12 + j*24) *scale;
                Bottom_Right = { (float)(Top_Left.x + TMAP_W * scale), (float)(Top_Left.y + TMAP_H * scale) };
                //Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);

                Left.x   = Top_Left.x + L_Offset.x * scale;
                Left.y   = Top_Left.y + L_Offset.y * scale;

                Top.x    = Top_Left.x + T_Offset.x * scale;
                Top.y    = Top_Left.y + T_Offset.y * scale;

                Right.x  = Top_Left.x + R_Offset.x * scale;
                Right.y  = Top_Left.y + R_Offset.y * scale;

                Bottom.x = Top_Left.x + B_Offset.x * scale;
                Bottom.y = Top_Left.y + B_Offset.y * scale;

                //Bottom.y = Origin.y + (i*TMAP_W) - 36;
                Draw_List->AddQuad(Left, Bottom, Right, Top, 0xff0000ff, 1.0f);
            }
        }
    }
}