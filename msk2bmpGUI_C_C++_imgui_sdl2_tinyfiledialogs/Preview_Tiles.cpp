#include "Preview_Tiles.h"
#include "imgui-docking/imgui.h"
#include "display_FRM_OpenGL.h"

//Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define TILE_W      (350)
#define TILE_H      (300)
#define TILE_SIZE   (350*300)

void preview_tiles(variables* My_Variables, image_data* img_data, int counter)
{
    shader_info* shaders = &My_Variables->shaders;

    draw_FRM_to_framebuffer(shaders->palette,
                           &shaders->render_FRM_shader,
                           &shaders->giant_triangle,
                            img_data);

    //ImVec2 Origin;
    //Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    //Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;
    // Preview window for tiles already converted to palettized and dithered format
    ImVec2 Top_Left;// = Origin;
    ImVec2 Bottom_Right = { 0, 0 };
    int max_box_x = img_data->width  / TILE_W;
    int max_box_y = img_data->height / TILE_H;

    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {
            Top_Left.x = ((x * (float)TILE_W)) / img_data->width;
            Top_Left.y = ((y * (float)TILE_H)) / img_data->height;

            ImVec2 Bottom_Right = { (Top_Left.x + ((float)TILE_W / img_data->width)),
                                    (Top_Left.y + ((float)TILE_H / img_data->height)) };
            //Bottom_Right = { (float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale) };

            ImGui::Image((ImTextureID)
                img_data->render_texture,
                ImVec2(TILE_W, TILE_H),
                Top_Left,
                Bottom_Right,
                My_Variables->tint_col,
                My_Variables->border_col);

            ImGui::SameLine();
        }
        ImGui::NewLine();
    }
}