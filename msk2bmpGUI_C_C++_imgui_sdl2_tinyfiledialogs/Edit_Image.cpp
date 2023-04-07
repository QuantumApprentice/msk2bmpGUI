#include <stdio.h>
#include <stdlib.h>

#include "Edit_Image.h"
#include "display_FRM_OpenGL.h"
#include "Load_Files.h"
#include "Zoom_Pan.h"
#include "imgui-docking/imgui_internal.h"

void Edit_Image(variables* My_Variables, LF* F_Prop, bool Palette_Update, uint8_t* Color_Pick) {
    //TODO: maybe pass the dithering choice through?

    image_data* edit_data = &F_Prop->edit_data;

    ////TODO: use a menu bar for the editor/previewer?
    //if (ImGui::BeginMenuBar()) {
    //    if (ImGui::BeginMenu("label")) {
    //        if (ImGui::MenuItem("Clear All Changes...")) {
    //            int texture_size = width * height;
    //            uint8_t* clear = (uint8_t*)malloc(texture_size);
    //            memset(clear, 0, texture_size);
    //            glBindTexture(GL_TEXTURE_2D, edit_data->PAL_texture);
    //            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    //            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
    //                width, height,
    //                0, GL_RED, GL_UNSIGNED_BYTE, clear);
    //            free(clear);
    //        }
    //        ImGui::EndMenu();
    //    }
    //    ImGui::EndMenuBar();
    //}

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(edit_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    //shortcuts
    float scale = edit_data->scale;
    int width = edit_data->width;
    int height = edit_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec4 tint   = My_Variables->tint_col;
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));


    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan with
    window->DrawList->AddImage(
        (ImTextureID)edit_data->render_texture,
        top_corner(edit_data), bottom_corner(size, top_corner(edit_data)),
        uv_min, uv_max, ImGui::GetColorU32(tint));

    bool image_edited = false;
    if (ImGui::GetIO().MouseDown[0] && ImGui::IsWindowFocused()) {
        image_edited = true;
        float x, y;
        x = My_Variables->new_mouse_pos.x - top_corner(edit_data).x;
        y = My_Variables->new_mouse_pos.y - top_corner(edit_data).y;

        if ((0 <= x && x <= size.x) && (0 <= y && y <= size.y)) {
            if (F_Prop->edit_MSK) {
                texture_paint(x / scale, y / scale, 10, 10, *Color_Pick, edit_data->MSK_texture);
            }
            else {
                texture_paint(x / scale, y / scale, 10, 10, *Color_Pick, edit_data->PAL_texture);
            }
        }
    }

    //Converts unpalettized image to texture for display
    if (Palette_Update || image_edited) {
        draw_PAL_to_framebuffer(My_Variables->palette,
            //&My_Variables->render_FRM_shader,
            &My_Variables->render_PAL_shader,
            &My_Variables->giant_triangle,
            edit_data);
    }

}

void Edit_MSK_OpenGL(variables* My_Variables, LF* F_Prop)
{

}

SDL_Surface* Create_MSK_SDL(SDL_Surface* image, GLuint* texture, bool* window)
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

bool Create_MSK_OpenGL(image_data* img_data)
{
    int img_width  = img_data->width;
    int img_height = img_data->height;
    int img_size = img_width * img_height;
    uint8_t* data = (uint8_t*)calloc(1, img_size);

    if (data) {
        //load & gen texture
        glGenTextures(1, &img_data->MSK_texture);
        glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
        //texture settings
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //MSK's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //bind data to FRM_texture for display
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RED, GL_UNSIGNED_BYTE, data);

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        img_data->MSK_data = data;
        return true;
    }
    else {
        printf("MSK image didn't load...\n");
        return false;
    }
}

