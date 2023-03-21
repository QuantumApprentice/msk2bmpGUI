#pragma once
#include "Zoom_Pan.h"
#include <stdio.h>

void zoom_wrap(float zoom_level, image_data* img_data) {
    zoom(zoom_level, 
         img_data->img_pos.new_tex_coord,
        &img_data->img_pos.old_zoom,
        &img_data->img_pos.new_zoom,
         img_data->img_pos.bottom_left);
}

void zoom(float zoom_level, struct position focus_point, float* old_zoom, float* new_zoom, float* bottom_left_pos)
{
    *old_zoom = *new_zoom;
    *new_zoom *= zoom_level;

    if (*new_zoom < 0.125)
    {
        *new_zoom = 0.125;
    }

    bottom_left_pos[0] = focus_point.x - *old_zoom / *new_zoom * (focus_point.x - bottom_left_pos[0]);
    bottom_left_pos[1] = focus_point.y - *old_zoom / *new_zoom * (focus_point.y - bottom_left_pos[1]);
}

struct position mouse_pos_to_texture_coord(struct position pos, float new_zoom, int frame_width, int frame_height, float* bottom_left_pos)
{
    float scale = 1 / new_zoom;

    position absolute_pos;
    absolute_pos.x = scale * (    pos.x / frame_width ) + bottom_left_pos[0];
    absolute_pos.y = scale * (1 - pos.y / frame_height) + bottom_left_pos[1];

    return absolute_pos;
}

void pan(struct image_data* img_data, struct position* image_offset, struct position* mouse_pos)
{
    //static double old_x, old_y;
    float scale = 1 / img_data->img_pos.new_zoom;
    int width  = img_data->width;
    int height = img_data->height;


}
