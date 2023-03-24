#pragma once
#include <stdio.h>

#include "Zoom_Pan.h"

void init_image_pos(image_position* img_pos, ImVec2 size)
{
    //add initial image position to img_pos.offset (accounts for window/screen position)
    img_pos->corner_pos.x = img_pos->offset.x + ImGui::GetCursorScreenPos().x;
    img_pos->corner_pos.y = img_pos->offset.y + ImGui::GetCursorScreenPos().y;

    ////bottom right corner based on zoom
    //img_pos->bottom_corner.x = img_pos->corner_pos.x + size.x;
    //img_pos->bottom_corner.y = img_pos->corner_pos.y + size.y;
}

void zoom_pan(image_data* img_data, position focus_point, position mouse_delta, ImVec2 size)
{
    float mouse_wheel = ImGui::GetIO().MouseWheel;
    if (mouse_wheel > 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()) {
        zoom(1.05, focus_point, &img_data->img_pos);
    }
    else if (mouse_wheel < 0 && (ImGui::GetIO().KeyCtrl) && ImGui::IsWindowHovered()) {
        zoom(0.95, focus_point, &img_data->img_pos);
    }


    if (ImGui::GetIO().MouseDown[1] && ImGui::IsWindowHovered()) {
        panning(img_data, mouse_delta);
    }


    //ImVec2 size = ImVec2(img_data->width  * img_data->img_pos.scale,
    //                     img_data->height * img_data->img_pos.scale);
    //bottom right corner based on zoom
    img_data->img_pos.bottom_corner.x = img_data->img_pos.corner_pos.x + size.x;
    img_data->img_pos.bottom_corner.y = img_data->img_pos.corner_pos.y + size.y;

}

void zoom(float zoom_level, struct position focus_point, image_position* img_pos)
{
    float* scale = &img_pos->scale;
    ImVec2* corner_pos = &img_pos->corner_pos;
    ImVec2* bottom_corner = &img_pos->bottom_corner;
    position* offset = &img_pos->offset;

    float old_zoom = *scale;
    *scale *= zoom_level;

    if (*scale < 0.125)
    {
        *scale = 0.125;
    }

    //mouse position relative to window/screen code here
    position zoom_center_offset;
    zoom_center_offset.x = corner_pos->x - focus_point.x;
    zoom_center_offset.y = corner_pos->y - focus_point.y;

    position new_corner;
    new_corner.x = focus_point.x + ( *scale / old_zoom)*zoom_center_offset.x;
    new_corner.y = focus_point.y + ( *scale / old_zoom)*zoom_center_offset.y;

    offset->x += new_corner.x - corner_pos->x;
    offset->y += new_corner.y - corner_pos->y;

    corner_pos->x = new_corner.x;
    corner_pos->y = new_corner.y;
}

void panning(struct image_data* img_data, position mouse_delta)
{
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();

    //calculate mouse offset and add to img_pos.offset
        img_data->img_pos.offset.x += mouse_delta.x;
        img_data->img_pos.offset.y += mouse_delta.y;
    //printf("offset.x: %f\n", img_data->img_pos.offset.x);
    //printf("offset.y: %f\n", img_data->img_pos.offset.y);

    //add initial image position to img_pos.offset (accounts for window/screen position)
    img_data->img_pos.corner_pos.x = img_data->img_pos.offset.x + cursor_screen_pos.x;
    img_data->img_pos.corner_pos.y = img_data->img_pos.offset.y + cursor_screen_pos.y;

}
