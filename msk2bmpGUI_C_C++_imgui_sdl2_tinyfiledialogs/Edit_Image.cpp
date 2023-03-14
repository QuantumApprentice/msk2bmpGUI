#include <stdio.h>
#include <stdlib.h>

#include "Edit_Image.h"
#include "imgui-docking/imgui.h"
#include "display_FRM_OpenGL.h"
#include "Load_Files.h"

void texture_paint(int x, int y, int value, unsigned int texture);


void Edit_Image(variables* My_Variables, LF* F_Prop, bool Palette_Update, SDL_Event* event, uint8_t* Color_Pick) {

    static bool run_once = true;
    if (run_once) {
        F_Prop->edit_data.width = F_Prop->Pal_Surface->w;
        F_Prop->edit_data.height = F_Prop->Pal_Surface->h;
        bind_PAL_data(F_Prop->Pal_Surface, &F_Prop->edit_data);
        run_once = false;
    }
    ImVec2 Origin = ImGui::GetItemRectMin();
    int width  = F_Prop->image->w;
    int height = F_Prop->image->h;
    bool image_edited = false;

    //TODO: maybe pass the dithering choice through?
    if (ImGui::IsMouseDown(event->button.button)) {
        image_edited = true;
        float x, y;
        x = ImGui::GetMousePos().x - Origin.x;
        y = ImGui::GetMousePos().y - Origin.y;

        SDL_Rect rect;
        rect.x = x-5;
        rect.y = y-5;
        rect.h = 10;
        rect.w = 10;

        if ((0 <= x && x <= width) && (0 <= y && y <= height)) {

            SDL_FillRect(F_Prop->Pal_Surface, &rect, *Color_Pick);
            //texture_paint(x, y, *Color_Pick, F_Prop->edit_data.PAL_data);
        }
    }

    //Converts unpalettized image to texture for display, sets window bool to true
    if (Palette_Update || image_edited) {
        Update_Palette(F_Prop, false);

        draw_PAL_to_framebuffer(My_Variables->palette,
                               &My_Variables->render_FRM_shader,
                               &My_Variables->giant_triangle,
                               &F_Prop->edit_data,
                                F_Prop->Pal_Surface);
    }

}

SDL_Surface* Create_Map_Mask(SDL_Surface* image, GLuint* texture, bool* window)
{
    int width  = image->w;
    int height = image->h;

    //if (Map_Mask)
    //    { SDL_FreeSurface(Map_Mask); }
    SDL_Surface* Map_Mask = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

    if (Map_Mask) {

        Uint32 color = SDL_MapRGBA(Map_Mask->format,
                                   0, 0, 0, 0);

        for (int i = 0; i < (width*height); i++)
        {
            ((Uint32*)Map_Mask->pixels)[i] = color; //rand() % 255;
            //uint8_t byte =
            //    (rand() % 2 << 0) |
            //    (rand() % 2 << 1) |
            //    (rand() % 2 << 2) |
            //    (rand() % 2 << 3) |
            //    (rand() % 2 << 4) |
            //    (rand() % 2 << 5) |
            //    (rand() % 2 << 6) |
            //    (rand() % 2 << 7);
            //((uint8_t*)MM_Surface->pixels)[i] = byte;
        }

        Image2Texture(Map_Mask,
            texture,
            window);
    }
    else {
        printf("Can't allocate surface for some reason...");
    }
    return Map_Mask;
}

//TODO: change input from My_Variables to F_Prop[]
void Edit_Map_Mask(LF* F_Prop, SDL_Event* event, bool* Palette_Update, ImVec2 Origin)
{
    int width  = F_Prop->image->w;
    int height = F_Prop->image->h;

    SDL_Surface* BB_Surface = F_Prop->Map_Mask;
    Uint32 white = SDL_MapRGB(F_Prop->Map_Mask->format,
                              255, 255, 255);

    ImVec2 MousePos = ImGui::GetMousePos();
    int x = (int)(MousePos.x - Origin.x);
    int y = (int)(MousePos.y - Origin.y);
    int pitch = BB_Surface->pitch;

    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.h = 10;
    rect.w = 10;

    if (ImGui::IsMouseDown(event->button.button)) {

        if ((0 <= x && x <= width) && (0 <= y && y <= height)) {

            SDL_FillRect(F_Prop->Map_Mask, &rect, white);
            *Palette_Update = true;


            ///*TODO: This stuff didn't work, delete it when done                   */
            ///*TODO: two problems with using binary surface:
            ///       pixel addressing skips by 8 pixels at a time,
            ///       and SDL_FillRect() doesn't work                               */
            //for (int i = 0; i < 4; i++)
            //{
            //    uint8_t* where_i_want_to_draw = 
            //                &((uint8_t*)BB_Surface->pixels)[pitch*y + x/8 + i];
            //    ((uint8_t*)where_i_want_to_draw)[0] = 255;
            //}
            ///*OpenGl stuff didn't work either :                                   */
            //opengl_stuff();
        }
    }

    if (*Palette_Update && (F_Prop->type == FRM)) {
    ///re-copy Pal_Surface to Final_Render each time to allow 
    ///transparency through the mask surface painting
        Update_Palette(F_Prop, true);
    }
    else if (*Palette_Update && (F_Prop->type == MSK)) {
        Update_Palette(F_Prop, true);
    }
}

