#include "imgui.h"
#include "imgui_internal.h"

#include "Preview_Tiles.h"
#include "display_FRM_OpenGL.h"
#include "Zoom_Pan.h"

#include "load_FRM_OpenGL.h"
#include "B_Endian.h"

// Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define MTILE_W (350)
#define MTILE_H (300)
#define MTILE_SIZE (350 * 300)

void tile_me(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data *img_data);
void draw_red_squares(image_data *img_data, bool show_squares);
void draw_red_tiles(image_data *img_data, bool show_squares);

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void Prev_WMAP_Tiles(variables *My_Variables, image_data *img_data)
{
    // handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info *shaders = &My_Variables->shaders;

    // draw_FRM_to_framebuffer(shaders->palette,
    //                         shaders->render_FRM_shader,
    //                        &shaders->giant_triangle,
    //                         img_data);

    if (img_data->FRM_dir)
    {
        if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL)
        {
            ImGui::Text("No Image Data");
            return;
        }
        else
        {
            animate_FRM_to_framebuff(shaders->palette,
                                     shaders->render_FRM_shader,
                                     shaders->giant_triangle,
                                     img_data,
                                     My_Variables->CurrentTime_ms,
                                     My_Variables->Palette_Update);
        }
    }
    else
    {
        ImGui::Text("No Image Data");
        return;
    }

    float scale = img_data->scale;
    int img_width = img_data->width;
    int img_height = img_data->height;

    tile_me(MTILE_W, MTILE_H, img_width, img_height, scale, img_data);
}

void tile_me(int tile_w, int tile_h, int img_w, int img_h, int scale, image_data *img_data)
{

    // ImVec2 size = ImVec2((float)(img_w * scale), (float)(img_h * scale));
    ImVec2 base_top_corner = top_corner(img_data);
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGuiWindow *window = ImGui::GetCurrentWindow();
    // Preview window for tiles already converted to palettized and dithered format
    ImVec2 uv_min; // = Origin;
    ImVec2 uv_max;
    int max_box_x = img_w / tile_w;
    int max_box_y = img_h / tile_h;
    int pxl_border = 3;

    for (int y = 0; y < max_box_y; y++)
    {
        for (int x = 0; x < max_box_x; x++)
        {

            uv_min.x = ((x * (float)tile_w)) / img_w;
            uv_min.y = ((y * (float)tile_h)) / img_h;

            uv_max = {(uv_min.x + ((float)tile_w / img_w)),
                      (uv_min.y + ((float)tile_h / img_h))};

            ImVec2 new_corner;
            new_corner.x = base_top_corner.x + (tile_w + pxl_border) * x * scale;
            new_corner.y = base_top_corner.y + (tile_h + pxl_border) * y * scale;

            ImVec2 new_bottom;
            new_bottom.x = new_corner.x + (tile_w * scale);
            new_bottom.y = new_corner.y + (tile_h * scale);

            // image I'm trying to pan and zoom with
            window->DrawList->AddImage(
                (ImTextureID)img_data->render_texture,
                new_corner, new_bottom,
                uv_min, uv_max,
                ImGui::GetColorU32(tint_col));

            // window->DrawList->AddImage(
            //     (ImTextureID)img_data->render_texture,
            //     new_corner, bottom_corner(size, new_corner),
            //     Top_Left, Bottom_Right,
            //     ImGui::GetColorU32(tint_col));
        }
    }
}

