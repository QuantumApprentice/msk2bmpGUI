#include "Edit_TILES_LST.h"
#include "town_map_tiles.h"

#include "B_Endian.h"

#include <string.h>
#include "tinyfiledialogs.h"

void save_TMAP_tile(char *save_path, uint8_t *data, char* name);


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

//explicitly floats so result can be rounded up
#define pxl_per_row_x       (124)   //  ((80+80-36)    /1) tile per repeat
#define pxl_per_row_y        (32)   //  ((36+36+36-12) /3) tiles per repeat
#define pxl_per_col_x        (62)   //  ((80+80-36)    /2) tiles per repeat
#define pxl_per_col_y        (48)   //  ((36+36+36-12) /2) tiles per repeat

//array version (stores tile position)
tt_arr_handle* crop_TMAP_tile_arr(int offset_x, int offset_y, image_data *img_data, char* save_path, char* name)
{
    uint8_t tile_buff[80 * 36] = {0};
    int img_w = img_data->width;
    int img_h = img_data->height;

    //rows/columns to the right and left
    //  of image origin corner
    float row_lft =      (float)img_h / (float)pxl_per_row_y;         //maybe use ceil() here?
    float row_rgt =      (float)img_w / (float)pxl_per_row_x;         //maybe use ceil() here?
    int   col_lft = ceil((float)img_h / (float)pxl_per_col_y);
    int   col_rgt = ceil((float)img_w / (float)pxl_per_col_x);

    //total number of rows/columns
    int row_cnt = ceil( row_lft + row_rgt );
    int col_cnt = col_lft + col_rgt;
    //move the start position to account for extra columns
    int origin_x = -48 * col_lft + offset_x;
    int origin_y =  12 * col_lft + offset_y;

    uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);
    tt_arr_handle* handle = (tt_arr_handle*)malloc(sizeof(tt_arr_handle) + row_cnt*col_cnt*(sizeof(tt_arr)));
    tt_arr* towntiles = handle->tile;
    tt_arr* tile = towntiles;
    int tile_num = 0;

    for (int row = 0; row < row_cnt; row++)
    {
        for (int col = 0; col < col_cnt; col++)
        {
            // //increment one tile position
            // origin_x +=  48;
            // origin_y += -12;
            tile = &towntiles[row*row_cnt + col];

            while ((origin_y >= img_h) || (origin_x <= -80)) {
                //assign blank values to this tile entry
                // tile[row*row_cnt + col].tile_id = 1 | 0x4000000;
                tile->tile_id = 1;
                tile->col     = col;
                tile->row     = row;

                //increment one tile column until in range of pixels
                origin_x +=  48;
                origin_y += -12;
                col++;

                if (col >= col_cnt) {
                    break;
                }

            }

            if ((origin_y <= -36) || (origin_x >= img_w)) {

                while (col < col_cnt) {
                    origin_x +=  48;
                    origin_y += -12;
                    tile->tile_id = 1;
                    col++;
                }

//Bakerstaunch
// int tile_x = origin_x + row * pxl_per_row_x + col * pxl_per_col_x;
// int tile_x = origin_y + row * pxl_per_row_y + col * pxl_per_col_y;
// and the same for y

// The starting origin x is 
// -(cols to the left * x pixels per col) 
// and origin y is 
// -(cols to the left * y pixels per col)

                origin_x =   -48 * col_lft
                            + 32 * row - 48
                            + offset_x;
                origin_y =    12 * col_lft
                            + 24 * row
                            + offset_y;
            }

            snprintf(tile->name_ptr, 14, "%s%03d.FRM", name, tile_num++);

            //TODO: clean this up, was used for testing different methods
            // crop_single_tileB(tile_buff, frm_pxls, img_w, img_h,
            //         origin.y, origin.x);
            crop_single_tile(tile_buff, frm_pxls, img_w, img_h, origin_x, origin_y);
            // crop_single_tile_vector_clear(tile_buff, frm_pxls, img_w, img_h,
            //         origin.y, origin.x);

            //TODO: check if blank tile before memcpy
            //      if blank, tile[row*col_cnt + col].tileID = 1;
            //      then move to next tile

            memcpy(tile->frm_data, tile_buff, 80*36);
            tile->row     = row;
            tile->col     = col;
            tile->tile_id = 0;


            //increment one tile column for next tile
            origin_x +=  48;
            origin_y += -12;

        }
        
    }

    handle->size = col_cnt*row_cnt;
    handle->col_cnt = col_cnt;
    handle->row_cnt = row_cnt;

    return handle;
}

//linked list version
town_tile* crop_TMAP_tile_ll(int offset_x, int offset_y, image_data *img_data, char* save_path, char* name)
{
    uint8_t tile_buff[80 * 36] = {0};
    int img_w = img_data->width;
    int img_h = img_data->height;

    int origin_x = -48 + offset_x;
    int origin_y =   0 + offset_y;

    uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);
    int name_length = strlen(name) + 7;
    town_tile* head = nullptr;
    town_tile* prev = nullptr;
    town_tile* tile = nullptr;
    int tile_num    = 0;
    int row_cnt     = 1;
    bool running    = true;
    while (running)
    {
        //TODO: clean this up, was used for testing different methods
        // crop_single_tileB(tile_buff, frm_pxls, img_w, img_h,
        //         origin.y, origin.x);
        crop_single_tile(tile_buff, frm_pxls, img_w, img_h, origin_x, origin_y);
        // crop_single_tile_vector_clear(tile_buff, frm_pxls, img_w, img_h,
        //         origin.y, origin.x);

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

        //TODO: maybe move this into a separate call that just uses town_tile*head and save_path?
        save_TMAP_tile(save_path, tile_buff, tile->name_ptr);


        if (head == nullptr) {
            head = tile;
        }
        if (prev == nullptr) {
            prev = head;
        } else {
            prev->next = tile;
            prev = tile;
        }
        tile_num++;


        //increment one tile position
        origin_x +=  48;
        origin_y += -12;

        if ((origin_y <= -36) || (origin_x >= img_w)) {
            //increment row and reset tile position
            origin_x = -16*row_cnt - 48;
            origin_y =  36*row_cnt;
            row_cnt++;
        }
        while ((origin_y >= img_h) || (origin_x <= -80)) {
            //increment one tile position until in range of pixels
            origin_x +=  48;
            origin_y += -12;
            //or until outside both width and height of image
            if (origin_x >= img_w) {
                running = false;
                break;
            }
        }
        // printf("row: %d, tile: %d, origin.x: %.0f, origin.y: %.0f\n", row_cnt, tile_num, origin.x, origin.y);
    }

    return head;

}


void save_TMAP_tile(char *save_path, uint8_t *data, char* name)
{
    char full_file_path[MAX_PATH];
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


    snprintf(full_file_path, MAX_PATH, "%s/%s", save_path, name);

    FILE *file_ptr = fopen(full_file_path, "wb");
    if (!file_ptr) {
        tinyfd_messageBox(
            "Error",
            "Can not open this file in write mode.\n"
            "Make sure the default game path is set.",
            "ok", "error", 1);
        return;
    }
    fwrite(&header, sizeof(FRM_Header), 1, file_ptr);
    fwrite(&frame,  sizeof(FRM_Frame),  1, file_ptr);
    fwrite(data, 80 * 36, 1, file_ptr);
    fclose(file_ptr);
}