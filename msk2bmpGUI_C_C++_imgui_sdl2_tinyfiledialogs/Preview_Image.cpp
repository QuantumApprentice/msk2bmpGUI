#include "Preview_Image.h"


void Preview_Image(variables* My_Variables, LF* F_Prop)
{
    image_data* img_data = &F_Prop->img_data;
    float scale = img_data->img_pos.new_zoom;
    int width   = img_data->width;
    int height  = img_data->height;
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)

    ImGuiWindow* window = ImGui::GetCurrentWindow();

    panning(img_data, My_Variables->mouse_delta);

    float mouse_wheel = ImGui::GetIO().MouseWheel;
    if      (mouse_wheel > 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()){
        zoom_wrap(1.05, img_data, My_Variables->new_mouse_pos);
    }
    else if (mouse_wheel < 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()){
        zoom_wrap(0.95, img_data, My_Variables->new_mouse_pos);
    }

    //if (F_Prop->type == FRM) {
        //image I'm trying to pan with
        window->DrawList->AddImage(
            (ImTextureID)img_data->render_texture,
            img_data->img_pos.corner_pos, img_data->img_pos.bottom_corner,
            uv_min, uv_max,
            ImGui::GetColorU32(My_Variables->tint_col));

        //TODO: need to figure out how I'm going to handle scrolling on large images
        ImGui::Dummy(size);
    //}
    //else {
        //old way to load the image
        //need to simplify to just get width, height

        //window->DrawList->AddImage(
        //    (ImTextureID)F_Prop->Optimized_Texture,
        //    img_data->img_pos.corner_pos, img_data->img_pos.bottom_corner,
        //    uv_min, uv_max,
        //    ImGui::GetColorU32(My_Variables->tint_col));


        //ImGui::Image(
        //    (ImTextureID)F_Prop->Optimized_Texture,
        //    ImVec2((float)(size.x), (float)(size.y)),
        //    My_Variables->uv_min,
        //    My_Variables->uv_max,
        //    My_Variables->tint_col,
        //    My_Variables->border_col);
    //}


}