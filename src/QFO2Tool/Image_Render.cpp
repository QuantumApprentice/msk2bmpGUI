#include "Image_Render.h"
#include "Zoom_Pan.h"
#include "imgui_internal.h"

//TODO: check if ImGui_SDL_Render.cpp/h is used anywhere?


//render a full image after palettizing
void image_render(variables* My_Variables, image_data* img_data)
{
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    //shortcuts
    float scale = img_data->scale;
    int width   = img_data->width;
    int height  = img_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan with
    window->DrawList->AddImage(
        (ImTextureID)(uintptr_t)img_data->render_texture,
        top_corner(img_data), bottom_corner(size, top_corner(img_data)),
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

}
