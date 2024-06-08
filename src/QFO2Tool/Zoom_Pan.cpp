#include <stdio.h>
#include <algorithm>

#include "Zoom_Pan.h"

ImVec2 top_corner(image_data* img_data)
{
    ImVec2 corner_pos;
    corner_pos.x = img_data->offset.x + ImGui::GetCursorScreenPos().x;
    corner_pos.y = img_data->offset.y + ImGui::GetCursorScreenPos().y;

    return corner_pos;
}

ImVec2 bottom_corner(ImVec2 size, ImVec2 corner_pos)
{
    ImVec2 bottom_corner;
    //bottom right corner based on zoom
    bottom_corner.x = corner_pos.x + size.x;
    bottom_corner.y = corner_pos.y + size.y;

    return bottom_corner;
}

void viewport_boundary(image_data* img_data, ImVec2 size)
{

    ImVec2 image_offset;
    image_offset.x = ImGui::GetCursorScreenPos().x - ImGui::GetWindowPos().x;
    image_offset.y = ImGui::GetCursorScreenPos().y - ImGui::GetWindowPos().y;

    img_data->offset.x += image_offset.x;
    img_data->offset.y += image_offset.y;

    ImVec2 window_size = ImGui::GetWindowSize();
    if (size.x >= window_size.x) {
        img_data->offset.x = std::max((float)(window_size.x / 2 - size.x), img_data->offset.x);
        img_data->offset.x = std::min((float)(window_size.x / 2         ), img_data->offset.x);
        // technically should update corner_pos here if the offset changed
    }
    else {
        img_data->offset.x = std::max((float)(-size.x / 2               ), img_data->offset.x);
        img_data->offset.x = std::min((float)(window_size.x - size.x / 2), img_data->offset.x);
        // technically should update corner_pos here if the offset changed
    }


    if (size.y >= window_size.y) {
        img_data->offset.y = std::max((float)(window_size.y / 2 - size.y), img_data->offset.y);
        img_data->offset.y = std::min((float)(window_size.y / 2         ), img_data->offset.y);
        // technically should update corner_pos here if the offset changed
    }
    else {
        img_data->offset.y = std::max((float)(-size.y / 2               ), img_data->offset.y);
        img_data->offset.y = std::min((float)(window_size.y - size.y / 2), img_data->offset.y);
        // technically should update corner_pos here if the offset changed
    }

    img_data->offset.x -= image_offset.x;
    img_data->offset.y -= image_offset.y;

}

//handle zoom and panning for the image, plus update image position every frame
void zoom_pan(image_data* img_data, ImVec2 focus_point, ImVec2 mouse_delta)
{
    float mouse_wheel = ImGui::GetIO().MouseWheel;
    if (mouse_wheel > 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()) {
        zoom(1.05, focus_point, img_data);
    }
    else if (mouse_wheel < 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()) {
        zoom(0.95, focus_point, img_data);
    }

    if (ImGui::GetIO().MouseDown[1] && ImGui::IsWindowHovered()) {
        panning(img_data, mouse_delta);
    }

    ImVec2 size = ImVec2(img_data->width  * img_data->scale,
        img_data->height * img_data->scale);

    viewport_boundary(img_data, size);
}

void zoom(float zoom_level, ImVec2 focus_point, image_data* img_data)
{
    float* scale = &img_data->scale;
    ImVec2* offset = &img_data->offset;

    ImVec2 corner_pos = top_corner(img_data);

    float old_zoom = *scale;
    *scale *= zoom_level;

    if (*scale < 0.125)
    {
        *scale = 0.125;
    }

    //mouse position relative to window/screen code here
    ImVec2 zoom_center_offset;
    zoom_center_offset.x = corner_pos.x - focus_point.x;
    zoom_center_offset.y = corner_pos.y - focus_point.y;

    ImVec2 new_corner;
    new_corner.x = focus_point.x + (*scale / old_zoom)*zoom_center_offset.x;
    new_corner.y = focus_point.y + (*scale / old_zoom)*zoom_center_offset.y;

    offset->x += new_corner.x - corner_pos.x;
    offset->y += new_corner.y - corner_pos.y;
}

void panning(struct image_data* img_data, ImVec2 mouse_delta)
{
    //calculate mouse offset
    img_data->offset.x += mouse_delta.x;
    img_data->offset.y += mouse_delta.y;
}
