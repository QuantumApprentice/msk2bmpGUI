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
    free(free_me); //freed

    return surface_8;
}

//calls FRM_color_convert()
//returns pointer to FRM header, including frame info
uint8_t* Crop_Frame(pxl_pos* pos_data, ANM_Frame* anm_frame, Palette* FO_Palette)
{
    int Frame_Width  = pos_data->w;
    int Frame_Height = pos_data->h;

    Rect src_rectangle;
    src_rectangle.w = Frame_Width;
    src_rectangle.h = Frame_Height;
    src_rectangle.x = pos_data->l_pxl;
    src_rectangle.y = pos_data->t_pxl;

    Rect dst_rectangle;
    dst_rectangle.w = Frame_Width;
    dst_rectangle.h = Frame_Height;
    dst_rectangle.x = 0;
    dst_rectangle.y = 0;

    Surface* out_surface = Create_RGBA_Surface(Frame_Width, Frame_Height);

    BlitSurface(anm_frame->frame_start, src_rectangle, out_surface, dst_rectangle);

    uint8_t* out_data = FRM_Color_Convert(out_surface, FO_Palette, 0);

    free(out_surface);
    return out_data;
}

bool crop_animation_SURFACE(image_data* src, image_data* dst, Palette* pal, int algo, shader_info* shaders)
{
    int num_frms = 0;
    bool free_surface = false;

    int dir = dst->display_orient_num = src->display_orient_num;
    dst->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir)*6);
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
            break;
        }

        num_frms = dst->ANM_dir[i].num_frames = src->ANM_dir[i].num_frames;
        dst->ANM_dir[i].frame_data  = (ANM_Frame*)malloc(sizeof(ANM_Frame)*num_frms);
        if (!dst->ANM_dir[dir].frame_data) {
            //TODO: log out to file
            set_popup_warning(
                "[ERROR] crop_animation_SURFACE()\n\n"
                "Couldn't convert image to FRM."
            );
            printf("Couldn't convert image to FRM: %d\n", __LINE__);
            free(dst->ANM_dir);
            return false;
        }
        dst->ANM_dir[i].frame_box   = (rectangle*)malloc(sizeof(rectangle)*num_frms);
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
        dst->ANM_dir[i].orientation = (Direction)i;

        rectangle bounding_box = {};
        rectangle FRM_bounding_box = {};
        pxl_pos prev_pos;

        // loop over all frames in each direction
        for (int j = 0; j < num_frms; j++)
        {
            Surface* src_surface = src->ANM_dir[i].frame_data[j].frame_start;
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
                free(dst->ANM_dir);
                for (int i = 0; i < 6; i++) {
                    free(dst->ANM_dir[i].frame_data);
                }
                free(dst->ANM_dir[i].frame_box);
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

            ANM_Frame* src_frame = &src->ANM_dir[i].frame_data[j];
            ANM_Frame* dst_frame = &dst->ANM_dir[i].frame_data[j];
            dst_frame->Frame_Width  = curr_pos.w;
            dst_frame->Frame_Height = curr_pos.h;
            dst_frame->Frame_Size   = curr_pos.w*curr_pos.h;
            if (j > 0) {
                dst_frame->Shift_Offset_x = (curr_pos.l_pxl + curr_pos.r_pxl) / 2 - (prev_pos.l_pxl + prev_pos.r_pxl) / 2;
                dst_frame->Shift_Offset_y =  curr_pos.b_pxl - prev_pos.b_pxl;
            } else {
                //set frame 0 offset
                dst_frame->Shift_Offset_x = 0;
                dst_frame->Shift_Offset_y = 0;
            }

            Surface* surface_8 = crop_frame_SURFACE(&curr_pos, src_frame->frame_start, pal, algo);
            if (!surface_8) {
                //TODO: log out to file
                printf("Error: Unable to allocate for surface_8: %d\n", __LINE__);
                free(dst->ANM_dir);
                for (int i = 0; i < 6; i++) {
                    free(dst->ANM_dir[i].frame_data);
                }
                free(dst->ANM_dir[i].frame_box);
                return false;
            }

            calculate_bounding_box_SURFACE(
                &bounding_box, &FRM_bounding_box,
                dst_frame, &dst->ANM_dir[i].frame_box[j]);

            dst_frame->frame_start = surface_8;
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
        free(dst->ANM_dir);
        for (int i = 0; i < 6; i++) {
            free(dst->ANM_dir[i].frame_data);
            free(dst->ANM_dir[i].frame_box);
        }
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
        dst->ANM_dir[dir].frame_data[0].frame_start,
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

bool Crop_Animation(image_data* img_data, image_data* edit_data, Palette* FO_Palette)
{
    // int FRM_frame_size = sizeof(FRM_Frame);
    int num_frames = 0;
    Color rgba;
    bool free_surface = false;

    FRM_Header* FRM_header = (FRM_Header*)malloc(sizeof(FRM_Header));
    if (!FRM_header) {
        printf("Unable to allocate memory for FRM_header: %d\n", __LINE__);
        return false;
    }
    else {
        new(FRM_header) FRM_Header;
    }

    FRM_Dir* FRM_dir = (FRM_Dir*)malloc(sizeof(FRM_Dir) * 6);
    if (!FRM_dir) {
        printf("Unable to allocate memory for FRM_dir: %d\n", __LINE__);
        return false;
    }
    else {
        new (FRM_dir) FRM_Dir[6];
        //FRM_dir[img_data->display_orient_num].orientation = (img_data->display_orient_num);
    }

    edit_data->display_orient_num = img_data->display_orient_num;

    //loop over all frames in all directions
    for (int i = 0; i < 6; i++)
    {
        if (img_data->ANM_dir[i].orientation > -1) {

            num_frames = img_data->ANM_dir[i].num_frames;
            FRM_dir[i].num_frames  = num_frames;
            FRM_dir[i].orientation = (Direction)i;

            pxl_pos* pos_data = (pxl_pos*)malloc(sizeof(pxl_pos)*num_frames);
            if (!pos_data) {
                printf("Unable to allocate memory for pos_data[]: %d\n", __LINE__);
                return false;
            }
            new(pos_data)pxl_pos[num_frames];

            FRM_Frame** frm_frame_ptrs = (FRM_Frame**)malloc(sizeof(FRM_Frame*)*num_frames);
            if (!frm_frame_ptrs) {
                printf("Unable to allocate memory for frm_frame: %d\n", __LINE__);
            }
            FRM_dir[i].bounding_box = (rectangle*)malloc(sizeof(rectangle) * num_frames);
            if (!FRM_dir[i].bounding_box) {
                printf("Unable to allocate memory for FRM_dir[]->bounding_box: %d", __LINE__);
                return false;
            }

            rectangle bounding_box = {};
            rectangle FRM_bounding_box = {};

            // loop over all frames in one direction
            for (int j = 0; j < num_frames; j++)
            {
                Surface* surface = img_data->ANM_dir[i].frame_data[j].frame_start;
                int width  = surface->w;
                int height = surface->h;
                int pitch  = surface->pitch;

                Surface* Surface_32 = NULL;

                if (surface->channels != 4) {
                    Surface_32 = Convert_Surface_to_RGBA(surface);
                    free_surface = true;
                } else {
                    Surface_32 = surface;
                }
                if (!Surface_32) {
                    printf("Error: unable to allocate Surface_32\n");
                    return false;
                }

                //check every pixel in a frame for edge of cropped image
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        int i = pitch*y + x*sizeof(Color);
                        memcpy(&rgba, &Surface_32->pxls[i], sizeof(Color));
                        // memcpy(&rgba, (uint8_t*)Surface_32->pxls + i, sizeof(Color));

                        if (rgba.a > img_data->alpha_threshold) {
                            if (x < pos_data[j].l_pxl) {
                                pos_data[j].l_pxl = x;
                            }
                            if (x > pos_data[j].r_pxl) {
                                pos_data[j].r_pxl = x;
                            }
                            if (y < pos_data[j].t_pxl) {
                                pos_data[j].t_pxl = y;
                            }
                            if (y > pos_data[j].b_pxl) {
                                pos_data[j].b_pxl = y;
                            }
                        }
                    }
                }

                //assign width and height from pixel edge data
                pos_data[j].w = pos_data[j].r_pxl - pos_data[j].l_pxl;
                pos_data[j].h = pos_data[j].b_pxl - pos_data[j].t_pxl;

                int image_size = pos_data[j].w*pos_data[j].h;

                FRM_Frame* frame_data = (FRM_Frame*)malloc(sizeof(FRM_Frame) + image_size);
                if (!frame_data) {
                    printf("Unable to allocate memory for frame_data: frame: %d, line: %d\n", j, __LINE__);
                    return false;
                }

                frame_data->Frame_Width  = pos_data[j].w;
                frame_data->Frame_Height = pos_data[j].h;
                frame_data->Frame_Size   = image_size;
                if (j > 0) {
                    frame_data->Shift_Offset_x = (pos_data[j].l_pxl + pos_data[j].r_pxl) / 2 - (pos_data[j - 1].l_pxl + pos_data[j - 1].r_pxl) / 2;
                    frame_data->Shift_Offset_y =  pos_data[j].b_pxl - pos_data[j - 1].b_pxl;
                } else {
                    //set frame 0 offset
                    frame_data->Shift_Offset_x = 0;
                    frame_data->Shift_Offset_y = 0;
                }

                //convert each frame to 8-bit paletted, copy to FRM_frame_ptrs at correct position
                uint8_t* data = Crop_Frame(&pos_data[j], &img_data->ANM_dir[i].frame_data[j], FO_Palette);
                memcpy(frame_data->frame_start, data + sizeof(FRM_Header) + sizeof(FRM_Frame), image_size);
                // memcpy((uint8_t*)frame_data + FRM_frame_size, data, data_frame_size);
                frm_frame_ptrs[j] = frame_data;
                free(data);

                calculate_bounding_box(&bounding_box, &FRM_bounding_box, frm_frame_ptrs[j], &FRM_dir[i].bounding_box[j]);// FRM_dir, i, j);

                if (free_surface) {
                    FreeSurface(Surface_32);
                }
            }
            //assign all the malloc'd stuff to appropriate positions in the img_data struct
            edit_data->FRM_bounding_box[i] = FRM_bounding_box;
            FRM_dir[i].frame_data = frm_frame_ptrs;
            edit_data->FRM_hdr = FRM_header;
            edit_data->FRM_dir = FRM_dir;
            edit_data->width  = edit_data->FRM_bounding_box[edit_data->display_orient_num].x2 - edit_data->FRM_bounding_box[edit_data->display_orient_num].x1;
            edit_data->height = edit_data->FRM_bounding_box[edit_data->display_orient_num].y2 - edit_data->FRM_bounding_box[edit_data->display_orient_num].y1;
        }
    }

    if (num_frames > 1) {
        FRM_header->FPS = 10;
        FRM_header->Frames_Per_Orient = num_frames;
    }
    else {
        FRM_header->FPS = 0;
        FRM_header->Frames_Per_Orient = 1;

    }
    edit_data->type = FRM;
    edit_data->FRM_hdr->version = 4;

    if (edit_data->FRM_dir[edit_data->display_orient_num].frame_data) {
        return Render_FRM0_OpenGL(edit_data, edit_data->display_orient_num);
    }
    else {
        printf("FRM image couldn't convert...\n");
        return false;
    }
}