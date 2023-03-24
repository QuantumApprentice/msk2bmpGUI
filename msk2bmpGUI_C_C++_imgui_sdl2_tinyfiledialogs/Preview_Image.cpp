#include "Preview_Image.h"


void Preview_Image(variables* My_Variables, struct image_data* img_data)
{
    float scale = img_data->img_pos.scale;
    int width   = img_data->width;
    int height  = img_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    if (img_data->img_pos.run_once) {

        init_image_pos(&img_data->img_pos, size);

        img_data->img_pos.run_once = false;
    }


    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta, size);


    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    window->DrawList->AddImage(
        (ImTextureID)img_data->render_texture,
        img_data->img_pos.corner_pos, img_data->img_pos.bottom_corner,
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

}