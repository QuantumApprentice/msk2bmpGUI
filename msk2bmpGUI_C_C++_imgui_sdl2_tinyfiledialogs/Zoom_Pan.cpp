#pragma once
#include <stdio.h>

#include "Zoom_Pan.h"

void zoom_wrap(float zoom_level, image_data* img_data, struct position focus_point) {
    zoom(zoom_level,
         focus_point,
        &img_data->img_pos.new_zoom,
        &img_data->img_pos.corner_pos,
        &img_data->img_pos.offset);
}

void zoom(float zoom_level, struct position focus_point, float* new_zoom, ImVec2* corner_pos, position* offset)
{
    float old_zoom = *new_zoom;
    *new_zoom *= zoom_level;

    if (*new_zoom < 0.125)
    {
        *new_zoom = 0.125;
    }

    //////////////////////////////////////////////////////////////////////////////////
    //mouse position relative to window/screen code here

    position zoom_center_offset;
    zoom_center_offset.x = corner_pos->x - focus_point.x;
    zoom_center_offset.y = corner_pos->y - focus_point.y;

    position new_corner;

    new_corner.x = focus_point.x + ( *new_zoom / old_zoom)*zoom_center_offset.x;
    new_corner.y = focus_point.y + ( *new_zoom / old_zoom)*zoom_center_offset.y;

    offset->x += new_corner.x - corner_pos->x;
    offset->y += new_corner.y - corner_pos->y;

    corner_pos->x = new_corner.x;
    corner_pos->y = new_corner.y;

    //////////////////////////////////////////////////////////////////////////////////
}

struct position mouse_pos_to_texture_coord(struct position pos, float new_zoom, int frame_width, int frame_height, float* bottom_left_pos)
{
    float scale = 1 / new_zoom;

    position absolute_pos;
    absolute_pos.x = scale * (    pos.x / frame_width ) + bottom_left_pos[0];
    absolute_pos.y = scale * (1 - pos.y / frame_height) + bottom_left_pos[1];

    return absolute_pos;
}

void panning(struct image_data* img_data, position offset)
{
    float scale = img_data->img_pos.new_zoom;
    int width  = img_data->width;
    int height = img_data->height;
    ImVec2 size = ImVec2((float)(width*scale), (float)(height*scale));

    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();

    if (ImGui::GetIO().MouseDown[1] && ImGui::IsWindowHovered()) {
    //calculate mouse offset and add to img_pos.offset
        img_data->img_pos.offset.x += offset.x;
        img_data->img_pos.offset.y += offset.y;
    //printf("offset.x: %f\n", img_data->img_pos.offset.x);
    //printf("offset.y: %f\n", img_data->img_pos.offset.y);
    }

    //add initial image position to img_pos.offset (accounts for window/screen position)
    img_data->img_pos.corner_pos.x = img_data->img_pos.offset.x + cursor_screen_pos.x;
    img_data->img_pos.corner_pos.y = img_data->img_pos.offset.y + cursor_screen_pos.y;

    //bottom right corner based on zoom
    img_data->img_pos.bottom_corner.x = img_data->img_pos.corner_pos.x + size.x;
    img_data->img_pos.bottom_corner.y = img_data->img_pos.corner_pos.y + size.y;


}