void masking(image_data *img_data, GLuint msk_texture, ImVec2 TLC, shader_info *shaders, uint8_t *img_buff, int tile_x, int tile_y)
{
    int width = img_data->width;
    int height = img_data->height;
    int img_size = width * height;

    // copy edited texture to buffer, combine with original image

    // create a buffer
    uint8_t texture_buffer[80 * 36];   // = (uint8_t*)malloc(80 * 36);
    uint8_t msk_texture_buff[80 * 36]; // = (uint8_t*)malloc(80 * 36);

    static GLuint framebuffer;
    if (!glIsFramebuffer(framebuffer))
    {
        glGenFramebuffers(1, &framebuffer);
    }
    glBindFramebuffer(GL_TEXTURE_2D, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->FRM_texture, 0);
    if ((TLC.x > -80) && (TLC.y > -36))
    {
        glReadPixels(TLC.x, TLC.y, 80, 36, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);
    }
    else
    {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, msk_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_UNSIGNED_BYTE, msk_texture_buff);

    for (int i = 0; i < 80 * 36; i++)
    {
        if (msk_texture_buff[i] > 0)
        {
            msk_texture_buff[i] = 255;
        }
    }

    // for (int y = 0; y < 36; y++)
    //{
    //     for (int x = 0; x < 80; x++)
    //     {
    //         texture_buffer[(y * 80 + x)] = img_buff[(y * 80 + x) + (tile_x * 80 + tile_y*width*36)];
    //     }
    // }

    for (int i = 0; i < 80 * 36; i++)
    {
        if (!(texture_buffer[i] && msk_texture_buff[i]))
        {
            texture_buffer[i] = 00;
        }
        // texture_buffer[i] &= msk_texture_buff[i];
    }

    static GLuint tile_texture;
    if (!glIsTexture(tile_texture))
    {
        glGenTextures(1, &tile_texture);
    }
    glBindTexture(GL_TEXTURE_2D, tile_texture);
    // texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 80, 36, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    if (glIsTexture(img_data->PAL_texture))
    {
        glDeleteTextures(1, &img_data->PAL_texture);
    }
    glGenTextures(1, &img_data->PAL_texture);
    // texture settings
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 80, 36, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindFramebuffer(GL_TEXTURE_2D, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->PAL_texture, 0);

    draw_FRM_to_framebuffer(shaders, 80, 36, framebuffer, tile_texture);
}

int tile_mask[] = {
    43, 50,
    39, 51,
    35, 53,
    31, 54,
    27, 55,
    22, 57,
    18, 58,
    14, 59,
    11, 60,
     7, 62,
     3, 63,
     0, 65,
     1, 66,
     3, 68,
     4, 69,
     6, 70,
     7, 72,
     8, 73,
     9, 74,
    11, 75,
    12, 77,
    13, 78,
    14, 79,
    16, 80,
    17, 79,
    19, 75,
    20, 71,
    21, 67,
    23, 62,
    24, 58,
    25, 54,
    26, 50,
    28, 46,
    29, 42,
    30, 39,
    32, 35};

ImVec2 L_Corner = {00, 00};
ImVec2 T_Corner = {48,-12};
ImVec2 R_Corner = {80, 12};
ImVec2 B_Corner = {32, 24};

void save_TMAP_tiles(char *name, uint8_t *data)
{
    FRM_Header header = {};
    header.version = 4; // not sure why 4? but vanilla game frm tiles have this
    header.FPS = 1;
    header.Frames_Per_Orient = 1;
    header.Frame_Area = 80 * 36 + sizeof(FRM_Frame);

    FRM_Frame frame = {};
    frame.Frame_Height = 36;
    frame.Frame_Width = 80;
    frame.Frame_Size = 80 * 36;
    B_Endian::flip_header_endian(&header);
    B_Endian::flip_frame_endian(&frame);

    FILE *file_ptr = fopen(name, "wb");
    fwrite(&header, sizeof(FRM_Header), 1, file_ptr);
    fwrite(&frame, sizeof(FRM_Frame), 1, file_ptr);
    fwrite(data, 80 * 36, 1, file_ptr);
    fclose(file_ptr);
}

void crop_single_tile_nofloat(int img_w, int img_h,
                    uint8_t* tile_buff,
                    uint8_t* frm_pxls,
                    int x_offs, int y_offs)
{
    int x = x_offs;
    int y = y_offs;
    for (int row = 0; row < 36; row++)
    {
        int lft = tile_mask[row * 2];
        int rgt = tile_mask[row * 2 + 1];
        int buf_pos = ((row)*80)   + lft;
        int pxl_pos = ((row)*img_w + lft)
                      + y*img_w + x;
        memcpy(tile_buff+buf_pos, frm_pxls+pxl_pos, rgt - lft);
    }

}

/////////////////Bakerstaunch version with simplified logic/////////////////
// Note - I've changed the parameter order
// and used separate parameters for the tile
// top and left

