#include "imgui_internal.h"
#include "display_FRM_OpenGL.h"
#include "Preview_Image.h"
#include "Zoom_Pan.h"

void show_image_stats_FRM_SURFACE(image_data* img_data, ImFont* font);

void preview_FRM_SURFACE(variables* My_Variables, struct image_data* img_data, bool show_stats)
{
    ImVec2 top_of_window = ImGui::GetCursorPos();

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    //redraws FRM to framebuffer every time the palette update timer is true or animates
    shader_info* shaders = &My_Variables->shaders;
    if (!img_data->ANM_dir) {
        ImGui::Text("No FRM_dir");
        return;
    }
    if (img_data->ANM_dir[img_data->display_orient_num].frame_data == NULL) {
        ImGui::Text("No frame_data");
        return;
    }

    Surface* sfc = img_data->ANM_dir[img_data->display_orient_num].frame_data[img_data->display_frame_num];
    animate_SURFACE_to_sub_texture(
        shaders->palette,
        shaders->render_FRM_shader,
        shaders->giant_triangle,
        img_data, sfc,
        My_Variables->CurrentTime_ms,
        My_Variables->Palette_Update
    );

    draw_texture_to_framebuffer(
        shaders->palette, shaders->render_FRM_shader, &shaders->giant_triangle,
        img_data->framebuffer, img_data->FRM_texture, img_data->width, img_data->height
    );

    draw_PAL_to_framebuffer(
        shaders->palette,
        shaders->render_FRM_shader,
        &shaders->giant_triangle,
        img_data
    );

    //handle frame display by orientation and number
    int orient  = img_data->display_orient_num;

    float scale = img_data->scale;
    int width   = img_data->ANM_bounding_box[orient].x2 - img_data->ANM_bounding_box[orient].x1;
    int height  = img_data->ANM_bounding_box[orient].y2 - img_data->ANM_bounding_box[orient].y1;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));


    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    //TODO: change top_corner() for img_pos passed in from outside
    window->DrawList->AddImage(
        (ImTextureID)(uintptr_t)img_data->render_texture,
        top_corner(img_data->offset), bottom_corner(size, top_corner(img_data->offset)),
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

    //show FRM stats over FRM
    ImGui::SetCursorPos(top_of_window);
    if (show_stats) {
        // show_image_stats_FRM(img_data, My_Variables->Font);
        show_image_stats_FRM_SURFACE(img_data, My_Variables->Font);
    }
}

// //TODO: delete Preview_FRM_Image() (replaced with preview_FRM_SURFACE())
// void Preview_FRM_Image(variables* My_Variables, struct image_data* img_data, bool show_stats)
// {
//     ImVec2 top_of_window = ImGui::GetCursorPos();

//     //handle zoom and panning for the image, plus update image position every frame
//     zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

//     //new openGL version of pallete cycling
//     //redraws FRM to framebuffer every time the palette update timer is true or animates
//     shader_info* shaders = &My_Variables->shaders;
//     if (!img_data->FRM_dir) {
//         ImGui::Text("No FRM_dir");
//         return;
//     }
//     if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL) {
//         ImGui::Text("No frame_data");
//         return;
//     }
//     animate_FRM_to_framebuff(shaders->palette,
//         shaders->render_FRM_shader,
//         shaders->giant_triangle,
//         img_data,
//         My_Variables->CurrentTime_ms,
//         My_Variables->Palette_Update);


//     //handle frame display by orientation and number
//     int orient  = img_data->display_orient_num;
//     // int frame   = img_data->display_frame_num;
//     // int max_frm = img_data->FRM_dir[orient].num_frames;

//     float scale = img_data->scale;
//     int width   = img_data->FRM_bounding_box[orient].x2 - img_data->FRM_bounding_box[orient].x1;
//     int height  = img_data->FRM_bounding_box[orient].y2 - img_data->FRM_bounding_box[orient].y1;
//     ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
//     ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
//     ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));