//bool blend == true = blend surfaces
void Update_Palette(struct LF* files, bool blend) {
    SDL_FreeSurface(files->Final_Render);
    if (files->type == MSK) {
        files->Final_Render =
            SDL_CreateRGBSurface(0, files->Map_Mask->w, files->Map_Mask->h, 32, 0, 0, 0, 0);
        SDL_BlitSurface(files->Map_Mask, NULL, files->Final_Render, NULL);
    }
    else {
        //Unpalettize image to new surface for display
        files->Final_Render = Unpalettize_Image(files->Pal_Surface);
    }
    if (blend) {
        CPU_Blend(    files->Map_Mask,
                      files->Final_Render);
        SDL_to_OpenGl(files->Final_Render,
                     &files->Optimized_Render_Texture);
    }
    else {
        //Image2Texture(files->Final_Render,
        //             &files->Optimized_Render_Texture,
        //             &files->edit_image_window);
        SDL_to_OpenGl(files->Final_Render,
            &files->Optimized_Render_Texture);
    }
}

void Update_Palette2(SDL_Surface* surface, GLuint* texture, SDL_PixelFormat* pxlFMT) {
    SDL_Surface* Temp_Surface;
    //Force image to use the global palette instead of allowing SDL to use a copy
    SDL_SetPixelFormatPalette(surface->format, pxlFMT->palette);
    Temp_Surface = Unpalettize_Image(surface);
    SDL_to_OpenGl(Temp_Surface, texture);

    SDL_FreeSurface(Temp_Surface);
}

void CPU_Blend(SDL_Surface* msk_surface, SDL_Surface* img_surface)
{
    int width  = msk_surface->w;
    int height = msk_surface->h;
    int pitch = msk_surface->pitch/4;

    Uint32 color_noAlpha = SDL_MapRGB(msk_surface->format,
                               255, 255, 255);
    Uint32 color_wAlpha = SDL_MapRGBA(img_surface->format,
                               255, 255, 255, 255);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int position = (pitch*i) + j;

            if (((Uint32*)msk_surface->pixels)[position] == color_noAlpha)
            {
                Uint32 pixel = ((Uint32*)img_surface->pixels)[position];
                uint8_t r, g, b, a;

                SDL_GetRGBA(pixel, img_surface->format, &r, &g, &b, &a);
                r = ((int)r + 255) / 2;
                g = ((int)g + 255) / 2;
                b = ((int)b + 255) / 2;
                a = ((int)a + 255) / 2;

                Uint32 color_wAlpha = SDL_MapRGBA(img_surface->format,
                                                  r, g, b, a);

                ((Uint32*)img_surface->pixels)[position] = color_wAlpha;
            }
        }
    }
}

bool bind_PAL_data(SDL_Surface* surface, struct image_data* img_data)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //load & gen texture
    glGenTextures(1, &img_data->PAL_data);
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_data);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (surface->pixels) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RED, GL_UNSIGNED_BYTE, surface->pixels);
        //TODO: control alignment of the image (auto aligned to 4-bytes) when converted to texture
        //GL_UNPACK_ALIGNMENT
        //TODO: control alignment of the image (auto aligned to 4-bytes) when read from texture
        //GL_PACK_ALIGNMENT
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        return true;
    }
    else {
        printf("surface.pixels image didn't load...\n");
        return false;
    }
}

void texture_paint(int x, int y, int value, unsigned int texture)
{
    int v = value * 255;
    uint8_t color[4] = { v, v, v };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 5, 5,
        GL_RED, GL_UNSIGNED_BYTE,
        color);
}