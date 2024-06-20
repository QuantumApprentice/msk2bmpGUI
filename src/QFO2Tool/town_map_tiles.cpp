//This will make your computer reboot
//into text mode after you reboot:
//  sudo systemctl set-default multi-user.target
//Then you can run this to make it reboot
//into graphical mode:
//  sudo systemctl set-default graphical.target
//Don't forget to chmod +x the .run file it downloads
#include "Edit_TILES_LST.h"
#include "town_map_tiles.h"

#include "B_Endian.h"

#include <string.h>
#include "tinyfiledialogs.h"

//crop town map tiles into linked list structs

//single tile crop using memcpy
void crop_single_tile(uint8_t* tile_buff,
                    uint8_t* frm_pxls,
                    int frm_w, int frm_h,
                    int x, int y)
{
    for (int row = 0; row < 36; row++)
    {
        int lft     = tile_mask[row * 2];
        int rgt     = tile_mask[row * 2 + 1];
        int offset  = rgt-lft;
        int buf_pos = ((row)*80)   + lft;
        int pxl_pos = ((row)*frm_w + lft)
                      + y*frm_w + x;

    //prevent TOP & BOTTOM pixels outside image from copying over
        if (row+y < 0 || row+y >= frm_h) {
            //just set the buffer lines to 0 and move on
            memset(tile_buff+buf_pos, 0, rgt-lft);
            continue;
        }

    //prevent RIGHT pixels outside the image from copying over
        if ((x + rgt) > frm_w) {
            //set offset amount of row to 0
            //this is outside the right of the image
            offset = frm_w - (x + lft);
            if (offset < 0) {
                //if the starting position is also outside the image
                //set the whole row to 0
                memset(tile_buff+buf_pos, 0, rgt-lft);
                continue;
            }
            memset(tile_buff+buf_pos+offset, 0, rgt-(offset)-lft);
        }
    //prevent LEFT pixels outside image from copying over
        if ((x+lft) < 0) {
            //set the part of the row not being copied to 0
            memset(tile_buff+buf_pos, 0, rgt-lft);
            //move pointers to account for part being skipped
            buf_pos += 0 - (x+lft);
            pxl_pos += 0 - (x+lft);
            offset = rgt + (x);
            if (offset < 0) {
                //skip if ending position offset is outside left side of image
                continue;
            }
        }

        memcpy(tile_buff+buf_pos, frm_pxls+pxl_pos, offset);
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

//TODO: needs more tweeking and testing
//Bakerstaunch vector clearing version w/SSE2 instructions
#include <emmintrin.h>
int crop_single_tile_vector_clear(
        __restrict__ uint8_t *dst, __restrict__ uint8_t *src,
        int src_width, int src_height,
        int src_tile_left, int src_tile_top)
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


town_tile* crop_TMAP_tile_ll(int offset_x, int offset_y, image_data *img_data, char* name)
{
    uint8_t tile_buff[80 * 36] = {0};
    int img_w = img_data->width;
    int img_h = img_data->height;

    uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);
    int name_length = strlen(name) + 7;
    town_tile* head = nullptr;
    town_tile* prev = nullptr;
    town_tile* tile = nullptr;

    int origin_x = -48 + offset_x;
    int origin_y =   0 + offset_y;
    int tile_num =   0;
    int row_cnt  =   1;
    bool running = true;
    while (running)
    {
        //TODO: clean this up, was used for testing different methods
        // crop_single_tileB(tile_buff, frm_pxls, img_w, img_h,
        //         origin.y, origin.x);
        crop_single_tile(tile_buff, frm_pxls, img_w, img_h, origin_x, origin_y);
        // crop_single_tile_vector_clear(tile_buff, frm_pxls, img_w, img_h,
        //         origin.y, origin.x);

        // snprintf(full_file_path, MAX_PATH, "%s/%s%03d.FRM", file_path, name, tile_num);
        // printf("making tile #%03d\n", tile_num);

        //turn each cropped tile into a linked list
        tile = (town_tile*)malloc(sizeof(town_tile));
        tile->name_ptr = (char*)malloc(name_length+1);
        tile->frm_data  = (uint8_t*)malloc(80*36);

        snprintf(tile->name_ptr, name_length + 1, "%s%03d.FRM", name, tile_num);
        memcpy(tile->frm_data, tile_buff, 80*36);
        tile->length  = name_length;
        tile->row     = row_cnt;
        tile->tile_id = 0;
        tile->next    = nullptr;


        if (head == nullptr) {
            head = tile;
        }
        if (prev == nullptr) {
            prev = head;
        } else {
            prev->next = tile;
            prev = tile;
        }
        // save_TMAP_tile(full_file_path, tile_buff, &header, &frame);
        tile_num++;

        //increment one tile position
        origin_x +=  48;
        origin_y += -12;

        if ((origin_y <= -36) || (origin_x >= img_w)) {
            //increment row and reset tile position
            row_cnt++;
            origin_x = -16*row_cnt - 48;
            origin_y =  36*row_cnt;
        }
        while ((origin_y >= img_h) || (origin_x <= -80)) {
            //increment one tile position until in range of pixels
            origin_x +=  48;
            origin_y += -12;
            //or until outside both width and height of image
            if ((origin_x >= img_w)) {
                running = false;
                break;
            }
        }
        // printf("row: %d, tile: %d, origin.x: %.0f, origin.y: %.0f\n", row_cnt, tile_num, origin.x, origin.y);
    }

    // char* new_tile_list = generate_new_tile_list(name, tile_num);
    // printf("%s", new_tile_list);
    // return new_tile_list;
    return head;

}


void save_TMAP_tile(char *file_path, uint8_t *data, FRM_Header* header, FRM_Frame* frame)
{
    FILE *file_ptr = fopen(file_path, "wb");
    if (!file_ptr) {
        tinyfd_messageBox(
            "Error",
            "Can not open this file in write mode.\n"
            "Make sure the default game path is set.",
            "ok", "error", 1);
        return;
    }
    fwrite(header, sizeof(FRM_Header), 1, file_ptr);
    fwrite(frame,  sizeof(FRM_Frame),  1, file_ptr);
    fwrite(data, 80 * 36, 1, file_ptr);
    fclose(file_ptr);
}

//crop town map tiles from a full image
//TODO: need to add offsets to x & y
char* crop_TMAP_tiles(int offset_x, int offset_y, image_data *img_data, char* file_path, char* name)
{
    char full_file_path[MAX_PATH];
    uint8_t tile_buff[80 * 36] = {0};
    int img_w = img_data->width;
    int img_h = img_data->height;

    FRM_Header header = {};
    header.version = 4; // not sure why 4? but vanilla game frm tiles have this
    header.FPS = 1;
    header.Frames_Per_Orient = 1;
    header.Frame_Area = 80 * 36 + sizeof(FRM_Frame);
    B_Endian::flip_header_endian(&header);

    FRM_Frame frame = {};
    frame.Frame_Height = 36;
    frame.Frame_Width  = 80;
    frame.Frame_Size   = 80 * 36;
    B_Endian::flip_frame_endian(&frame);

    uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);

    int origin_x = -48 + offset_x;
    int origin_y =   0 + offset_y;
    int tile_num =   0;
    int row_cnt  =   0;
    bool running = true;
    while (running)
    {
        //TODO: clean this up, was used for testing different methods
        // crop_single_tileB(tile_buff, frm_pxls, img_w, img_h,
        //         origin.y, origin.x);
        crop_single_tile(tile_buff, frm_pxls, img_w, img_h, origin_x, origin_y);
        // crop_single_tile_vector_clear(tile_buff, frm_pxls, img_w, img_h,
        //         origin.y, origin.x);

        snprintf(full_file_path, MAX_PATH, "%s/%s%03d.FRM", file_path, name, tile_num);
        // printf("making tile #%03d\n", tile_num);
        save_TMAP_tile(full_file_path, tile_buff, &header, &frame);
        tile_num++;

        //increment one tile position
        origin_x +=  48;
        origin_y += -12;

        if ((origin_y <= -36) || (origin_x >= img_w)) {
            //increment row and reset tile position
            row_cnt++;
            origin_x = -16*row_cnt - 48;
            origin_y =  36*row_cnt;
        }
        while ((origin_y >= img_h) || (origin_x <= -80)) {
            //increment one tile position until in range of pixels
            origin_x +=  48;
            origin_y += -12;
            //or until outside both width and height of image
            if ((origin_x >= img_w)) {
                running = false;
                break;
            }
        }
        // printf("row: %d, tile: %d, origin.x: %.0f, origin.y: %.0f\n", row_cnt, tile_num, origin.x, origin.y);
    }

    char* new_tile_list = generate_new_tile_list(name, tile_num);
    // printf("%s", new_tile_list);

    return new_tile_list;
}