// This function requires src_tile_left to be >-80
// and <src_width, otherwise, we could write more
// than intended when we're handling the trimming
// on the left and right edge of the tile  i.e.
// the two trimmed variables below could extend
// past the bounds of the row being written to
void crop_single_tileB(uint8_t *dst,
        uint8_t *src, int src_width, int src_height,
        int src_tile_top, int src_tile_left)
{
    assert(src_tile_left > -80);
    assert(src_tile_left < src_width);
    for (int row = 0; row < 36; ++row) {
        int row_left  = tile_mask[row*2+0];
        int row_right = tile_mask[row*2+1];

        uint8_t *dst_row_ptr = dst + row * 80;

        int src_row = src_tile_top + row;
        if (src_row < 0 || src_row >= src_height) {
            // above top or bottom of the src image
            // so we have no pixels to copy, set to 0
            memset(dst_row_ptr + row_left, 0, row_right - row_left);
            // continue could be used here for an early return,
            // but given it's in the middle of the loop it seems
            // clearer to use an else
        } else {
            int src_offset = src_row * src_width + src_tile_left;
            int src_row_left  = src_tile_left + row_left;
            int src_row_right = src_tile_left + row_right;

            // put 0s in the left side of dst when we're off
            // the left edge of the src image
            if (src_row_left < 0) {
                int trimmed = -src_row_left;
                // technically this could set more than
                // row_right - row_left bytes, but that's okay, as it
                // won't go past the end of the row in the destination
                // buffer as long as src_tile_left is > -80
                memset(dst_row_ptr + row_left, 0, trimmed);
                row_left += trimmed;

                // the following line is not needed as we currently
                // don't use src_row_left later in the function;
                // however it has been left here in case it's needed
                // in the future as part of the job of this if block
                // is to make sure the left side is within bounds
                // when we read it
                src_row_left = 0;
            }

            // put 0s in the right side of dst when we're off
            // the right edge of the src image; it's unlikely
            // this will also run when we're trimming the left
            // however for thin source images it's possible
            if (src_row_right > src_width) {
                int trimmed = src_row_right - src_width;
                row_right -= trimmed;
                // technically this could set more than
                // row_right - row_left bytes, but that's okay, as it
                // won't go past the end of the row in the destination
                // buffer as long as src_tile_left is < src_width
                memset(dst_row_ptr + row_right, 0, trimmed);

                // similar to above, we don't need the following line
                // at the moment
                src_row_right = src_width;
            }

            // we need this check as a safeguard against negative
            // sizes which could be the result of the row's pixels
            // entirely being in a trimmed area
            if (row_right - row_left > 0) {
                memcpy(dst_row_ptr + row_left,
                    src + src_offset + row_left,
                    row_right - row_left);
            }
        }
    }
}

//Bakerstaunch vector clearing version w/SSE2 instructions
#include <emmintrin.h>
int crop_single_tile_vector_clear(
        __restrict__ uint8_t *dst, __restrict__ uint8_t *src,
        int src_width, int src_height,
        int src_tile_top, int src_tile_left)
{
    __m128i ZERO = _mm_setzero_si128();
    int copied_pixels = 0;
    for (int row = 0; row < 36; ++row) {
        int row_left  = tile_mask[row*2+0];
        int row_right = tile_mask[row*2+1];

        uint8_t *dst_row_ptr = dst + row * 80;

        // clear the row with transparent pixels
        __m128i *dst_row_vec_ptr = (__m128i *)dst_row_ptr;
        _mm_storeu_si128(dst_row_vec_ptr+0, ZERO);
        _mm_storeu_si128(dst_row_vec_ptr+1, ZERO);
        _mm_storeu_si128(dst_row_vec_ptr+2, ZERO);
        _mm_storeu_si128(dst_row_vec_ptr+3, ZERO);
        _mm_storeu_si128(dst_row_vec_ptr+4, ZERO);

        int src_row = src_tile_top + row;
        if (src_row < 0 || src_row >= src_height) {
            // above top or bottom of the src image
            // and we've already cleared the row so
            // just go to the next row
            continue;
        }

        int src_row_left  = src_tile_left + row_left;
        // if we're off the left side of the image, increase
        // row_left by the amount we're off the left side
        if (src_row_left < 0) {
            // note src_row_left is negative which makes
            // this subtraction an addition
            row_left -= src_row_left;
        }

        int src_row_right = src_tile_left + row_right;
        // if we're off the right side of the image, decrease
        // row_right by the amount we're off the right side
        if (src_row_right > src_width) {
            row_right -= src_row_right - src_width;
        }

        int amount_to_copy = row_right - row_left;
        // we need this check as a safeguard against negative
        // sizes which could be the result of the row's pixels
        // entirely being in a trimmed area
        if (amount_to_copy > 0) {
            int src_offset = src_row * src_width + src_tile_left;
            memcpy(dst_row_ptr + row_left,
                src + src_offset + row_left,
                amount_to_copy);
            copied_pixels += amount_to_copy;
        }
    }
    return copied_pixels;
}



