#include "imgui-docking/imgui_internal.h"
#include "Preview_Image.h"
#include "Zoom_Pan.h"


void Preview_Image(variables* My_Variables, struct image_data* img_data)
{
    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);
    //handle frame display by orientation and number
    int orient  = img_data->display_orient_num;
    int frame   = img_data->display_frame_num;
    int max_frm = img_data->FRM_Info.Frames_Per_Orient;

    float scale = img_data->scale;

    int width   = img_data->FRM_bounding_box[orient].x2 - img_data->FRM_bounding_box[orient].x1;
    int height  = img_data->FRM_bounding_box[orient].y2 - img_data->FRM_bounding_box[orient].y1;


    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    window->DrawList->AddImage(
        (ImTextureID)img_data->render_texture,
        top_corner(img_data), bottom_corner(size, top_corner(img_data)),
        //corner_pos, bottom_corner,
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));


    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

}