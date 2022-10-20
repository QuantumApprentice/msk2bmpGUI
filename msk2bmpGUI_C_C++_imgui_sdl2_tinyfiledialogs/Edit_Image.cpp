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

            ///*old code, moved to Palette_Update section*/
            ////Unpalettize image to new surface for display
            //SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);
            //My_Variables->F_Prop[counter].Final_Render
            //    = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);
        }
    }

    ///*old but working color cycling method*/
    //uint8_t pal_color = 255;
    //for (int x = 0; x < width; x++)
    //{
    //    for (int y = 0; y < height; y++)
    //    {
    //        pal_color = ((uint8_t*)My_Variables->F_Prop[counter].Pal_Surface->pixels)[pitch*y + x];
    //        if (pal_color >= 225)
    //        {
    //            Image_Color_Cycle(pal_color, x, y,
    //                               My_Variables->PaletteColors,
    //                               My_Variables->F_Prop[counter].Final_Render);
    //        }
    //    }
    //}

    //Converts unpalettized image to texture for display, sets window bool to true
    if (My_Variables->Palette_Update || image_edited) {
        Update_Palette(My_Variables, counter, false);

        //SDL_SetPaletteColors(My_Variables->F_Prop[counter].Pal_Surface->format->palette,
        //    &My_Variables->pxlFMT_FO_Pal->palette->colors[228], 228, 28);

        //Unpalettize image to new surface for display
        //SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);

        //My_Variables->F_Prop[counter].Final_Render
        //    = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);

        //Image2Texture(My_Variables->F_Prop[counter].Final_Render,
        //    &My_Variables->F_Prop[counter].Optimized_Render_Texture,
        //    &My_Variables->F_Prop[counter].edit_image_window);
    }
}