//     ImGuiWindow* window = ImGui::GetCurrentWindow();
//     //image I'm trying to pan and zoom with
//     window->DrawList->AddImage(
//         (ImTextureID)(uintptr_t)img_data->render_texture,
//         top_corner(img_data->offset), bottom_corner(size, top_corner(img_data->offset)),
//         uv_min, uv_max,
//         ImGui::GetColorU32(My_Variables->tint_col));

//     //TODO: need to figure out how I'm going to handle scrolling on large images
//     ImGui::Dummy(size);

//     //show FRM stats over FRM
//     ImGui::SetCursorPos(top_of_window);
//     if (show_stats) {
//         show_image_stats_FRM(img_data, My_Variables->Font);
//     }
// }

void Preview_MSK_Image(variables* My_Variables, struct image_data* img_data, bool show_stats)
{
    ImVec2 top_of_window = ImGui::GetCursorPos();

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info* shaders = &My_Variables->shaders;
    if (img_data->MSK_data == NULL) {
        ImGui::Text("No Image Data");
        return;
    }

    //handle frame display by orientation and number
    float scale   = img_data->scale;
    int width     = img_data->width;
    int height    = img_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size   = ImVec2((float)(width * scale), (float)(height * scale));


    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    //TODO: change top_corner() for img_pos passed in from outside
    window->DrawList->AddImage(
        (ImTextureID)(uintptr_t)img_data->render_texture,
        top_corner(img_data->offset), bottom_corner(size, top_corner(img_data->offset)),
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

    //show MSK stats over MSK image
    ImGui::SetCursorPos(top_of_window);
    if (show_stats) {
        show_image_stats_MSK(img_data, My_Variables->Font);
    }
}

#define tile_grid_w         (128.0)
#define tile_grid_h         (96.0)

//TODO: refactor this
void Preview_Image(variables* My_Variables, struct image_data* img_data, bool show_stats)
{
    ImVec2 top_of_window = ImGui::GetCursorPos();

    //handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info* shaders = &My_Variables->shaders;

    //handle frame display by orientation and number
    float scale = img_data->scale;
    int width = img_data->width;
    int height = img_data->height;
    ImVec2 uv_min = My_Variables->uv_min;      // (0.0f,0.0f)
    ImVec2 uv_max = My_Variables->uv_max;      // (1.0f,1.0f)
    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    if (img_data->ANM_dir[img_data->display_orient_num].frame_data == NULL) {
        ImGui::Text("No Image Data");
        return;
    }
    animate_OTHER_to_framebuff(
        My_Variables->shaders.render_OTHER_shader,
        &My_Variables->shaders.giant_triangle,
        img_data,
        My_Variables->CurrentTime_ms);

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //image I'm trying to pan and zoom with
    //TODO: change top_corner() for img_pos passed in from outside
    window->DrawList->AddImage(
        (ImTextureID)(uintptr_t)img_data->render_texture,
        top_corner(img_data->offset), bottom_corner(size, top_corner(img_data->offset)),
        uv_min, uv_max,
        ImGui::GetColorU32(My_Variables->tint_col));

    ///////////////////////////////////////////////////////////////////////

    ImVec2 tile_corner, tile_bottom;
    tile_corner = ImGui::GetWindowPos();
    tile_bottom.x = tile_corner.x + ImGui::GetWindowSize().x;
    tile_bottom.y = tile_corner.y + ImGui::GetWindowSize().y;
    float tile_w = (tile_bottom.x - tile_corner.x) / tile_grid_w / scale;
    float tile_h = (tile_bottom.y - tile_corner.y) / tile_grid_h / scale;


    ImVec2 tile_min = { 0,0 };
    ImVec2 tile_max = { tile_w, tile_h };
    //window->DrawList->AddImage(
    //    (ImTextureID)My_Variables->tile_texture_prev,
    //    tile_corner, tile_bottom, tile_min, tile_max);
    ///////////////////////////////////////////////////////////////////////



    //TODO: need to figure out how I'm going to handle scrolling on large images
    ImGui::Dummy(size);

    //show image stats over image
    ImGui::SetCursorPos(top_of_window);
    if (show_stats) {
        show_image_stats_ANM(img_data, My_Variables->Font);
    }

}


//TODO: replace this with something that doesn't suck
void show_image_stats_FRM_SURFACE(image_data* img_data, ImFont* font)
{
    int num = img_data->display_frame_num;
    int dir = img_data->display_orient_num;
    if (!img_data->ANM_dir[dir].frame_data) {
        ImGui::Text("Like it says, No Image Data");
        return;
    }
    char buff[256];

    ImGui::PushFont(font);
    snprintf(buff, 256, "framerate: %d",       img_data->FRM_hdr->FPS);
    ImGui::Text(buff);

    snprintf(buff, 256, "Shift_Orient_x: %d",  img_data->FRM_hdr->Shift_Orient_x[dir]);
    ImGui::Text(buff);
    snprintf(buff, 256, "Shift_Orient_y: %d",  img_data->FRM_hdr->Shift_Orient_y[dir]);
    ImGui::Text(buff);

    snprintf(buff, 256, "bounding_x1: %d\t",   img_data->ANM_dir[dir].frame_box[num].x1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "width: %d\t",         img_data->ANM_dir[dir].frame_data[num]->w);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "x_offset: %d",        img_data->ANM_dir[dir].frame_data[num]->x);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_x2: %d",     img_data->ANM_dir[dir].frame_box[num].x2);
    ImGui::Text(buff);

    snprintf(buff, 256, "bounding_y1: %d\t",   img_data->ANM_dir[dir].frame_box[num].y1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "height: %d\t",        img_data->ANM_dir[dir].frame_data[num]->h);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "y_offset: %d",        img_data->ANM_dir[dir].frame_data[num]->y);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_y2: %d",     img_data->ANM_dir[dir].frame_box[num].y2);
    ImGui::Text(buff);

    snprintf(buff, 256, "FRM_bounding_x1: %d", img_data->ANM_bounding_box[dir].x1);
    ImGui::Text(buff);
    snprintf(buff, 256, "FRM_bounding_x2: %d", img_data->ANM_bounding_box[dir].x2);
    ImGui::Text(buff);
    snprintf(buff, 256, "FRM_bounding_y1: %d", img_data->ANM_bounding_box[dir].y1);
    ImGui::Text(buff);
    snprintf(buff, 256, "FRM_bounding_y2: %d", img_data->ANM_bounding_box[dir].y2);
    ImGui::Text(buff);
    ImGui::PopFont();
}


