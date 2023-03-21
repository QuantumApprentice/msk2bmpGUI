#include "Preview_Image.h"


void Preview_Image(variables* My_Variables, LF* F_Prop)
{
    float scale = F_Prop->img_data.img_pos.new_zoom;
    int width   = F_Prop->img_data.width;
    int height  = F_Prop->img_data.height;

    float mouse_wheel = ImGui::GetIO().MouseWheel;
    if      (mouse_wheel > 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()){
        zoom_wrap(1.05, &F_Prop->img_data);
    }
    else if (mouse_wheel < 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()){
        zoom_wrap(0.95, &F_Prop->img_data);
    }

    if (F_Prop->type == FRM) {
        ImGui::Image(
            (ImTextureID)F_Prop->img_data.render_texture,
            ImVec2((float)(width*scale), (float)(height*scale)),
            My_Variables->uv_min,
            My_Variables->uv_max,
            My_Variables->tint_col,
            My_Variables->border_col);
    }
    else {
        //old way to load the image
        //need to simplify to just get width, height
        ImGui::Image(
            (ImTextureID)F_Prop->Optimized_Texture,
            ImVec2((float)(width*scale), (float)(height*scale)),
            My_Variables->uv_min,
            My_Variables->uv_max,
            My_Variables->tint_col,
            My_Variables->border_col);
    }


}