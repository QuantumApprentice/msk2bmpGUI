#include <limits.h>

#include "Edit_Animation.h"
#include "FRM_Convert.h"

union Pxl_RGBA_32 {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint8_t arr[4];
};

struct pixel_position {
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

uint8_t* Crop_Frame(pixel_position* pos_data, ANM_Frame* anm_frame)
{
    int Frame_Width = pos_data->r_pxl  - pos_data->l_pxl;
    int Frame_Height = pos_data->b_pxl - pos_data->t_pxl;

    SDL_Rect src_rectangle;
    src_rectangle.w = Frame_Width;
    src_rectangle.h = Frame_Width;
    src_rectangle.x = pos_data->l_pxl;
    src_rectangle.y = pos_data->t_pxl;

    SDL_Rect dst_rectangle;
    dst_rectangle.w = Frame_Width;
    dst_rectangle.h = Frame_Height;
    dst_rectangle.x = 0;
    dst_rectangle.y = 0;

    SDL_Surface* out_surface = SDL_CreateRGBSurfaceWithFormat(0, Frame_Width, Frame_Height, 32, SDL_PIXELFORMAT_RGBA8888);

    SDL_BlitSurface(anm_frame->frame_start, &src_rectangle, out_surface, &dst_rectangle);

    SDL_PixelFormat* pxl_trash = loadPalette(NULL);

    uint8_t* out_data = FRM_Color_Convert(out_surface, pxl_trash, 0);

    return out_data;
}

bool Crop_Animation(image_data* img_data, image_data* edit_data)
{
    int FRM_frame_size = sizeof(FRM_Frame);
    int num_frames = 0;
    Pxl_RGBA_32 rgba;

    SDL_PixelFormat* pxlFMT_UnPal;
    pxlFMT_UnPal = SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
    if (!pxlFMT_UnPal) {
        printf("Unable to allocate memory for pxlFMT_UnPal: %d\n", SDL_GetError);
        return false;
    }
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

            pixel_position* pos_data = (pixel_position*)malloc(sizeof(pixel_position)*num_frames);
            if (!pos_data) {
                printf("Unable to allocate memory for pos_data[]: %d\n", __LINE__);
                return false;
            }
            else {
                new(pos_data) pixel_position[num_frames];
            }

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
                SDL_Surface* surface = img_data->ANM_dir[i].frame_data[j].frame_start;
                int width = surface->w;
                int height = surface->h;
                int pitch = surface->pitch;

                SDL_Surface* Surface_32 = NULL;

                if (surface->format->format != SDL_PIXELFORMAT_ABGR8888) {
                    Surface_32 = SDL_ConvertSurface(surface, pxlFMT_UnPal, 0);
                    free_surface = true;
                }
                else {
                    Surface_32 = surface;
                }
                if (!Surface_32) {
                    printf("Error: %s\n", SDL_GetError());
                }

                //check every pixel in a frame for edge of cropped image
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        int i = (pitch * y) + x * (sizeof(Pxl_RGBA_32));
                        memcpy(&rgba, (uint8_t*)Surface_32->pixels + i, sizeof(Pxl_RGBA_32));

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

                int data_frame_size = pos_data[j].w*pos_data[j].h;

                FRM_Frame* frame_data = (FRM_Frame*)malloc(FRM_frame_size + data_frame_size);
                if (!frame_data) {
                    printf("Unable to allocate memory for frame_data: frame: %d, line: %d\n", j, __LINE__);
                    return false;
                }

                frame_data->Frame_Width = pos_data[j].w;
                frame_data->Frame_Height = pos_data[j].h;
                frame_data->Frame_Size = data_frame_size;
                if (j > 0) {
                    frame_data->Shift_Offset_x = (pos_data[j].l_pxl + pos_data[j].r_pxl) / 2 - (pos_data[j - 1].l_pxl + pos_data[j - 1].r_pxl) / 2;
                    frame_data->Shift_Offset_y = pos_data[j].b_pxl - pos_data[j - 1].b_pxl;
                }
                else {
                    //set frame 0 offset
                    frame_data->Shift_Offset_x = 0;
                    frame_data->Shift_Offset_y = 0;
                }

                //convert each frame to 8-bit paletted, copy to FRM_frame_ptrs at correct position
                uint8_t* data = Crop_Frame(&pos_data[j], &img_data->ANM_dir[i].frame_data[j]);
                memcpy((uint8_t*)frame_data + FRM_frame_size, data, data_frame_size);
                frm_frame_ptrs[j] = frame_data;
                free(data);

                calculate_bounding_box(&bounding_box, &FRM_bounding_box, frm_frame_ptrs[j], FRM_dir, i, j);

                if (free_surface) {
                    SDL_FreeSurface(Surface_32);
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

    SDL_FreeFormat(pxlFMT_UnPal);

    if (edit_data->FRM_dir[edit_data->display_orient_num].frame_data) {
        return Render_FRM0_OpenGL(edit_data, edit_data->display_orient_num);
    }
    else {
        printf("FRM image couldn't convert...\n");
        return false;
    }

}