// //TODO: delete this?
// void show_image_stats_FRM(image_data* img_data, ImFont* font)
// {
//     int num, dir, max;
//     num = img_data->display_frame_num;
//     dir = img_data->display_orient_num;
//     if (img_data->FRM_dir[dir].frame_data) {
//         char buff[256];

//         ImGui::PushFont(font);
//         snprintf(buff, 256, "framerate: %d",       img_data->FRM_hdr->FPS);
//         ImGui::Text(buff);

//         snprintf(buff, 256, "Shift_Orient_x: %d",  img_data->FRM_hdr->Shift_Orient_x[dir]);
//         ImGui::Text(buff);
//         snprintf(buff, 256, "Shift_Orient_y: %d",  img_data->FRM_hdr->Shift_Orient_y[dir]);
//         ImGui::Text(buff);

//         snprintf(buff, 256, "bounding_x1: %d\t",   img_data->FRM_dir[dir].bounding_box[num].x1);
//         ImGui::Text(buff);
//         ImGui::SameLine();
//         snprintf(buff, 256, "width: %d\t",         img_data->FRM_dir[dir].frame_data[num]->Frame_Width);
//         ImGui::Text(buff);
//         ImGui::SameLine();
//         snprintf(buff, 256, "x_offset: %d",        img_data->FRM_dir[dir].frame_data[num]->Shift_Offset_x);
//         ImGui::Text(buff);
//         snprintf(buff, 256, "bounding_x2: %d",     img_data->FRM_dir[dir].bounding_box[num].x2);
//         ImGui::Text(buff);

