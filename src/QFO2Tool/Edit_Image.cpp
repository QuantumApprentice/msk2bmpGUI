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


void surface_paint(variables* My_Variables, Surface* edit_srfc, float x, float y);

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
    ANM_Dir* anm_dir = edit_data->ANM_dir;

    //TODO: maybe set display_frame_num to 0 for every edit image?
    if (num > anm_dir[dir].num_frames) {
        num = anm_dir[dir].num_frames-1;
    }

    Surface* edit_srfc;
    if (edit_MSK == false) {
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
        if (edit_MSK) {
            x = img_offset.x;
            y = img_offset.y;
        } else {
            x = (sub_image_offset.x);
            y = (sub_image_offset.y);
        }

        if ((0 <= x && x < edit_srfc->w) && (0 <= y && y < edit_srfc->h)) {

            image_edited = true;

            Surface* srfc_ptr = edit_srfc;
            GLuint texture    = edit_data->FRM_texture;
            if (edit_MSK) {
                srfc_ptr = edit_MSK_srfc;
                texture  = edit_data->MSK_texture;
            }
            //paint MSK surface
            surface_paint(My_Variables, srfc_ptr, x, y);
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
        Surface* src   = edit_data->ANM_dir[dir].frame_data[num];
        GLuint texture = edit_data->FRM_texture;
        if (edit_MSK) {
            src        = edit_data->MSK_srfc;
            texture    = edit_data->MSK_texture;
        }
        memcpy(edit_srfc->pxls, src->pxls, src->w*src->h);

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
        // if (edit_MSK != true) {
        if (edit_data->ANM_dir[dir].frame_data) {
            animate_SURFACE_to_sub_texture(
                shaders->palette,
                shaders->render_FRM_shader,
                shaders->giant_triangle,
                edit_data, edit_struct[dir].frame_data[num],
                My_Variables->CurrentTime_ms
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

//paint surfaces for both MSK and FRM items (possibly also PAL? or other 32bit surfaces?)
//TODO: need to have a brush shape in place of (or on top of) the mouse cursor when painting
//TODO: repack all the x&y variables into vectors of appropriate type (int/float)
void surface_paint(variables* My_Variables, Surface* dst, float x, float y)
{
    int color_pick = My_Variables->Color_Pick;
    float brush_w  = My_Variables->brush_size.x;
    float brush_h  = My_Variables->brush_size.y;
    int brush_size = brush_h * brush_w;

    int w = dst->w;
    int h = dst->h;

    //clamp brush size to within surface
    if (brush_w > w) {
        brush_w = w;
    }
    if (brush_h > h) {
        brush_h = h;
    }
    //clamp brush position to edge
    if ((x + brush_w / 2) > w) {
        x = w - brush_w / 2;
    }
    if ((x - brush_w / 2) < 0) {
        x = brush_w /2;
    }
    if ((y + brush_h / 2) > h) {
        y = h - brush_h / 2;
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
    PaintSurface(dst, dst_rect, color_pick);
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