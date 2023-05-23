#include "Edit_Animation.h"

union Pxl_RGBA_32 {
    struct {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    uint8_t arr[4];
};

void Crop_Animation(image_data* img_data)
{
    SDL_Surface* surface = img_data->ANM_dir[img_data->display_orient_num].frame_data[0].frame_start;
    int width  = surface->w;
    int height = surface->h;
    int pitch  = surface->pitch;

    int farthest_left_pixel  = INT_MAX;
    int farthest_right_pixel = 0;
    int top_most_pixel       = INT_MAX;
    int bottom_most_pixel    = 0;

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

    Pxl_RGBA_32 rgba;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = (pitch * y) + x * (sizeof(Pxl_RGBA_32));

            memcpy(&rgba, (uint8_t*)Surface_32->pixels + i, sizeof(Pxl_RGBA_32));

            if (rgba.a > img_data->alpha_threshold) {
                if (x < farthest_left_pixel) {
                    farthest_left_pixel = x;
                }
                if (x > farthest_right_pixel) {
                    farthest_right_pixel = x;
                }
                if (y < top_most_pixel) {
                    top_most_pixel = y;
                }
                if (y > bottom_most_pixel) {
                    bottom_most_pixel = y;
                }
            }
        }
    }

    //img_data->ANM_dir->bounding_box.

    printf("\nfarthest left pixel: %d \nfarthest right pixel: %d \ntop most pixel: %d \nbottom most pixel: %d",
            farthest_left_pixel, farthest_right_pixel, top_most_pixel, bottom_most_pixel);
}