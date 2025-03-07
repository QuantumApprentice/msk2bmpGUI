// If you feel like dropping a dollar on me :)
// https://www.subscribestar.com/quantumapprentice

#include <stdio.h>
#include <stdlib.h>

#include "Edit_Image.h"
#include "display_FRM_OpenGL.h"
#include "Load_Files.h"
#include "Zoom_Pan.h"
#include "imgui_internal.h"
#include "ImGui_Warning.h"


void surface_paint(variables* My_Variables, image_data* edit_data, Surface* edit_srfc, float x, float y);

//displays edit_data->render_texture in window
//size and position stored in edit_data
//uv_min/uv_max/tint all stored in My_Variables
//TODO: check this against image_render()
//returns total position of image in desktop
ImVec2 display_img_ImGUI(variables* My_Variables, image_data* edit_data)
{
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec4 tint   = My_Variables->tint_col;
    //shortcuts
    int dir     = edit_data->display_orient_num;
    float scale = edit_data->scale;
    int width   = edit_data->ANM_bounding_box[dir].x2 - edit_data->ANM_bounding_box[dir].x1;
    int height  = edit_data->ANM_bounding_box[dir].y2 - edit_data->ANM_bounding_box[dir].y1;
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    ImVec2 img_pos = top_corner(edit_data->offset);

    //image I'm trying to pan with
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DrawList->AddImage(
        (ImTextureID)(uintptr_t)edit_data->render_texture,
        img_pos, bottom_corner(size, img_pos),
        uv_min, uv_max, ImGui::GetColorU32(tint));

    return img_pos;
}

//TODO: maybe pass the dithering choice through?
void Edit_Image(variables* My_Variables, ImVec2 img_pos,
                image_data* edit_data, ANM_Dir* edit_struct,
                Surface* edit_MSK_srfc, bool edit_MSK,
                bool Palette_Update, uint8_t* Color_Pick) {
    shader_info* shaders  = &My_Variables->shaders;
    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(edit_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);





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

    //handle frame display by orientation and number
    int num = edit_data->display_frame_num;
    int dir = edit_data->display_orient_num;




    Surface* edit_srfc;
    if (edit_data->type == FRM) {
        edit_srfc = edit_struct[dir].frame_data[num];
    } else {
        edit_srfc = edit_MSK_srfc;
    }



    if (!edit_data->ANM_dir) {
        ImGui::Text("No ANM_dir");
        return;
    }
    if (edit_data->ANM_dir[dir].frame_data == NULL && !edit_data->MSK_srfc) {
        ImGui::Text("No frame_data");
        return;
    }


    bool image_edited = false;
    if (ImGui::GetIO().MouseDown[0] && ImGui::IsWindowFocused()) {



        ImVec2 mouse_pos  = My_Variables->new_mouse_pos;
        int x_offset;
        int y_offset;
        if (edit_data->type == MSK) {
            x_offset = 0;
            y_offset = 0;
        } else {
            x_offset = edit_data->ANM_dir[dir].frame_box[num].x1 - edit_data->ANM_bounding_box[dir].x1;
            y_offset = edit_data->ANM_dir[dir].frame_box[num].y1 - edit_data->ANM_bounding_box[dir].y1;
        }

        float scale = edit_data->scale;
        ImVec2 img_offset = {
            (mouse_pos.x - img_pos.x)/scale,
            (mouse_pos.y - img_pos.y)/scale,
        };

        ImVec2 sub_image_offset = {
            (img_offset.x - x_offset),
            (img_offset.y - y_offset),
        };

        float x, y;
        x = (sub_image_offset.x);
        y = (sub_image_offset.y);

        if ((0 <= x && x < edit_srfc->w) && (0 <= y && y < edit_srfc->h)) {
            image_edited = true;

            Surface* srfc_ptr = edit_srfc;
            GLuint texture    = edit_data->FRM_texture;
            if (edit_MSK) {
                srfc_ptr = edit_MSK_srfc;
                texture  = edit_data->MSK_texture;
            }
            //paint MSK surface
            // texture_paint(My_Variables, edit_data, edit_srfc, edit_MSK);
            surface_paint(My_Variables, edit_data, srfc_ptr, x, y);
            //MSK & FRM are aligned to 1-byte
            SURFACE_to_texture(srfc_ptr, texture, srfc_ptr->w, srfc_ptr->h, 1);
        }
    }

    //TODO: zoom display and other info needs to be its own function call
    //      window_info()? window_stats()? image_stats()?
    ImGui::PushItemWidth(100);
    ImGui::DragFloat("##Zoom", &edit_data->scale, 0.1f, 0.0f, 10.0f, "Zoom: %%%.2fx", 0);
    ImGui::PopItemWidth();
    if (ImGui::Button("Reset Image")) {
        ClearSurface(edit_srfc);
        Surface* src;
        GLuint texture;
        if (edit_data->type == FRM) {
            src     = edit_data->ANM_dir[dir].frame_data[num];
            texture = edit_data->FRM_texture;
        }
        if (edit_data->type == MSK) {
            src     = edit_data->MSK_srfc;
            texture = edit_data->MSK_texture;
        }
        memcpy(edit_srfc->pxls, src->pxls, src->w*src->h);


        // BlitSurface(edit_data->FRM_dir[dir].frame_data[num])
        SURFACE_to_texture(edit_srfc, texture,
                            edit_srfc->w, edit_srfc->h, 1);
    }

    //Converts unpalettized image to texture for display
    //TODO:
    //***this no longer works as described above
    //      now it takes 3 textures (MSK, PAL, FRM)
    //      and draws them all onto edit_data.framebuffer
    //      also it only does this in draw_PAL_to_framebuffer()
    //      which also needs to be renamed
    if (Palette_Update || image_edited) {
        if (edit_data->type != MSK) {
            animate_SURFACE_to_sub_texture(
                shaders->palette,
                shaders->render_FRM_shader,
                shaders->giant_triangle,
                edit_data, edit_srfc,
                My_Variables->CurrentTime_ms,
                My_Variables->Palette_Update
            );
        }

        //TODO: rename?
        //      this takes 3 textures and draws them into 1 framebuffer
        draw_PAL_to_framebuffer(shaders->palette,
                                shaders->render_PAL_shader,
                                &shaders->giant_triangle,
                                edit_data);
    }

}

