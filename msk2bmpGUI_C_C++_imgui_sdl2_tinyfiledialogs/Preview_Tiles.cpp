#include "Preview_Tiles.h"
#include "imgui-docking/imgui.h"
#include "display_FRM_OpenGL.h"

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
    int max_box_x = img_data->width  / 350;
    int max_box_y = img_data->height / 300;

    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {
            Top_Left.x = ((x * 350.0f)) / img_data->width;
            Top_Left.y = ((y * 300.0f)) / img_data->height;

            ImVec2 Bottom_Right = { (Top_Left.x + (350.0f / img_data->width)),
                                    (Top_Left.y + (300.0f / img_data->height)) };
            //Bottom_Right = { (float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale) };

            ImGui::Image((ImTextureID)
                img_data->render_texture,
                ImVec2(350, 300),
                Top_Left,
                Bottom_Right,
                My_Variables->tint_col,
                My_Variables->border_col);

            ImGui::SameLine();
        }
        ImGui::NewLine();
    }
}