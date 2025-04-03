#include <limits.h>

#include "Edit_Animation.h"
#include "FRM_Convert.h"
#include "ImGui_Warning.h"

#include "Image2Texture.h"

struct pxl_pos {
    int l_pxl = INT_MAX;        //left most pixel
    int r_pxl = 0;              //right most pixel
    int t_pxl = INT_MAX;        //top most pixel
    int b_pxl = 0;              //bottom most pixel
    int w = 0;
    int h = 0;
};

struct offsets {
    int16_t x_offset = 0;
    int16_t y_offset = 0;
};

//crops src onto 8bit surface
//removing pixels by their alpha value
//returns 8bit surface, sized at crop values
Surface* crop_frame_SURFACE(pxl_pos* curr_pos, Surface* src, Palette* pal, int algo)
{
    int w = curr_pos->w;
    int h = curr_pos->h;

    Rect src_rect;
    src_rect.w = w;
    src_rect.h = h;
    src_rect.x = curr_pos->l_pxl;
    src_rect.y = curr_pos->t_pxl;

    Rect dst_rect;
    dst_rect.w = w;
    dst_rect.h = h;
    dst_rect.x = 0;
    dst_rect.y = 0;

    //crop surface first, leaving fewer pixels to convert
    Surface* free_me = Create_RGBA_Surface(w, h);
    BlitSurface(src, src_rect, free_me, dst_rect);
    Surface* surface_8 = PAL_Color_Convert(free_me, pal, algo);
    free(free_me); //free the Freeman

    return surface_8;
}