void Create_Map_Mask(variables* My_Variables, int counter)
{
    int width =  My_Variables->F_Prop[counter].image->w;
    int height = My_Variables->F_Prop[counter].image->h;

    if (My_Variables->F_Prop[counter].Map_Mask)
        { SDL_FreeSurface(My_Variables->F_Prop[counter].Map_Mask); }
    My_Variables->F_Prop[counter].Map_Mask = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

    if (My_Variables->F_Prop[counter].Map_Mask) {

        Uint32 color = SDL_MapRGBA(My_Variables->F_Prop[counter].Map_Mask->format,
                                   0, 0, 0, 0);

        SDL_Surface* MM_Surface = My_Variables->F_Prop[counter].Map_Mask;
        for (int i = 0; i < (width*height); i++)
        {
            ((Uint32*)MM_Surface->pixels)[i] = color; //rand() % 255;
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

//TODO: change input from My_Variables to F_Prop[]
void Edit_Map_Mask(variables* My_Variables, SDL_Event* event, int counter, ImVec2 Origin)
{
    int width =  My_Variables->F_Prop[counter].image->w;
    int height = My_Variables->F_Prop[counter].image->h;

    SDL_Surface* BB_Surface = My_Variables->F_Prop[counter].Map_Mask;
    Uint32 white = SDL_MapRGB(My_Variables->F_Prop[counter].Map_Mask->format,
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


    //if (My_Variables->Palette_Update) {
    //    SDL_SetPaletteColors(My_Variables->F_Prop[counter].Pal_Surface->format->palette,
    //        My_Variables->PaletteColors, 0, 256);
    //}

    if (ImGui::IsMouseDown(event->button.button)) {

        if ((0 <= x && x <= width) && (0 <= y && y <= height)) {

            SDL_FillRect(My_Variables->F_Prop[counter].Map_Mask, &rect, white);
            My_Variables->Palette_Update = true;


            ///*TODO: This stuff didn't work, delete it when done*/
            //for (int i = 0; i < 4; i++)
            //{
            ///*TODO: two problems with this method:
            ///       pixel addressing skips by 8 pixels at a time,
            ///       and SDL_FillRect() doesn't work*/
            //    uint8_t* where_i_want_to_draw = 
            //                &((uint8_t*)BB_Surface->pixels)[pitch*y + x/8 + i];
            //    ((uint8_t*)where_i_want_to_draw)[0] = 255;
            //}
            ///*OpenGl stuff didn't work either :*/
            //opengl_stuff();
        }
    }

    if (My_Variables->Palette_Update) {
        ///re-copy Pal_Surface to Final_Render each time to allow 
        ///transparency through the mask surface painting
        Update_Palette(My_Variables, counter, true);


        //SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);
        //My_Variables->F_Prop[counter].Final_Render =
        //    Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);
        /////

        //CPU_Blend(My_Variables->F_Prop[counter].Map_Mask,
        //    My_Variables->F_Prop[counter].Final_Render);

        //SDL_to_OpenGl(My_Variables->F_Prop[counter].Final_Render,
        //    &My_Variables->F_Prop[counter].Optimized_Render_Texture);
    }
}

void Update_Palette(variables* My_Variables, int counter, bool blend) {
        //Unpalettize image to new surface for display
        SDL_FreeSurface(My_Variables->F_Prop[counter].Final_Render);

        My_Variables->F_Prop[counter].Final_Render
            = Unpalettize_Image(My_Variables->F_Prop[counter].Pal_Surface);

        if (blend) {
            CPU_Blend(My_Variables->F_Prop[counter].Map_Mask,
                My_Variables->F_Prop[counter].Final_Render);
            SDL_to_OpenGl(My_Variables->F_Prop[counter].Final_Render,
                &My_Variables->F_Prop[counter].Optimized_Render_Texture);
        }
        else {
            Image2Texture(My_Variables->F_Prop[counter].Final_Render,
                &My_Variables->F_Prop[counter].Optimized_Render_Texture,
                &My_Variables->F_Prop[counter].edit_image_window);
        }
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

//void opengl_stuff()
//{
//    ////SDL_PixelFormat* pxlFMT_UnPal;
//            ////pxlFMT_UnPal = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
//            ////temp = SDL_ConvertSurface(BB_Surface, pxlFMT_UnPal, 0);
//
//            //int bpp = 0;
//            //Uint32 Rmask, Gmask, Bmask, Amask;
//            //SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_ABGR8888, &bpp,
//            //    &Rmask, &Gmask, &Bmask, &Amask);
//            //SDL_Surface* temp;
//            //temp = SDL_CreateRGBSurfaceWithFormatFrom(BB_Surface->pixels, width, height, 32, bpp, SDL_PIXELFORMAT_RGBA8888);
//            //SDL_SetSurfaceAlphaMod(temp, 128);
//            //SDL_SetSurfaceBlendMode(temp, SDL_BLENDMODE_BLEND);
//            //SDL_SetSurfaceBlendMode(My_Variables->F_Prop[counter].Final_Render, SDL_BLENDMODE_BLEND);
//
//
//
//            //if (!temp) { printf(SDL_GetError()); }
//
//            //SDL_RenderCopy()
//            //SDL_SetRenderTarget()
//            //SDL_BlitSurface(                  temp,         NULL,
//            //    My_Variables->F_Prop[counter].Final_Render, NULL);
//
//            //SDL_BlitSurface(My_Variables->F_Prop[counter].Map_Mask,     NULL,
//            //                My_Variables->F_Prop[counter].Final_Render, NULL);
//
//            //Image2Texture(My_Variables->F_Prop[counter].Final_Render,
//            //    &My_Variables->F_Prop[counter].Optimized_Mask_Texture,
//            //    &My_Variables->F_Prop[counter].edit_image_window);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    //glBlendEquation() ??
//    //glActiveTexture(             My_Variables->F_Prop[counter].Optimized_Mask_Texture  );
//    //glBindTexture(GL_TEXTURE_2D, My_Variables->F_Prop[counter].Optimized_Render_Texture);
//
//    /////////////////////////////////////////////////////////////////////////////////
//    //trying out a shader to see if this is how to combine textures these days
//    const char *vertexShaderSource = "#version 330 core\n"
//        "layout (location = 0) in vec3 aPos;\n"
//        "void main()\n"
//        "{\n"
//        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
//        "}\0";
//    const char *fragmentShaderSource = "#version 330 core\n"
//        "out vec4 FragColor;\n"
//        "uniform sampler2D Texture;\n"
//        "vec2 TexCoord;\n"
//        "void main()\n"
//        "{\n"
//        "   TexCoord = vec2(0.0, 1.0);\n"
//        "   FragColor = texture(Texture, TexCoord);\n"
//        "}\n\0";
//    unsigned int fragmentShader;
//    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//    glCompileShader(fragmentShader);
//    int  success;
//    char infoLog[512];
//
//    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
//    }
//
//    unsigned int shaderProgram;
//    shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//
//    glGetShaderiv(shaderProgram, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(shaderProgram, 512, NULL, infoLog);
//        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
//    }
//
//    glUseProgram(shaderProgram);
//    glDeleteShader(fragmentShader);
//    /////////////////////////////////////////////////////////////////////////////////
//
//    glEnable(GL_TEXTURE_2D);
//    glBindTexture(GL_TEXTURE_2D, My_Variables->F_Prop[counter].Optimized_Render_Texture);
//
//    glTexImage2D(GL_TEXTURE_2D,
//        0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
//        My_Variables->F_Prop[counter].Final_Render->pixels);
//}