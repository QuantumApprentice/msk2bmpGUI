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
};

struct offsets {
    int16_t x_offset = 0;
    int16_t y_offset = 0;
};

uint8_t* Crop_Frame(pixel_position* pos_data, FRM_Frame* frm_frame, ANM_Frame* anm_frame)
{
    int Frame_Width = pos_data->r_pxl - pos_data->l_pxl;
    int Frame_Height = pos_data->b_pxl - pos_data->t_pxl;
    frm_frame->Frame_Width = Frame_Width;
    frm_frame->Frame_Height = Frame_Height;

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

    //SDL_Surface* out_surface = SDL_CreateRGBSurface(0, Frame_Width, Frame_Height, 32, 0,0,0,0);
    SDL_Surface* out_surface = SDL_CreateRGBSurfaceWithFormat(0, Frame_Width, Frame_Height, 32, SDL_PIXELFORMAT_RGBA8888);

    SDL_BlitSurface(anm_frame->frame_start, &src_rectangle, out_surface, &dst_rectangle);

    SDL_PixelFormat* pxl_trash = loadPalette(NULL);

    uint8_t* out_data = FRM_Color_Convert(out_surface, pxl_trash, 0);

    return out_data;
}

void Crop_Animation(image_data* img_data)
{
    int num_frames = img_data->ANM_dir[img_data->display_orient_num].num_frames;

    Pxl_RGBA_32 rgba;
    pixel_position* pos_data = (pixel_position*)malloc(sizeof(pixel_position)*num_frames);
    new(pos_data) pixel_position[num_frames];

    //TODO: loop over all frames in one direction
    //TODO: loop over all frames in all directions
    //for (int i = 0; i < 6; i++)
    //{
    //    if (img_data->ANM_dir[i].orientation > -1) {
    //    }
    //}

    for (int j = 0; j < num_frames; j++)
    {
        SDL_Surface* surface = img_data->ANM_dir[img_data->display_orient_num].frame_data[j].frame_start;
        int width  = surface->w;
        int height = surface->h;
        int pitch  = surface->pitch;

        SDL_Surface* Surface_32 = NULL;

        if (surface->format->format != SDL_PIXELFORMAT_ABGR8888) {
            SDL_PixelFormat* pxlFMT_UnPal;
            pxlFMT_UnPal = SDL_AllocFormat(SDL_PIXELFORMAT_ABGR8888);
            Surface_32 = SDL_ConvertSurface(surface, pxlFMT_UnPal, 0);
        }
        else {
            Surface_32 = surface;
        }
        if (!Surface_32) {
            printf("Error: %s\n", SDL_GetError());
        }

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
    }

    int left   = INT_MAX;
    int right  = 0;
    int top    = INT_MAX;
    int bottom = 0;

    int total_width  = 0;
    int total_height = 0;

    for (int i = 0; i < num_frames; i++)
    {
        if (left < pos_data[i].l_pxl) {
            left = pos_data[i].l_pxl;
        }
        if (right > pos_data[i].r_pxl) {
            right = pos_data[i].r_pxl;
        }
        if (top < pos_data[i].t_pxl) {
            top = pos_data[i].t_pxl;
        }
        if (bottom > pos_data[i].b_pxl) {
            bottom = pos_data[i].b_pxl;
        }
    }
    total_width  = right  - left;
    total_height = bottom - top;

    FRM_Dir*   FRM_dir = (FRM_Dir*)malloc(sizeof(FRM_Dir) * 6);
    FRM_Frame* FRM_frame = (FRM_Frame*)malloc(sizeof(FRM_Frame)*num_frames);
    FRM_dir[img_data->display_orient_num].frame_data = FRM_frame;
    img_data->FRM_dir = FRM_dir;

    FRM_Frame* frm_frame = img_data->FRM_dir[img_data->display_orient_num].frame_data; //(offsets*)malloc(sizeof(offsets)*num_frames);
    frm_frame[0].Shift_Offset_x = 0;// (pos_data[0].r_pxl - pos_data[0].l_pxl) / 2;
    frm_frame[0].Shift_Offset_y = 0;//  pos_data[0].t_pxl - pos_data[0].b_pxl;

    for (int i = 1; i < num_frames; i++)
    {
        frm_frame[i].Shift_Offset_x = (pos_data[i].l_pxl + pos_data[i].r_pxl) / 2 - (pos_data[i - 1].l_pxl + pos_data[i - 1].r_pxl) / 2;
        frm_frame[i].Shift_Offset_y =  pos_data[i].b_pxl - pos_data[i - 1].b_pxl;

        img_data->FRM_data = Crop_Frame(&pos_data[i], &frm_frame[i], &img_data->ANM_dir[img_data->display_orient_num].frame_data[i]);
    }


    printf("just a place to set a break");
}