//single tile crop using memcpy
void crop_single_tile(int img_w, int img_h,
                    uint8_t* tile_buff,
                    uint8_t* frm_pxls,
                    ImVec2 pxl_offs)
{
    int x = pxl_offs.x;
    int y = pxl_offs.y;
    for (int row = 0; row < 36; row++)
    {
        int lft = tile_mask[row * 2];
        int rgt = tile_mask[row * 2 + 1];
        int offset = rgt-lft;
        int buf_pos = ((row)*80)   + lft;
        int pxl_pos = ((row)*img_w + lft)
                      + y*img_w + x;
    // printf("x: %d, rgt: %d, total: %d\n", x, rgt, x+rgt);
    //prevent RIGHT pixels outside the image from copying over
        if ((x + rgt) > img_w) {
            offset = img_w - (x + lft);
            memset(tile_buff+buf_pos+offset, 0, 80-(offset));
            if (offset < 0) {
                memset(tile_buff+buf_pos, 0, 80-lft);
                continue;
            }
        }

    //prevent LEFT pixels outside image from copying over
        if ((x+lft) < 0) {
            memset(tile_buff+buf_pos, 0, -(x+lft));
            buf_pos += 0 - (x+lft);
            pxl_pos += 0 - (x+lft);
            offset = rgt + (x);
            if (offset < 0) {
                continue;
            }
        }

    //prevent TOP & BOTTOM pixels outside image from copying over
        if (row+y < 0 || row+y >= img_h) {
            memset(tile_buff+buf_pos, 0, rgt-lft);
            continue;
        }

        // printf("offset: %d\n", offset);
        memcpy(tile_buff+buf_pos, frm_pxls+pxl_pos, offset);
    }
}

bool runonce = true;
void crop_TMAP_tiles(ImVec2 top_corner, image_data *img_data)
{
    char file_name[32];
    uint8_t tile_buff[80 * 36] = {0};
    int img_w = img_data->width;
    int img_h = img_data->height;

    uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);
    if (runonce)
    {
        runonce = false;
        ImVec2 origin = {-48,0};
        int tile_num = 0;
        int row_cnt = 0;
        while (tile_num < 200)
        {

            // crop_single_tileB(tile_buff, frm_pxls, img_w, img_h,
            //         origin.y, origin.x);
            crop_single_tile(img_w, img_h, tile_buff, frm_pxls, origin);
            // crop_single_tile_vector_clear(tile_buff, frm_pxls, img_w, img_h,
            //         origin.y, origin.x);

            snprintf(file_name, 32, "test_tile_%03d.frm", tile_num);
            // printf("making tile #%03d\n", tile_num);
            save_TMAP_tiles(file_name, tile_buff);
            tile_num++;

            //increment one tile position
            origin.x +=  48;
            origin.y += -12;

            if ((origin.y <= -36) || (origin.x >= img_w)) {
                //increment row and reset tile position
                row_cnt++;
                origin.x = -16*row_cnt - 48;
                origin.y = row_cnt * 36;
            }
            while ((origin.y >= img_h) || (origin.x <= -80)) {
                //increment one tile position until in range of pixels
                origin.x +=  48;
                origin.y += -12;
                //or until outside both width and height of image
                if ((origin.x >= img_w)) {
                    return;
                }
            }


            // printf("row: %d, tile: %d, origin.x: %.0f, origin.y: %.0f\n", row_cnt, tile_num, origin.x, origin.y);

 
// break;

        }
    }
}



//crops 4 tiles in staggered order
void tile_quad_crop(image_data *img_data, int *t_mask)
{
    char file_name[32];
    uint8_t tile_buff[80 * 36] = {0};
    uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);
    int img_w = img_data->width;
    int img_h = img_data->height;

    int origin_x = 0;
    int origin_y = 0;
    int tile_num = 0;
    while (tile_num < 20) {
        if (origin_x > img_w) {
            origin_x = 0;
            origin_y += 96;
        }
        for (int tile_quad = 0; tile_quad < 4; tile_quad++)
        {
            ImVec2 origin = {0,0};
            switch (tile_quad)
            {
            case 0:
                origin.x += 0;
                origin.y += -12;
                break;
            case 1:
                origin.x += +48;
                origin.y += -24;
                break;
            case 2:
                origin.x += +32;
                origin.y += +12;
                break;
            case 3:
                origin.x += 80;
                origin.y += 0;
                break;
            }
            // frm_pxls
            snprintf(file_name, 32, "test_tile_%02d.frm", tile_num);
            tile_num++;

            crop_single_tile(img_w, img_h, tile_buff, frm_pxls, origin);

            printf("making tile #%02d\n", tile_num);
            save_TMAP_tiles(file_name, tile_buff);
        }
        origin_x += 160;
    }
}