//TODO: change input from My_Variables to F_Prop[] (or maybe image_data* now)
//TODO: also change to work with openGL
//void Edit_MSK_SDL(LF* F_Prop, bool* Palette_Update, ImVec2 Origin)
//{
//    int width  = F_Prop->IMG_Surface->w;
//    int height = F_Prop->IMG_Surface->h;
//
//    SDL_Surface* BB_Surface = F_Prop->Map_Mask;
//    Uint32 white = SDL_MapRGB(F_Prop->Map_Mask->format,
//                              255, 255, 255);
//
//    ImVec2 MousePos = ImGui::GetMousePos();
//    int x = (int)(MousePos.x - Origin.x);
//    int y = (int)(MousePos.y - Origin.y);
//    int pitch = BB_Surface->pitch;
//
//    SDL_Rect rect;
//    rect.x = x;
//    rect.y = y;
//    rect.h = 10;
//    rect.w = 10;
//
//    if (ImGui::GetIO().MouseDown[0]) {
//
//        if ((0 <= x && x <= width) && (0 <= y && y <= height)) {
//
//            SDL_FillRect(F_Prop->Map_Mask, &rect, white);
//            *Palette_Update = true;
//
//
//            ///*TODO: This stuff didn't work, delete it when done                   */
//            ///*TODO: two problems with using binary surface:
//            ///       pixel addressing skips by 8 pixels at a time,
//            ///       and SDL_FillRect() doesn't work                               */
//            //for (int i = 0; i < 4; i++)
//            //{
//            //    uint8_t* where_i_want_to_draw = 
//            //                &((uint8_t*)BB_Surface->pixels)[pitch*y + x/8 + i];
//            //    ((uint8_t*)where_i_want_to_draw)[0] = 255;
//            //}
//            ///*OpenGl stuff didn't work either :                                   */
//            //opengl_stuff();
//        }
//    }
//
//    if (*Palette_Update && (F_Prop->type == FRM)) {
//    ///re-copy Pal_Surface to Final_Render each time to allow 
//    ///transparency through the mask surface painting
//        Update_Palette(F_Prop, true);
//    }
//    else if (*Palette_Update && (F_Prop->type == MSK)) {
//        Update_Palette(F_Prop, true);
//    }
//}

//bool blend == true will blend surfaces
//TODO: remove! left here for Map_Mask stuff, need to remove after mask stuff converted to openGL
//void Update_Palette(struct LF* files, bool blend) {
//    SDL_FreeSurface(files->Final_Render);
//    if (files->type == MSK) {
//        files->Final_Render =
//            SDL_CreateRGBSurface(0, files->Map_Mask->w, files->Map_Mask->h, 32, 0, 0, 0, 0);
//        SDL_BlitSurface(files->Map_Mask, NULL, files->Final_Render, NULL);
//    }
//    else {
//        //Unpalettize image to new surface for display
//        files->Final_Render = Unpalettize_Image(files->PAL_Surface);
//    }
//    if (blend) {
//        CPU_Blend(    files->Map_Mask,
//                      files->Final_Render);
//        SDL_to_OpenGl(files->Final_Render,
//                     &files->Optimized_Render_Texture);
//    }
//    else {
//        //Image2Texture(files->Final_Render,
//        //             &files->Optimized_Render_Texture,
//        //             &files->edit_image_window);
//        SDL_to_OpenGl(files->Final_Render,
//            &files->Optimized_Render_Texture);
//    }
//}
////TODO: remove! same as above
//void Update_Palette2(SDL_Surface* surface, GLuint* texture, SDL_PixelFormat* pxlFMT) {
//    SDL_Surface* Temp_Surface;
//    //Force image to use the global palette instead of allowing SDL to use a copy
//    SDL_SetPixelFormatPalette(surface->format, pxlFMT->palette);
//    Temp_Surface = Unpalettize_Image(surface);
//    SDL_to_OpenGl(Temp_Surface, texture);
//
//    SDL_FreeSurface(Temp_Surface);
//}

////TODO: remove! same as above
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

void texture_paint(int x, int y, int brush_w, int brush_h, int value, unsigned int texture)
{
    int v = value;
    int size = brush_h * brush_w;
    uint8_t* color = (uint8_t*)malloc(size);
    memset(color, value, size);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x-(brush_w/2), y-(brush_h/2), brush_w, brush_h,
        GL_RED, GL_UNSIGNED_BYTE,
        color);

    free(color);
}