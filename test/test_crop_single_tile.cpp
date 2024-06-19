#include "test/test_header.h"


void save_tile_buff_frm(uint8_t* tile_buff, const char* path)
{
    FRM_Header header_out = {};
    header_out.version = 4;
    header_out.FPS = 1;
    header_out.Frames_Per_Orient = 1;
    header_out.Frame_Area = TILE_W*TILE_H*3 + sizeof(FRM_Frame);
    FRM_Frame frame_out = {};
    frame_out.Frame_Height = TILE_H*3;
    frame_out.Frame_Width  = TILE_W;
    frame_out.Frame_Size   = TILE_W*TILE_H*3;
    B_Endian::flip_header_endian(&header_out);
    B_Endian::flip_frame_endian(&frame_out);

    FILE* file = fopen(path, "wb");
    if (file == nullptr) {
        printf("failed at step 0\n");
    }
    fwrite(&header_out, sizeof(FRM_Header), 1, file);
    fwrite(&frame_out,  sizeof(FRM_Frame),  1, file);
    fwrite(tile_buff,  TILE_W*TILE_H*3,    1, file);
    fclose(file);
}

void save_tile_buff_animated_frm(const char* path, FRM_Dir* dir, data_ll* head)
{
    FRM_Header header_out = {};
    header_out.version = 4;
    header_out.FPS = 120;
    header_out.Frames_Per_Orient = dir->num_frames;
    header_out.Frame_Area = TILE_W*TILE_H*3 + sizeof(FRM_Frame);

    FRM_Frame frame_out = {};
    frame_out.Frame_Height = TILE_H*3;
    frame_out.Frame_Width  = TILE_W;
    frame_out.Frame_Size   = TILE_W*TILE_H*3;
    B_Endian::flip_header_endian(&header_out);
    B_Endian::flip_frame_endian(&frame_out);

    data_ll* node = head;

    char buff[256];
    snprintf(buff, strlen(path) + strlen("/HMJMPSAB.FRM") + 1, "%s%s", path, "/HMJMPSAB.FRM");

    FILE* file = fopen(buff, "wb");
    if (file == nullptr) {
        printf("failed at step 0\n");
    }

    fwrite(&header_out, sizeof(FRM_Header), 1, file);

    for (int i = 0; i < dir->num_frames; i++)
    {
        fwrite(&frame_out, sizeof(FRM_Frame), 1, file);
        fwrite(node->pxls, TILE_W*TILE_H*3,   1, file);
        node = node->next;
    }

    fclose(file);
}

void append_tile_buff(uint8_t* tile_buff, data_ll* node)
{
    memcpy(node->pxls, tile_buff, TILE_W*TILE_H*3);
    node->next = (data_ll*)malloc(sizeof(data_ll) + TILE_W*TILE_H*3);
}

int test_crop_single_tile(int frm_w, int frm_h, int frm_x, int frm_y, uint8_t* tile_buff, uint8_t* frm_pxls)
{
    memset(tile_buff + TILE_W*TILE_H, 216, TILE_W*TILE_H);
    uint8_t* buff_ptr = tile_buff + TILE_W*TILE_H;

    //run the test
    crop_single_tile(buff_ptr, frm_pxls, frm_w, frm_h, frm_x, frm_y);
    // crop_single_tile_vector_clear(buff_ptr, frm_pxls, frm_w, frm_h, frm_x, frm_y);

    //check the result
    uint8_t expected_value = 168;
    buff_ptr = tile_buff;
    for (int i = 0; i < TILE_W*TILE_H; i++)
    {
        if (buff_ptr[i] != expected_value) {
            expected_value = FAIL;
        }
    }
    buff_ptr = tile_buff + 2*TILE_W*TILE_H;
    for (int i = 0; i < TILE_W*TILE_H; i++)
    {
        if (buff_ptr[i] != expected_value) {
            expected_value = FAIL;
        }
    }

    return expected_value == 168;

}

int test()
{

    data_ll* head = (data_ll*)malloc(sizeof(data_ll) + TILE_W*TILE_H*3);
    data_ll* node = head;
    FRM_Dir dir = {};

    int result = PASS;
    char name[256] = {"/home/quantum/Programming/Qs Modding Tool/test junk/test#"};
    char path[256] = {"/home/quantum/Programming/Qs Modding Tool/test junk/test#"};
    int frm_w = 100;
    int frm_h = 40;
    uint8_t tile_buff[TILE_W*TILE_H*3];
    uint8_t frm_pxls[frm_w*frm_h];
    //memset whole tile_buff so we can see
    //  if anything is written out of bounds
    memset(tile_buff, 168, TILE_W*TILE_H*3);
    //memset frm_pxls to color so we can see what
    //  part was copied from frm_pxls to tile_buff
    memset(frm_pxls, 133, frm_w*frm_h);
    //memset middle of tile_buff to 0 so we
    //  can indicate where the tile _should_
    //  be copied to, and have space around
    //  if copies happen outside the middle
    //  Also prevents seg-faults/crashes if
    //  we do write outside
    memset(tile_buff + TILE_W*TILE_H, 216, TILE_W*TILE_H);

    uint64_t start_time = start_timer();
    for (int y = -36; y < frm_h; y++)
    {
        for (int x = -80; x < frm_w; x++)
        {
            result = test_crop_single_tile(frm_w, frm_h, x, y, tile_buff, frm_pxls);
            dir.num_frames++;

            append_tile_buff(tile_buff, node);
            node = node->next;

            if (result == FAIL) {
                break;
            }
            // printf("PASS: %04d\n", dir.num_frames);
        }
    }
    print_timer(start_time);

    *strrchr(path, '/') = '\0';
    save_tile_buff_animated_frm(path, &dir, head);

    return result;
}

int main()
{
    if (!test()) return 1;
    printf("Tests passed\n");
    return 0;
}