void tile_t(image_data *img_data, shader_info *shaders,
            GLuint tile_texture,
            int img_w, int img_h,
            int tile_w, int tile_h)
{
    float scale = img_data->scale;
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec2 base_top_corner = top_corner(img_data);

    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

    ImVec2 Top_Left; // = Origin;
    ImVec2 Bottom_Right = {0, 0};
    ImVec2 uv_min = {0, 0};
    ImVec2 uv_max = {1.0, 1.0};
    // int pxl_border = 30;

    // //Save tiles button
    // char* Save_File_Name
    // if (ImGui::Button("Save Tiles")) {
        // Save_File_Name = tinyfd_saveFileDialog(
        // "default_name",
        // temp_name,
        // num_patterns,
        // lFilterPatterns,
        // nullptr);
    //     save_tiles()
    // }

    ImVec2 TLC, BRC;

    static float offset_x;
    static float offset_y;
    ImGui::SliderFloat("offset_x", &offset_x, -200, 200.0, NULL);
    ImGui::SliderFloat("offset_y", &offset_y, -200, 200.0, NULL);

    uint8_t *temp_buffer = (uint8_t *)malloc(img_data->width * img_data->height);
    // read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, temp_buffer);
    // glReadPixels(TLC.x, TLC.y, 200, 200, GL_RGB, GL_UNSIGNED_BYTE, texture_buffer);

    ImVec2 Left, Top, Bottom, Right, new_origin;

    static int offset1;
    static int offset2;
    static int offset3;
    static int offset4;
    ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
    ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
    ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    int col_w = (80 + 48);      // 128
    int row_h = (36 + 36 + 24); // 96

    int max_box_x; // = img_w / 32;// tile_w;
    int max_box_y; // = img_h / 24;// tile_h;
    int min_box_x;
    int min_box_y;

    max_box_x = +2 * ((img_w + (col_w - 1) - offset_x) / col_w);
    min_box_x = -3 * ((img_h + (row_h - 1) + offset_y) / row_h);
    max_box_y = 2 * ((img_w + (col_w - 1) - offset_y) / col_w) + 3 * ((img_h + (row_h - 1) - offset_y) / row_h);
    min_box_y = -3 * offset_y / row_h - 2 * offset_x / col_w;

    int tile_num = 0;
    for (int y = min_box_y; y < max_box_y; y++)
    {
        for (int x = min_box_x; x < max_box_x; x++)
        {
            new_origin.x = Origin.x + x * offset1 * scale;
            new_origin.y = Origin.y + y * offset2 * scale;

            Top_Left.x = new_origin.x + (x * 48 + y * 32) * scale;
            Top_Left.y = new_origin.y + (x * -12 + y * 24) * scale;

            Left.x = Top_Left.x + L_Corner.x * scale;
            Left.y = Top_Left.y + L_Corner.y * scale;

            Top.x = Top_Left.x + T_Corner.x * scale;
            Top.y = Top_Left.y + T_Corner.y * scale;

            Right.x = Top_Left.x + R_Corner.x * scale;
            Right.y = Top_Left.y + R_Corner.y * scale;

            Bottom.x = Top_Left.x + B_Corner.x * scale;
            Bottom.y = Top_Left.y + B_Corner.y * scale;

            ImVec2 uv_l, uv_t, uv_r, uv_b;
            uv_l.x = (Left.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_l.y = (Left.y + offset_y - new_origin.y) / img_data->height / scale;

            uv_t.x = (Top.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_t.y = (Top.y + offset_y - new_origin.y) / img_data->height / scale;

            uv_r.x = (Right.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_r.y = (Right.y + offset_y - new_origin.y) / img_data->height / scale;

            uv_b.x = (Bottom.x + offset_x - new_origin.x) / img_data->width / scale;
            uv_b.y = (Bottom.y + offset_y - new_origin.y) / img_data->height / scale;

            glBindTexture(GL_TEXTURE_2D, img_data->render_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            if (((uv_l.x >= 0) && (uv_l.x <= 1.0) && (uv_l.y >= 0) && (uv_l.y <= 1.0)) || ((uv_t.x >= 0) && (uv_t.x <= 1.0) && (uv_t.y >= 0) && (uv_t.y <= 1.0)) || ((uv_r.x >= 0) && (uv_r.x <= 1.0) && (uv_r.y >= 0) && (uv_r.y <= 1.0)) || ((uv_b.x >= 0) && (uv_b.x <= 1.0) && (uv_b.y >= 0) && (uv_b.y <= 1.0)))
            {

                window->DrawList->AddImageQuad(
                    (ImTextureID)img_data->render_texture,
                    Left, Top, Right, Bottom,
                    uv_l, uv_t, uv_r, uv_b);

                // int corner_x = Origin.x + x * offset1 + (x *  48 + y * 32);
                // int corner_y = Origin.y + y * offset2 + (x * -12 + y * 24);
                // uint8_t tile_buff[80*36] = {0};
                // crop_single_tile_nofloat(img_w, img_h, tile_buff,
                //                         img_data->FRM_data+sizeof(FRM_Header)+sizeof(FRM_Frame),
                //                         corner_x, corner_y);
                // printf("tile #%d, cornerX: %d, cornerY: %d", tile_num, corner_x, corner_y);
                // char file_name[32];
                // snprintf(file_name, 32, "test_tile_%03d.frm", tile_num);
                // printf("making tile #%03d\n", tile_num);
                // save_TMAP_tiles(file_name, tile_buff);
                // tile_num++;

                crop_TMAP_tiles(Left, img_data);
                // ImGui::ShowMetricsWindow();
                // printf("test: %s\n", (char*)(window->DrawList->_Data));

                // printf("position: %d,%d\n", Left.x, Left.y);
            }

            // Top_Left.x = Origin.x + x * (pxl_border_x)*scale;
            // Top_Left.y = Origin.y + y * (pxl_border_y)*scale;
            // Bottom_Right = { (float)(Top_Left.x + tile_w * scale), (float)(Top_Left.y + tile_h * scale) };

            TLC.x = (float)(x * 48.0 + y * 32.0);
            TLC.y = (float)(x * -12.0 + y * 24.0);

            // uv_min.x = TLC.x/ img_w;
            // uv_min.y = TLC.y/ img_h;

            // uv_max.x = uv_min.x + ((float)tile_w / img_w);
            // uv_max.y = uv_min.y + ((float)tile_h / img_h);

            // masking(img_data, tile_texture, TLC, shaders, temp_buffer, x, y);

            // window->DrawList->AddImage(
            //     (ImTextureID)img_data->PAL_texture,
            //     Top_Left, Bottom_Right,
            //     ImVec2(0, 0), ImVec2(1.0, 1.0),
            //     ImGui::GetColorU32(tint_col));
        }
    }

    free(temp_buffer);
}

// #define TMAP_W          (80)
// #define TMAP_H          (36)
#define TMAP_W (80 + 48)
#define TMAP_H (36 + 24)
void Prev_TMAP_Tiles(variables *My_Variables, image_data *img_data)
{
    // handle zoom and panning for the image, plus update image position every frame
    zoom_pan(img_data, My_Variables->new_mouse_pos, My_Variables->mouse_delta);

    shader_info *shaders = &My_Variables->shaders;

    if (!img_data->FRM_dir)
    {
        ImGui::Text("No Image Data");
        return;
    }
    if (img_data->FRM_dir[img_data->display_orient_num].frame_data == NULL)
    {
        ImGui::Text("No Image Data");
        return;
    }

    animate_FRM_to_framebuff(shaders->palette,
                             shaders->render_FRM_shader,
                             shaders->giant_triangle,
                             img_data,
                             My_Variables->CurrentTime_ms,
                             My_Variables->Palette_Update);

    ImVec2 uv_min = {0, 0};
    ImVec2 uv_max = {1.0, 1.0};
    int width = img_data->width;
    int height = img_data->height;
    float scale = img_data->scale;

    ImVec2 size = ImVec2((float)(width * scale), (float)(height * scale));

    static bool image_toggle = true;
    checkbox_handler("toggle image", &image_toggle);

    if (image_toggle)
    {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        window->DrawList->AddImage(
            (ImTextureID)img_data->render_texture,
            top_corner(img_data), bottom_corner(size, top_corner(img_data)),
            uv_min, uv_max,
            ImGui::GetColorU32(My_Variables->tint_col));
    }

    tile_t(img_data, &My_Variables->shaders, My_Variables->tile_texture_rend, width, height, TMAP_W, TMAP_H);
}

void draw_red_squares(image_data *img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (show_squares)
    {
        ImDrawList *Draw_List = ImGui::GetWindowDrawList();
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        ImVec2 Top_Left;
        ImVec2 Bottom_Right = {0, 0};
        int max_box_x = img_data->width / 350;
        int max_box_y = img_data->height / 300;

        for (int j = 0; j < max_box_y; j++)
        {
            for (int i = 0; i < max_box_x; i++)
            {
                Top_Left.x = Origin.x + (i * 350) * scale;
                Top_Left.y = Origin.y + (j * 300) * scale;
                Bottom_Right = {(float)(Top_Left.x + 350 * scale), (float)(Top_Left.y + 300 * scale)};
                Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
            }
        }
    }
}

// //this function not used right now
// void draw_quad(ImVec2 Image_Corner, ImVec2 Top_Left, float scale)
// {
//     ImDrawList *Draw_List = ImGui::GetWindowDrawList();
//     ImVec2 Left, Top, Bottom, Right, new_origin;
//     ImVec2 Bottom_Right = { 0, 0 };
//     ImVec2 L_Offset = { 00, 00 };
//     ImVec2 T_Offset = { 48,-12 };
//     ImVec2 R_Offset = { 80, 12 };
//     ImVec2 B_Offset = { 32, 24 };
//     // Bottom_Right = { (float)(Top_Left.x + TMAP_W * scale), (float)(Top_Left.y + TMAP_H * scale) };
//     //Draw_List->AddRect(Top_Left, Bottom_Right, 0xff0000ff, 0, 0, 5.0f);
//     Left.x   = Top_Left.x + L_Offset.x * scale;
//     Left.y   = Top_Left.y + L_Offset.y * scale;
//     Top.x    = Top_Left.x + T_Offset.x * scale;
//     Top.y    = Top_Left.y + T_Offset.y * scale;
//     Right.x  = Top_Left.x + R_Offset.x * scale;
//     Right.y  = Top_Left.y + R_Offset.y * scale;
//     Bottom.x = Top_Left.x + B_Offset.x * scale;
//     Bottom.y = Top_Left.y + B_Offset.y * scale;
//     if (Bottom.y < (Image_Corner.y-1)) {
//         return;
//     }
//     //Bottom.y = Origin.y + (i*TMAP_W) - 36;
//     Draw_List->AddQuad(Left, Bottom, Right, Top, 0xff0000ff, 1.0f);
// }

// struct to hold the 4 points for the quadrilateral (tile shape or image shape)
struct outline
{
    ImVec2 Top,
        Rgt,
        Btm,
        Lft;
};

// add offset to each point of square
void add_offset(ImVec2 offset, outline *square)
{
    square->Top.x += offset.x;
    square->Top.y += offset.y;
    square->Rgt.x += offset.x;
    square->Rgt.y += offset.y;
    square->Btm.x += offset.x;
    square->Btm.y += offset.y;
    square->Lft.x += offset.x;
    square->Lft.y += offset.y;
}

// draw tiles for the preview screen when checking the tiles box
void draw_red_tiles(image_data *img_data, bool show_squares)
{
    // Draw red boxes to indicate where the tiles will be cut from
    float scale = img_data->scale;
    if (!show_squares)
    {
        return;
    }

    ImDrawList *Draw_List = ImGui::GetWindowDrawList();
    ImVec2 Origin;
    Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
    Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

    ImVec2 Top_Left;
    outline tile_offsets;

    tile_offsets.Top = {48 * scale, -12 * scale};
    tile_offsets.Rgt = {80 * scale, 12 * scale};
    tile_offsets.Btm = {32 * scale, 24 * scale};
    tile_offsets.Lft = {00 * scale, 00 * scale};

    int max_box_x = img_data->width / TMAP_W;
    int max_box_y = img_data->height / TMAP_H;

    // ImVec2 new_origin;//, Left, Top, Bottom, Right;

    static int offset1;
    static int offset2;
    static int offset3;
    static int offset4;
    ImGui::SliderInt("offset1", &offset1, -80, 80, NULL);
    ImGui::SliderInt("offset2", &offset2, -80, 80, NULL);
    ImGui::SliderInt("offset3", &offset3, -80, 80, NULL);
    ImGui::SliderInt("offset4", &offset4, -80, 80, NULL);

    Origin.x += offset3 * scale;

    ImVec2 offset = {48 * scale + scale * offset1,
                     -12 * scale + scale * offset2};
    ImVec2 row_offset = {-16 * scale + scale * offset4,
                         36 * scale + scale * offset4};

    Top_Left.x = Origin.x - 32 * scale;
    Top_Left.y = Origin.y;

    outline row_start;                 // used when incrementing rows
    outline new_square;                // stores tile position for each tile
    new_square = tile_offsets;         // copy default dimensions
    add_offset(Top_Left, &new_square); // move first tile position to image corner
    row_start = new_square;            // copy this position for re-use

    int img_right = Origin.x + img_data->width * scale;
    int img_bottom = Origin.y + img_data->height * scale;
    bool drew_row = true;
    int count = 0;

    // for (int i = 0; i < 50; i++)
    // int i = 0;
    // int j = 0;
    while (true)
    {
        // int x = -16*scale*i;// + (-16) + offset4*scale;
        // int y =  36*scale*i     ;

        new_square = row_start;

        // when you switch to the next row, after doing the current addition,
        // you could "advance" the start of the row by regular tile offset
        //(h_offset and v_offset) until it's inside the image,
        // or past the end of it
        // to test it's working correctly, you could comment out the if
        // condition wrapping the draw quad
        // And you'd break if you couldn't "find" the start of the row
        //(i.e. the left point is past the right edge of the image)

        if (!drew_row)
        {
            printf("count: %d\n", count);
            break;
        }
        drew_row = false;

        // for (int j = 0; j < 50; j++)
        while (true)
        {
            if ((new_square.Top.y < img_bottom) && // crop bottom
                (new_square.Rgt.x > Origin.x))
            { // crop left
                Draw_List->AddQuad(new_square.Lft,
                                   new_square.Btm,
                                   new_square.Rgt,
                                   new_square.Top, 0xff0000ff, 1.0f);
                drew_row = true;
                count++;
            }

            add_offset(offset, &new_square);

            if (new_square.Btm.y < (Origin.y))
            { // crop top
                break;
            }
            if (new_square.Lft.x > (img_right))
            { // crop right
                break;
            }
        }

        add_offset(row_offset, &row_start);
    }

    // for (int j = 0; j < 5; j++)
    // {
    //     new_origin.x = Origin.x + j*offset1 * scale -32*scale;
    //     new_origin.y = Origin.y + j*offset2 * scale;

    //     Top_Left.x = new_origin.x + (j*32) *scale;
    //     Top_Left.y = new_origin.y;// + (12)*scale + (-12 + 24) *scale;

    //     draw_quad(Top_Left, scale);
    //-----------------------------------------------------------
    //     for (int i = 0; i < max_box_x*3; i++)
    //     {
    //         Top_Left.x = new_origin.x - (j * 48)*scale + (i* 48 + j*32) *scale;
    //         if (Top_Left.x < (Origin.x - TMAP_W)) {
    //             new_origin.x = Origin.x;
    //         }
    //         Top_Left.y = new_origin.y + (j * 12)*scale + (i*-12 + j*24) *scale;
    //         draw_quad(Top_Left, scale);
    //     }
    // }
}

void draw_tiles_OpenGL(image_data *img_data, shader_info *shader, GLuint *texture, bool draw_tiles)
{
    float scale = img_data->scale;

    printf("draw_tiles_OpenGL is being called...\n");

    if (draw_tiles)
    {
        ImVec2 Origin;
        Origin.x = img_data->offset.x + ImGui::GetItemRectMin().x;
        Origin.y = img_data->offset.y + ImGui::GetItemRectMin().y;

        glViewport(0, 0, img_data->width, img_data->height);
        glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);
        glBindVertexArray(shader->giant_triangle.VAO);
        glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 96, GL_RED, GL_UNSIGNED_BYTE, texture);

        glDrawArrays(GL_TRIANGLES, 0, shader->giant_triangle.vertexCount);

        // bind framebuffer back to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void draw_tiles_ImGui(image_data *img_data, ImVec2 Top_Corner, ImVec2 Bottom_Corner)
{
    // float scale = img_data->scale;
    // int tile_w  = img_data->width/128;
    // int tile_h  = img_data->height/96;

    // for (int x = 0; x < tile_w; x++)
    //{
    //     for (int y = 0; y < tile_h; y++)
    //     {

    // Top_Left.x = new_origin.x + (x * 48 + y * 32) *scale;
    // Top_Left.y = new_origin.y + (x *-12 + y * 24) *scale;

    // window->DrawList->AddImage(
    //     (ImTextureID)texture,
    //     new_corner, new_bottom,
    //     Top_Left, Bottom_Right,
    //     )

    //    }
    //}
}