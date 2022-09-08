#include <stdio.h>

#include "Edit_Image.h"
#include "imgui-docking/imgui.h"
#include "FRM_Animate.h"

void Edit_Image(variables* My_Variables, int counter, SDL_Event* event) {
    //ImDrawList *Draw_List = ImGui::GetWindowDrawList();

    ImVec2 Origin = ImGui::GetItemRectMin();
    //ImVec2 Top_Left = Origin;
    int width = My_Variables->F_Prop[counter].image->w;
    int height = My_Variables->F_Prop[counter].image->h;
    int size = My_Variables->F_Prop[counter].Pal_Surface->w * My_Variables->F_Prop[counter].Pal_Surface->h;

    //ImVec2 Bottom_Right = { width + Origin.x, height + Origin.y };
    //Draw_List->AddRectFilled(Top_Left, Bottom_Right, 0x5f0000ff, 0, ImDrawFlags_Closed);
    if (ImGui::IsMouseDown(event->button.button)) {
        float x, y;
        x = ImGui::GetMousePos().x - Origin.x;
        y = ImGui::GetMousePos().y - Origin.y;
        int pitch = My_Variables->F_Prop[counter].Pal_Surface->pitch;

        if ((0 <= x && x <= width) && (0 <= y && y <= height)) {
            printf("%f, %f\n", x, y);

            for (int i = 0; i < 4; i++)
            {
                ((uint8_t*)My_Variables->F_Prop[counter].Pal_Surface->pixels)[(pitch*(int)y) + (int)x + i]
                    = My_Variables->Color_Pick;
            }
            //TODO: maybe pass the dithering choice through?
            //Unpalettize image to new surface for display
            SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);
            My_Variables->F_Prop[counter].Final_Render
                = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);
        }
    }
    for (int i = 0; i < size; i++)
    {
        if (((uint8_t*)My_Variables->F_Prop[counter].Pal_Surface->pixels)[i] >= 229) {
            Image_Color_Cycle(
                My_Variables->F_Prop[counter].Pal_Surface, i,
                My_Variables->PaletteColors,
                My_Variables->F_Prop[counter].Final_Render);
        }
    }
    //Converts unpalettized image to texture for display, sets window bool to true
    Image2Texture(My_Variables->F_Prop[counter].Final_Render,
        &My_Variables->F_Prop[counter].Optimized_Render_Texture,
        &My_Variables->F_Prop[counter].edit_image_window);


}

void Map_Mask(variables* My_Variables, SDL_Event* event, int counter)
{
    //SDL_FreeSurface(BB_Surface);
    //SDL_FreeSurface(My_Variables->F_Prop[counter].Map_Mask);
    int* width = &My_Variables->F_Prop[counter].image->w;
    int* height = &My_Variables->F_Prop[counter].image->h;
    My_Variables->F_Prop[counter].Map_Mask = SDL_CreateRGBSurface(0, *width, *height, 32, 0, 0, 0, 255);
    SDL_Surface* BB_Surface = My_Variables->F_Prop[counter].Map_Mask;
    for (int i = 0; i < (*width)*(*height); i++)
    {
        ((ImVec4*)BB_Surface->pixels)[i] = ImVec4(128, 128, 128, 255);
    }
}

void Edit_Map_Mask(variables* My_Variables, SDL_Event* event, int counter)
{
    SDL_FreeSurface(My_Variables->F_Prop[counter].Map_Mask);
    int* width = &My_Variables->F_Prop[counter].image->w;
    int* height = &My_Variables->F_Prop[counter].image->h;
    ImVec2 Origin = ImGui::GetItemRectMin();
    ImVec2 Bottom_Right = { *width + Origin.x, *height + Origin.y };
    SDL_Surface* BB_Surface = My_Variables->F_Prop[counter].Map_Mask;

    float x, y;
    x = ImGui::GetMousePos().x - Origin.x;
    y = ImGui::GetMousePos().y - Origin.y;
    int pitch = BB_Surface->pitch;

    if (ImGui::IsMouseDown(event->button.button)) {

        if ((0 <= x && x <= *width) && (0 <= y && y <= *height)) {
            printf("%f, %f\n", x, y);

            for (int i = 0; i < 4; i++)
            {
                ((uint8_t*)BB_Surface->pixels)[(pitch*(int)y) + (int)x + i] = My_Variables->Color_Pick;
            }

        }
    }

}