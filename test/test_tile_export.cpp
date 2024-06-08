#define main app_main
#import "../src/build_linux.cpp"
#undef main

#import <stdio.h>
#import <assert.h>

#define FAIL 1

int main()
{
  // yes, this leaks memory on failure, no, I don't really care, it's a test
  Surface* surface = LoadFileAsRGBASurface("test_resources/tile_mask.png");
  if (!surface) {
    printf("Unable to load tile template\n");
    return FAIL;
  }
  uint8_t tile_buff[80 * 36] = {0};
  uint8_t frm_pxls[100 * 100] = {0};
  memset(frm_pxls, 123, sizeof(frm_pxls));
  crop_single_tile(100, 100, tile_buff, frm_pxls, -40, -10);
  for (int x = 0; x < 80; x++)
  {
    for (int y = 0; y < 36; y++)
    {
      Color* surface_pixel = (Color*) (surface->pixels + y * surface->pitch + x * surface->channels);
      uint8_t expected_value = 123;

      // if the pixel is transparent in the mask,
      // it should be transparent in the tile
      if (surface_pixel->a == 0) expected_value = 0;

      // if the pixel is "to the left" of the image,
      // it should be transparent in the tile
      if (x < 40) expected_value = 0;

      // if the pixel is "above" the image,
      // it should be transparent in the tile
      if (y < 10) expected_value = 0;

      uint8_t actual_value = tile_buff[y * 80 + x];

      if (actual_value != expected_value) {
        Palette* palette = load_palette_to_Palette("resources/palette/color.pal");
        Surface* exported = Create8BitSurface(80, 36, palette);
        exported = ConvertSurfaceToRGBA(exported);
        memcpy(exported->pixels, tile_buff, sizeof(tile_buff));
        const char* filename = "test_resources/failed_tile_export.png";
        SaveSurfaceAsPNG(exported, filename);
        printf("Expected value %d at x:%d,y:%d, but we got %d. Wrote output to %s\n",
          expected_value, x, y, actual_value, filename);
        return FAIL;
      }
    }
  }
  FreeSurface(surface);
  printf("Tests passed\n");
  return 0;
}