//SDL_Surface* Create_MSK_SDL(SDL_Surface* image, GLuint* texture, bool* window)
//{
//    int width  = image->w;
//    int height = image->h;
//
//    //if (Map_Mask)
//    //    { SDL_FreeSurface(Map_Mask); }
//    SDL_Surface* Map_Mask = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
//
//    if (Map_Mask) {
//
//        Uint32 color = SDL_MapRGBA(Map_Mask->format,
//                                   0, 0, 0, 0);
//
//        for (int i = 0; i < (width*height); i++)
//        {
//            ((Uint32*)Map_Mask->pixels)[i] = color; //rand() % 255;
//            //uint8_t byte =
//            //    (rand() % 2 << 0) |
//            //    (rand() % 2 << 1) |
//            //    (rand() % 2 << 2) |
//            //    (rand() % 2 << 3) |
//            //    (rand() % 2 << 4) |
//            //    (rand() % 2 << 5) |
//            //    (rand() % 2 << 6) |
//            //    (rand() % 2 << 7);
//            //((uint8_t*)MM_Surface->pixels)[i] = byte;
//        }
//
//        Image2Texture(Map_Mask,
//            texture,
//            window);
//    }
//    else {
//        printf("Can't allocate surface for some reason...");
//    }
//    return Map_Mask;
//}

bool Create_MSK_OpenGL(image_data* img_data)
{
    int img_width  = img_data->width;
    int img_height = img_data->height;
    //int img_size = img_width * img_height;
    //uint8_t* data = (uint8_t*)calloc(1, img_size);

    //if (data) {
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
    //    img_data->MSK_data = data;
        return true;
    //}
    //else {
    //    printf("MSK image didn't load...\n");
    //    return false;
    //}
}

//TODO: DELETE!
//  change input from My_Variables to F_Prop[] (or maybe image_data* now)
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

//TODO: DELETE!
//bool blend == true will blend surfaces
//TODO: DELETE! left here for Map_Mask stuff, need to remove after mask stuff converted to openGL
//void Update_Palette_SDL(struct LF* files, bool blend) {
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

////TODO: DELETE! same as above
//void CPU_Blend_SDL(SDL_Surface* msk_surface, SDL_Surface* img_surface)
//{
//    int width  = msk_surface->w;
//    int height = msk_surface->h;
//    int pitch = msk_surface->pitch/4;
//
//    Uint32 color_noAlpha = SDL_MapRGB(msk_surface->format,
//                               255, 255, 255);
//    Uint32 color_wAlpha = SDL_MapRGBA(img_surface->format,
//                               255, 255, 255, 255);
//
//    for (int i = 0; i < height; i++)
//    {
//        for (int j = 0; j < width; j++)
//        {
//            int position = (pitch*i) + j;
//
//            if (((Uint32*)msk_surface->pixels)[position] == color_noAlpha)
//            {
//                Uint32 pixel = ((Uint32*)img_surface->pixels)[position];
//                uint8_t r, g, b, a;
//
//                SDL_GetRGBA(pixel, img_surface->format, &r, &g, &b, &a);
//                r = ((int)r + 255) / 2;
//                g = ((int)g + 255) / 2;
//                b = ((int)b + 255) / 2;
//                a = ((int)a + 255) / 2;
//
//                Uint32 color_wAlpha = SDL_MapRGBA(img_surface->format,
//                                                  r, g, b, a);
//
//                ((Uint32*)img_surface->pixels)[position] = color_wAlpha;
//            }
//        }
//    }
//}

