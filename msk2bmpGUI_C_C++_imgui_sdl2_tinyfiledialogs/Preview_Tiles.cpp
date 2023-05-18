#include "imgui-docking/imgui.h"
#include "imgui-docking/imgui_internal.h"

#include "Preview_Tiles.h"
#include "display_FRM_OpenGL.h"
#include "Zoom_Pan.h"

//Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define TILE_W      (350)
#define TILE_H      (300)
#define TILE_SIZE   (350*300)

void preview_tiles(variables* My_Variables, image_data* img_data)
{
    shader_info* shaders = &My_Variables->shaders;

    draw_FRM_to_framebuffer(shaders->palette,
                            shaders->render_FRM_shader,
                           &shaders->giant_triangle,
                            img_data);

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    float scale = img_data->scale;
    int img_width  = img_data->width;
    int img_height = img_data->height;
    ImVec2 size = ImVec2((float)(img_width * scale), (float)(img_height * scale));

    ImVec2 base_top_corner = top_corner(img_data);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with

    // Preview window for tiles already converted to palettized and dithered format
    ImVec2 Top_Left;// = Origin;
    ImVec2 Bottom_Right = { 0, 0 };
    int max_box_x = img_width  / TILE_W;
    int max_box_y = img_height / TILE_H;
    int pxl_border = 3;


    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {
            Top_Left.x = ((x * (float)TILE_W)) / img_width;
            Top_Left.y = ((y * (float)TILE_H)) / img_height;

            ImVec2 Bottom_Right = { (Top_Left.x + ((float)TILE_W / img_width)),
                                    (Top_Left.y + ((float)TILE_H / img_height)) };

            ImVec2 new_corner;
            new_corner.x = base_top_corner.x + (TILE_W + pxl_border) * x * scale;
            new_corner.y = base_top_corner.y + (TILE_H + pxl_border) * y * scale;

            ImVec2 new_bottom;
            new_bottom.x = new_corner.x + ( TILE_W * scale);
            new_bottom.y = new_corner.y + ( TILE_H * scale);

            window->DrawList->AddImage(
                (ImTextureID)img_data->render_texture,
                new_corner, new_bottom,
                Top_Left, Bottom_Right,
                ImGui::GetColorU32(My_Variables->tint_col));

        }
    }
}