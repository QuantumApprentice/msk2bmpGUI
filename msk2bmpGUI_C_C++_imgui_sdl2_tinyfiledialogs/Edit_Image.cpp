#include <stdio.h>

#include "Edit_Image.h"
#include "imgui-docking/imgui.h"
#include "FRM_Animate.h"
#include <stdlib.h>

void Edit_Image(variables* My_Variables, int counter, SDL_Event* event) {
    //ImDrawList *Draw_List = ImGui::GetWindowDrawList();

    ImVec2 Origin = ImGui::GetItemRectMin();
    //ImVec2 Top_Left = Origin;
    int width  = My_Variables->F_Prop[counter].image->w;
    int height = My_Variables->F_Prop[counter].image->h;
    int size = width * height;
    //int pitch = My_Variables->F_Prop[counter].image->pitch;
    int pitch = (My_Variables->F_Prop[counter].Pal_Surface->pitch);
    bool image_edited = false;

    //ImVec2 Bottom_Right = { width + Origin.x, height + Origin.y };
    if (ImGui::IsMouseDown(event->button.button)) {
        image_edited = true;
        float x, y;
        x = ImGui::GetMousePos().x - Origin.x;
        y = ImGui::GetMousePos().y - Origin.y;

        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.h = 10;
        rect.w = 10;

        if ((0 <= x && x <= width) && (0 <= y && y <= height)) {

            SDL_FillRect(My_Variables->F_Prop[counter].Pal_Surface, &rect, My_Variables->Color_Pick);

            //TODO: maybe pass the dithering choice through?

            //Unpalettize image to new surface for display
            SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);
            My_Variables->F_Prop[counter].Final_Render
                = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);
        }
    }

    uint8_t pal_color = 255;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            pal_color = ((uint8_t*)My_Variables->F_Prop[counter].Pal_Surface->pixels)[pitch*y + x];
            if (pal_color >= 225)
            {
                Image_Color_Cycle2(pal_color, x, y,
                                   My_Variables->PaletteColors,
                                   My_Variables->F_Prop[counter].Final_Render);
            }
        }
    }

    //for (int i = 0; i < size; i++)
    //{
    //    if (((uint8_t*)My_Variables->F_Prop[counter].Pal_Surface->pixels)[i] >= 225)
    //    {
    //        Image_Color_Cycle(
    //            My_Variables->F_Prop[counter].Pal_Surface, i,
    //            My_Variables->PaletteColors,
    //            My_Variables->F_Prop[counter].Final_Render);
    //    }
    //}
    //Converts unpalettized image to texture for display, sets window bool to true

    if (My_Variables->Palette_Update || image_edited) {

        Image2Texture(My_Variables->F_Prop[counter].Final_Render,
            &My_Variables->F_Prop[counter].Optimized_Render_Texture,
            &My_Variables->F_Prop[counter].edit_image_window);
    }
}

void Create_Map_Mask(variables* My_Variables, SDL_Event* event, int counter)
{
    int width =  My_Variables->F_Prop[counter].image->w;
    int height = My_Variables->F_Prop[counter].image->h;

    if (My_Variables->F_Prop[counter].Map_Mask)
        { SDL_FreeSurface(My_Variables->F_Prop[counter].Map_Mask); }
    //My_Variables->F_Prop[counter].Map_Mask = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
    My_Variables->F_Prop[counter].Map_Mask = SDL_CreateRGBSurfaceWithFormat(
                                                SDL_SWSURFACE,
                                                width, height, 1,
                                                SDL_PIXELFORMAT_INDEX1MSB);
    //SDL_SetSurfaceBlendMode(My_Variables->F_Prop[counter].Map_Mask, SDL_BLENDMODE_ADD);

    if (My_Variables->F_Prop[counter].Map_Mask) {

        SDL_Surface* MM_Surface = My_Variables->F_Prop[counter].Map_Mask;
        for (int i = 0; i < (width*height/8); i++)
        {
            ((uint8_t*)MM_Surface->pixels)[i] = rand() % 255;
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

        Image2Texture(My_Variables->F_Prop[counter].Map_Mask,
            &My_Variables->F_Prop[counter].Optimized_Mask_Texture,
            &My_Variables->F_Prop[counter].edit_image_window);
    }
    else {
        printf("Can't allocate surface for some reason...");
    }
}

void Edit_Map_Mask(variables* My_Variables, SDL_Event* event, int counter, ImVec2 Origin)
{
    int width =  My_Variables->F_Prop[counter].image->w;
    int height = My_Variables->F_Prop[counter].image->h;

    SDL_Surface* BB_Surface = My_Variables->F_Prop[counter].Map_Mask;

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
            //printf("%f, %f\n", x, y);

            for (int i = 0; i < 4; i++)
            {
                uint8_t* where_i_want_to_draw = 
                            &((uint8_t*)BB_Surface->pixels)[pitch*y + x/8 + i];
                //SDL_FillRect(My_Variables->F_Prop[counter].Map_Mask, &rect, 255);
                ((uint8_t*)where_i_want_to_draw)[0] = 255;
            }
            SDL_BlitSurface(My_Variables->F_Prop[counter].Map_Mask, NULL,
                My_Variables->F_Prop[counter].Final_Render, NULL);
            Image2Texture(My_Variables->F_Prop[counter].Final_Render,
                &My_Variables->F_Prop[counter].Optimized_Mask_Texture,
                &My_Variables->F_Prop[counter].edit_image_window);
        }
    }
}