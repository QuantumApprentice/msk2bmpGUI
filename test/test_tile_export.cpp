#define main app_main
#include "../src/build_linux.cpp"
#undef main

#include <stdio.h>
#include <assert.h>

// we might want to make the FAIL non-zero if we want multiple
// types of failures, but doing !test_blah to check for failure
// reads a little nicer so we'll only make that change if we do
// have multiple things we care about how it failed
#define FAIL 0
#define PASS 1

#define TILE_W 80
#define TILE_H 36

int test_single_tile_crop(Surface* mask, uint8_t tile_buff_full[TILE_W*TILE_H*3], uint8_t frm_pxls[100 * 100], int frm_offset_x, int frm_offset_y)
{
  memset(tile_buff_full, 0xCD, TILE_W*TILE_H);
  memset(frm_pxls, 133, 100*100); // 133 = bright red
  // this gives a full tile in every direction around the central tile
  uint8_t* tile_buff = tile_buff_full + TILE_W*TILE_H;
  memset(tile_buff, 0, TILE_W*TILE_H);
  memset(tile_buff + TILE_W*TILE_H, 0xCD, TILE_W*TILE_H);
  crop_single_tile(100, 100, tile_buff, frm_pxls, frm_offset_x, frm_offset_y);
  for (int x = 0; x < TILE_W; x++)
  {
    for (int y = 0; y < TILE_H; y++)
    {
      Color* mask_pixel = (Color*) (mask->pixels + y * mask->pitch + x * mask->channels);
      uint8_t expected_value = 133;

      // if the pixel is transparent in the mask,
      // it should be transparent in the tile
      if (mask_pixel->a == 0) expected_value = 0;

      // if the pixel is "to the left or right" of the image,
      // it should be transparent in the tile
      if (x + frm_offset_x < 0 || x + frm_offset_x >= 100) expected_value = 0;

      // if the pixel is "above or below" the image,
      // it should be transparent in the tile
      if (y + frm_offset_y < 0 || y + frm_offset_y >= 100) expected_value = 0;

      uint8_t actual_value = tile_buff[y * TILE_W + x];

      if (actual_value != expected_value) {
        Palette* palette = load_palette_to_Palette("resources/palette/color.pal");
        Surface* exported = Create8BitSurface(TILE_W, TILE_H, palette);
        memcpy(exported->pixels, tile_buff, TILE_W*TILE_H);
        exported = ConvertSurfaceToRGBA(exported);
        const char* filename = "test_resources/failed_tile_export.png";
        SaveSurfaceAsPNG(exported, filename);
        printf("Expected value %d at x:%d,y:%d, but we got %d. Wrote output to %s\n",
          expected_value, x, y, actual_value, filename);
        return FAIL;
      }
    }
  }
  uint8_t* oob_check_1 = tile_buff_full;
  uint8_t* oob_check_2 = tile_buff + TILE_W*TILE_H;
  for (int i = 0; i < TILE_W*TILE_H; i++)
  {
    uint8_t* p;
    if ((p = oob_check_1)[i] != 0xCD || (p = oob_check_2)[i] != 0xCD) {
      printf("Detected out-of-bounds write at index %ld relative to the tile_buff pointer\n",
        (int64_t)p - (int64_t)tile_buff);
      return FAIL;
    }
  }
  return PASS;
}

int test()
{
  // yes, this leaks memory on failure, no, I don't really care, it's a test
  Surface* mask = LoadFileAsRGBASurface("test_resources/tile_mask.png");
  if (!mask) {
    printf("Unable to load tile template\n");
    return FAIL;
  }
  int result = PASS;
  uint8_t tile_buff_full[TILE_W*TILE_H*3]; // 3x as large so we can test for writing out of bounds
  uint8_t frm_pxls[100 * 100];
  if (!test_single_tile_crop(mask, tile_buff_full, frm_pxls, 10, 10)) {
    printf("Full crop test failed\n");
    result = FAIL;
  }
  if (!test_single_tile_crop(mask, tile_buff_full, frm_pxls, -40, -10)) {
    printf("Top-left crop test failed\n");
    result = FAIL;
  }
  if (!test_single_tile_crop(mask, tile_buff_full, frm_pxls, 60, -10)) {
    printf("Top-right crop test failed\n");
    result = FAIL;
  }
  if (!test_single_tile_crop(mask, tile_buff_full, frm_pxls, -40, 80)) {
    printf("Bottom-left crop test failed\n");
    result = FAIL;
  }
  if (!test_single_tile_crop(mask, tile_buff_full, frm_pxls, 60, 80)) {
    printf("Bottom-right crop test failed\n");
    result = FAIL;
  }
  FreeSurface(mask);
  return result;
}

int main()
{
  if (!test()) return 1;
  printf("Tests passed\n");
  return 0;
}