//         snprintf(buff, 256, "bounding_y1: %d\t",   img_data->FRM_dir[dir].bounding_box[num].y1);
//         ImGui::Text(buff);
//         ImGui::SameLine();
//         snprintf(buff, 256, "height: %d\t",        img_data->FRM_dir[dir].frame_data[num]->Frame_Height);
//         ImGui::Text(buff);
//         ImGui::SameLine();
//         snprintf(buff, 256, "y_offset: %d",        img_data->FRM_dir[dir].frame_data[num]->Shift_Offset_y);
//         ImGui::Text(buff);
//         snprintf(buff, 256, "bounding_y2: %d",     img_data->FRM_dir[dir].bounding_box[num].y2);
//         ImGui::Text(buff);

//         snprintf(buff, 256, "FRM_bounding_x1: %d", img_data->FRM_bounding_box[dir].x1);
//         ImGui::Text(buff);
//         snprintf(buff, 256, "FRM_bounding_x2: %d", img_data->FRM_bounding_box[dir].x2);
//         ImGui::Text(buff);
//         snprintf(buff, 256, "FRM_bounding_y1: %d", img_data->FRM_bounding_box[dir].y1);
//         ImGui::Text(buff);
//         snprintf(buff, 256, "FRM_bounding_y2: %d", img_data->FRM_bounding_box[dir].y2);
//         ImGui::Text(buff);
//         ImGui::PopFont();
//     } else {
//         ImGui::Text("Like it says, No Image Data");
//     }
// }

void show_image_stats_ANM(image_data* img_data, ImFont* font)
{
    int num, dir, max;
    num = img_data->display_frame_num;
    dir = img_data->display_orient_num;
    char buff[256];

    ImGui::PushFont(font);
    //snprintf(buff, 256, "framerate: %d", img_data->ANM_hdr->FPS);
    //ImGui::Text(buff);

    //snprintf(buff, 256, "orient_shift_x: %d", img_data->ANM_hdr->Shift_Orient_x[r]);
    //ImGui::Text(buff);
    //snprintf(buff, 256, "orient_shift_y: %d", img_data->ANM_hdr->Shift_Orient_y[r]);
    //ImGui::Text(buff);

    snprintf(buff, 256, "bounding_x1: %d\t",    img_data->ANM_dir[dir].frame_box->x1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "width: %d\t",          img_data->ANM_dir[dir].frame_data[num]->w);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "x_offset: %d",         img_data->ANM_dir[dir].frame_data[num]->x);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_x2: %d",      img_data->ANM_dir[dir].frame_box->x2);
    ImGui::Text(buff);

    snprintf(buff, 256, "bounding_y1: %d\t",    img_data->ANM_dir[dir].frame_box->y1);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "height: %d\t",         img_data->ANM_dir[dir].frame_data[num]->h);
    ImGui::Text(buff);
    ImGui::SameLine();
    snprintf(buff, 256, "y_offset: %d",         img_data->ANM_dir[dir].frame_data[num]->y);
    ImGui::Text(buff);
    snprintf(buff, 256, "bounding_y2: %d",      img_data->ANM_dir[dir].frame_box->y2);
    ImGui::Text(buff);

    snprintf(buff, 256, "ANM_bounding_x1: %d",  img_data->ANM_bounding_box[dir].x1);
    ImGui::Text(buff);
    snprintf(buff, 256, "ANM_bounding_x2: %d",  img_data->ANM_bounding_box[dir].x2);
    ImGui::Text(buff);
    snprintf(buff, 256, "ANM_bounding_y1: %d",  img_data->ANM_bounding_box[dir].y1);
    ImGui::Text(buff);
    snprintf(buff, 256, "ANM_bounding_y2: %d",  img_data->ANM_bounding_box[dir].y2);
    ImGui::Text(buff);
    ImGui::PopFont();
}

void show_image_stats_MSK(image_data* img_data, ImFont* font)
{
    char buff[256];

    ImGui::PushFont(font);

    snprintf(buff, 256, "width: %d\t",          img_data->width);
    ImGui::Text(buff);
    snprintf(buff, 256, "height: %d\t",         img_data->height);
    ImGui::Text(buff);

    ImGui::PopFont();
}