//TODO: archive & DELETE
//      surface_paint() is used now
//void texture_paint(int x, int y, int brush_w, int brush_h, int value, unsigned int texture)
void texture_paint(variables* My_Variables, image_data* edit_data, Surface* edit_srfc, bool edit_MSK)
{
    int color_pick = My_Variables->Color_Pick;
    float brush_w  = My_Variables->brush_size.x;
    float brush_h  = My_Variables->brush_size.y;
    int brush_size = brush_h * brush_w;
    uint8_t* color = (uint8_t*)malloc(brush_size);
    memset(color, color_pick, brush_size);

    int orient = edit_data->display_orient_num;
    int frame  = edit_data->display_frame_num;

    int offset_x = edit_data->FRM_dir[orient].frame_data[frame]->Shift_Offset_x;
    int offset_y = edit_data->FRM_dir[orient].frame_data[frame]->Shift_Offset_y;


    float scale = edit_data->scale;
    int width   = edit_data->width;
    int height  = edit_data->height;
    ImVec2 img_size = ImVec2((float)(width * scale), (float)(height * scale));


    float x, y;
    x = (My_Variables->new_mouse_pos.x - top_corner(edit_data->offset).x)/scale;
    y = (My_Variables->new_mouse_pos.y - top_corner(edit_data->offset).y)/scale;

    GLuint* texture = NULL;
    if (edit_MSK) {
        texture = &edit_data->MSK_texture;
    }
    else {
        texture = &edit_data->PAL_texture;
    }

    if ((0 <= x && x <= img_size.x) && (0 <= y && y <= img_size.y)) {
        //clamp brush to edge when close enough
        if ((x + brush_w / 2) > width) {
            x = width - brush_w / 2;
        }
        if ((x - brush_w / 2) < 0) {
            x = brush_w /2;
        }
        if ((y + brush_h / 2) > height) {
            y = height - brush_h / 2;
        }
        if ((y - brush_h / 2) < 0) {
            y = brush_h / 2;
        }

        //TODO: switch to editing the both MSK & FRM surface directly
        //TODO: implement undo tree
        if (!edit_MSK) {
            x -= brush_w/2;
            y -= brush_h/2;
            x -= offset_x;
            y -= offset_y;
            Rect dst_rect = {(int)x, (int)y, (int)brush_w, (int)brush_h};
            PaintSurface(edit_srfc, dst_rect, My_Variables->Color_Pick);
        } else {
            //TODO: edit MSK surface here?
            //...no, I should move this check outside of this function
            // and just use this to edit both MSK and FRM as Surface*
        }

    //old code showing how to paint a brush sized square
    //onto an openGL bound texture
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, *texture);
        // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // glTexSubImage2D(GL_TEXTURE_2D, 0, x-(brush_w/2), y-(brush_h/2), brush_w, brush_h,
        //     GL_RED, GL_UNSIGNED_BYTE,
        //     color);
    }

    free(color);
}

//paint surfaces for both MSK and FRM items (possibly also PAL? or other 32bit surfaces?)
//TODO: need to have a brush shape in place of (or on top of) the mouse cursor when painting
//TODO: repack all the x&y variables into vectors of appropriate type (int/float)
void surface_paint(variables* My_Variables, image_data* edit_data, Surface* edit_srfc, float x, float y)
{


    int color_pick = My_Variables->Color_Pick;
    float brush_w  = My_Variables->brush_size.x;
    float brush_h  = My_Variables->brush_size.y;
    int brush_size = brush_h * brush_w;

    int dir = edit_data->display_orient_num;
    int num = edit_data->display_frame_num;

    int width = edit_srfc->w;
    int height = edit_srfc->h;

    //clamp brush size to within surface
    if (brush_w > width) {
        brush_w = width;
    }
    if (brush_h > height) {
        brush_h = height;
    }
    //clamp brush position to edge
    if ((x + brush_w / 2) > width) {
        x = width - brush_w / 2;
    }
    if ((x - brush_w / 2) < 0) {
        x = brush_w /2;
    }
    if ((y + brush_h / 2) > height) {
        y = height - brush_h / 2;
    }
    if ((y - brush_h / 2) < 0) {
        y = brush_h / 2;
    }

    //TODO: implement undo tree
    //further clamp the brush to prevent overflow
    //TODO: this is a lazy implementation that doesn't allow
    //      the brush to shrink in size as it goes over the edge
    //      or rather, to clip the brush so it doesn't paint off the edge
    x -= brush_w/2;
    y -= brush_h/2;

    Rect dst_rect = {(int)x, (int)y, (int)brush_w, (int)brush_h};
    PaintSurface(edit_srfc, dst_rect, color_pick);
}

void brush_size_handler(variables* My_Variables)
{
    ImGui::DragFloat("###width", &My_Variables->brush_size.x, 1.0f, 1.0f, FLT_MAX, "Brush Width: %.0f pixels");
    ImGui::SameLine();
    ImGui::Checkbox("Link", &My_Variables->link_brush_sizes);

    if (My_Variables->link_brush_sizes) {
        My_Variables->brush_size.y = My_Variables->brush_size.x;
    }
    ImGui::DragFloat("###height", &My_Variables->brush_size.y, 1.0f, 1.0f, FLT_MAX, "Brush Height: %.0f pixels");
}