bool crop_animation_SURFACE(image_data* src, image_data* dst, Palette* pal, int algo, shader_info* shaders)
{
    int num_frms = 0;
    bool free_surface = false;

    int dir = dst->display_orient_num = src->display_orient_num;
    dst->ANM_dir = (ANM_Dir*)calloc(1,sizeof(ANM_Dir)*6);
    if (!dst->ANM_dir) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] crop_animation_SURFACE()\n\n"
            "Failed to allocate for ANM_dir."
        );
        printf("Failed to allocate for ANM_dir: %d\n", __LINE__);
    }

    //loop over all frames in all directions
    for (int i = 0; i < 6; i++)
    {
        if (src->ANM_dir[i].orientation < 0) {
            dst->ANM_dir[i].orientation = no_data;
            continue;
        }
        dst->ANM_dir[i].orientation = src->ANM_dir[i].orientation;

        num_frms = dst->ANM_dir[i].num_frames = src->ANM_dir[i].num_frames;
        dst->ANM_dir[i].frame_data = (Surface**)calloc(1,sizeof(Surface*)*num_frms);
        if (!dst->ANM_dir[i].frame_data) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] crop_animation_SURFACE()\n\n"
                "Couldn't convert image to FRM."
            );
            printf("Couldn't convert image to FRM: %d\n", __LINE__);
            free(dst->ANM_dir);
            return false;
        }
        dst->ANM_dir[i].frame_box = (rectangle*)malloc(sizeof(rectangle)*num_frms);
        if (!dst->ANM_dir[i].frame_box) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] crop_animation_SURFACE()\n\n"
                "Failed to allocate for ANM_dir[i].frame_box."
            );
            free(dst->ANM_dir);
            for (int i = 0; i < 6; i++) {
                free(dst->ANM_dir[i].frame_data);
            }
            return false;
        }

        rectangle bounding_box = {};
        rectangle FRM_bounding_box = {};
        pxl_pos prev_pos;

        // loop over all frames in each direction
        for (int j = 0; j < num_frms; j++)
        {
            Surface* src_surface = src->ANM_dir[i].frame_data[j];
            int src_width  = src_surface->w;
            int src_height = src_surface->h;
            int src_pitch  = src_surface->pitch;

            Surface* surface_32 = NULL;

            if (src_surface->channels != 4) {
                surface_32 = Convert_Surface_to_RGBA(src_surface);
            } else {
                surface_32 = src_surface;
            }
            if (!surface_32) {
                //TODO: log out to file
                set_popup_warning(
                    "[ERROR] crop_animation_SURFACE()\n\n"
                    "Unable to allocate Surface_32."
                );
                printf("Error: unable to allocate Surface_32: %d\n", __LINE__);
                for (int i = 0; i < 6; i++) {
                    free(dst->ANM_dir[i].frame_data);
                }
                free(dst->ANM_dir[i].frame_box);
                free(dst->ANM_dir);
                return false;
            }

            pxl_pos curr_pos;

            //check every pixel in a frame for edge of cropped image
            for (int y = 0; y < src_height; y++)
            {
                for (int x = 0; x < src_width; x++)
                {
                    Color rgba;
                    int i = src_pitch*y + x*sizeof(Color);
                    memcpy(&rgba, &surface_32->pxls[i], sizeof(Color));

                    if (rgba.a > src->alpha_threshold) {
                        if (x < curr_pos.l_pxl) {
                            curr_pos.l_pxl = x;
                        }
                        if (x > curr_pos.r_pxl) {
                            curr_pos.r_pxl = x;
                        }
                        if (y < curr_pos.t_pxl) {
                            curr_pos.t_pxl = y;
                        }
                        if (y > curr_pos.b_pxl) {
                            curr_pos.b_pxl = y;
                        }
                    }
                }
            }

            if (surface_32 != src_surface) {
                free(surface_32);
            }
            //assign width and height from pixel edge data
            curr_pos.w = curr_pos.r_pxl - curr_pos.l_pxl;
            curr_pos.h = curr_pos.b_pxl - curr_pos.t_pxl;

            Surface* src_frame = src->ANM_dir[i].frame_data[j];
            dst->ANM_dir[i].frame_data[j] = crop_frame_SURFACE(&curr_pos, src_frame, pal, algo);
            Surface* dst_frame = dst->ANM_dir[i].frame_data[j];
            if (!dst_frame) {
                //TODO: log out to file
                set_popup_warning(
                    "[ERROR] crop_animation_SURFACE()\n\n"
                    "Failed to allocate frame_data[j] surface8."
                );
                printf("Error: Unable to allocate frame_data[j] surface8: %d\n", __LINE__);

                for (int i = 0; i < 6; i++) {
                    free(dst->ANM_dir[i].frame_data);
                }
                free(dst->ANM_dir[i].frame_box);
                free(dst->ANM_dir);
                return false;
            }

            if (j > 0) {
                dst_frame->x = (curr_pos.l_pxl + curr_pos.r_pxl) / 2 - (prev_pos.l_pxl + prev_pos.r_pxl) / 2;
                dst_frame->y =  curr_pos.b_pxl - prev_pos.b_pxl;
            } else {
                //set frame 0 offset
                dst_frame->x = 0;
                dst_frame->y = 0;
            }

            calculate_bounding_box_SURFACE(
                &bounding_box, &FRM_bounding_box,
                dst_frame, &dst->ANM_dir[i].frame_box[j]);

            prev_pos = curr_pos;
        }

        dst->ANM_bounding_box[i] = FRM_bounding_box;
    }

    //malloc a blank FRM_Header for use in editor/viewer
    dst->FRM_hdr = (FRM_Header*)calloc(1, sizeof(FRM_Header));
    if (!dst->FRM_hdr) {
        //TODO: log out to file
        set_popup_warning(
            "[ERROR] crop_animation_SURFACE()\n\n"
            "Failed to allocate FRM_hdr."
        );
        printf("Error: Failed to allocate FRM_hdr: %d\n", __LINE__);
        for (int i = 0; i < 6; i++) {
            free(dst->ANM_dir[i].frame_data);
            free(dst->ANM_dir[i].frame_box);
        }
        free(dst->ANM_dir);
        return false;
    }

    dst->FRM_hdr->version = 4;
    dst->type    = FRM;
    dst->width   = dst->ANM_bounding_box[dir].x2 - dst->ANM_bounding_box[dir].x1;
    dst->height  = dst->ANM_bounding_box[dir].y2 - dst->ANM_bounding_box[dir].y1;

    if (num_frms > 1) {
        dst->FRM_hdr->FPS = 10;
        dst->FRM_hdr->Frames_Per_Orient = num_frms;
    }
    else {
        dst->FRM_hdr->FPS = 0;
        dst->FRM_hdr->Frames_Per_Orient = 1;
    }

    dst->FRM_texture = init_texture(
        dst->ANM_dir[dir].frame_data[0],
        dst->width,
        dst->height,
        dst->type);
    if (!dst->FRM_texture) {
        //TODO: log out to file
        printf("init_texture failed: %d", __LINE__);
        return false;
    }

    bool success = framebuffer_init(
        &dst->render_texture,
        &dst->framebuffer,
        dst->width, dst->height);
    if (!success) {
        //TODO: log out to file
        // set_popup_warning(
        //     "[ERROR] crop_animation_SURFACE()\n\n"
        //     "framebuffer_init failed."
        // );
        printf("Error: framebuffer_init failed: %d", __LINE__);
        return false;
    }

    return success;
}