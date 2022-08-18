#include "FRM_Convert.h"
#include "FRM_Animate.h"
#include "B_Endian.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <SDL.h>

#define PALETTE_NUMBER 256

union Pxl_info_32 {
	struct {
		uint8_t a;
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};
	uint8_t arr[4];
};

union Pxl_err {
	struct {
		int a;
		int b;
		int g;
		int r;
	};
	int arr[4];
};

uint8_t convert_colors(uint8_t bytes);

// Used to convert Fallout's palette colors to normal values
uint8_t convert_colors(uint8_t bytes) {
	if (bytes < 64) {
		return 4 * bytes;
	}
	else {
		return bytes;
	}
}

SDL_Color PaletteColors[256];
SDL_Color* loadPalette(char * name)
{
	std::ifstream f("color.pal", 
		std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		printf("Error opening color.pal.\n");
		return false;
	}

	uint8_t r, g, b;
	//printf("Palette size: %d\n", 256);

	for (int i = 0; i < 256; i++)
	{
		uint8_t bytes[4];

		f.read((char*)bytes, 3);
		r = convert_colors(bytes[0]);
		g = convert_colors(bytes[1]);
		b = convert_colors(bytes[2]);
		PaletteColors[i] = SDL_Color{ r, g, b };
	}
	return PaletteColors;
}

// Converts the color space to Fallout's paletted format
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface, bool SDL)
{
	// Convert all surfaces to 32bit RGBA8888 format for easy conversion
	SDL_PixelFormat pxlFMT_UnPal;
	pxlFMT_UnPal = *SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	pxlFMT_UnPal.palette = NULL;
	pxlFMT_UnPal.BitsPerPixel = 32;
	pxlFMT_UnPal.BytesPerPixel = 4;
	SDL_Surface* Surface_32 = SDL_ConvertSurface(surface, &pxlFMT_UnPal, 0);
	if (!Surface_32) {
		printf("Error: %s", SDL_GetError());
	}

	// Setup for palettizing image
	SDL_Palette myPalette;
	SDL_PixelFormat pxlFMT_Pal;
	SDL_Surface* Surface_8;

	myPalette.ncolors = PALETTE_NUMBER;
	myPalette.colors = loadPalette("default");

	// Convert image to palettized surface
	pxlFMT_Pal = *SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
	pxlFMT_Pal.palette = &myPalette;
	pxlFMT_Pal.BitsPerPixel = 8;
	pxlFMT_Pal.BytesPerPixel = 1;
	Surface_8 = SDL_ConvertSurface(surface, &pxlFMT_Pal, 0);
	if (!Surface_8) {
		printf("Error: %s", SDL_GetError());
	}

	//switch to change between euclidian and sdl color match algorithms
	if (SDL == true) {
		SDL_Color_Match(Surface_32,	&pxlFMT_Pal, Surface_8);
	}
	else
	{
		Euclidian_Distance_Color_Match(Surface_32, Surface_8);
	}
	return Surface_8;
}

void SDL_Color_Match(SDL_Surface* Surface_32,
					SDL_PixelFormat* pxlFMT_Pal,
					SDL_Surface* Surface_8)
{
	uint8_t w_PaletteColor;
	Pxl_info_32 abgr;
	Pxl_err err;
	// Convert image color to indexed palette
	for (int y = 0; y < Surface_32->h; y++)
	{
		for (int x = 0; x < Surface_32->w; x++)
		{
			int i = (Surface_32->w * y) + x;

			memcpy(&abgr, (Pxl_info_32*)Surface_32->pixels + i, sizeof(Pxl_info_32));
			w_PaletteColor = SDL_MapRGBA(pxlFMT_Pal, abgr.r, abgr.g, abgr.b, abgr.a);

			err.a = abgr.a - PaletteColors[w_PaletteColor].a;
			err.b = abgr.b - PaletteColors[w_PaletteColor].b;
			err.g = abgr.g - PaletteColors[w_PaletteColor].g;
			err.r = abgr.r - PaletteColors[w_PaletteColor].r;

			int* pxl_index_arr[2];
			pxl_index_arr[0] = &x;
			pxl_index_arr[1] = &y;

			limit_dither(Surface_32, &err, pxl_index_arr);

			if (i == 100)		{ printf("SDL loop #: %d\n", i); }
			if (i == 1000)		{ printf("SDL loop #: %d\n", i); }
			if (i == 10000)		{ printf("SDL loop #: %d\n", i); }
			if (i == 100000)	{ printf("SDL loop #: %d\n", i); }
			if (i == 1000000)	{ printf("SDL loop #: %d\n", i); }
			if (i == 2000000)	{ printf("SDL loop #: %d\n", i); }

			((uint8_t*)Surface_8->pixels)[(Surface_8->pitch*y)+x] = w_PaletteColor;
		}
	}
}

void Euclidian_Distance_Color_Match(
				SDL_Surface* Surface_32,
				SDL_Surface* Surface_8)
{
	uint8_t w_PaletteColor;
	Pxl_info_32 abgr;
	Pxl_err err;

	int w_smallest;

	int s;
	int t;
	int u;
	int v;
	int w;

	int pixel_idx = 0;

	for (int y = 0; y < Surface_32->h; y++)
	{
		for (int x = 0; x < Surface_32->w; x++)
		{
			int i = (Surface_32->w * y) + x;
			w_smallest = INT_MAX;
			memcpy(&abgr, (Pxl_info_32*)Surface_32->pixels + i, sizeof(Pxl_info_32));

			for (int j = 0; j < 256; j++)
			{
				s = abgr.r - PaletteColors[j].r;
				t = abgr.g - PaletteColors[j].g;
				u = abgr.b - PaletteColors[j].b;
				v = abgr.a - PaletteColors[j].a;

				s *= s;
				t *= t;
				u *= u;
				v *= v;
				w = sqrt(s + t + u + v);

				if (w < w_smallest) {
					w_smallest = w;
					w_PaletteColor = j;
				}

				if (j == 256 - 1)
				{
					err.r = abgr.r - PaletteColors[w_PaletteColor].r;
					err.g = abgr.g - PaletteColors[w_PaletteColor].g;
					err.b = abgr.b - PaletteColors[w_PaletteColor].b;
					err.a = abgr.a - PaletteColors[w_PaletteColor].a;

					int* pxl_index_arr[2];
					pxl_index_arr[0] = &x;
					pxl_index_arr[1] = &y;
						
					limit_dither(Surface_32, &err, pxl_index_arr);
				}
			}
			if (i == 100)		{ printf("loop #: %d\n", i); }
			if (i == 1000)		{ printf("loop #: %d\n", i); }
			if (i == 10000)		{ printf("loop #: %d\n", i); }
			if (i == 100000)	{ printf("loop #: %d\n", i); }
			if (i == 1000000)	{ printf("loop #: %d\n", i); }
			if (i == 2000000)	{ printf("loop #: %d\n", i); }

			((uint8_t*)Surface_8->pixels)[(Surface_8->pitch*y) + x] = w_PaletteColor;
		}
	}
}

void limit_dither(SDL_Surface *Surface_32,
	union Pxl_err *err,
	int **pxl_index_arr)
{
	int x = *pxl_index_arr[0];
	int y = *pxl_index_arr[1];

	if ((x - 1 < 0) || (x + 1 > Surface_32->w)) return;
	if (y + 1 > Surface_32->h) return;

	int pixel_index[4];
	pixel_index[0] = (Surface_32->w * (y + 0)) + (x + 1);
	pixel_index[1] = (Surface_32->w * (y + 1)) + (x - 1);
	pixel_index[2] = (Surface_32->w * (y + 1)) + (x + 0);
	pixel_index[3] = (Surface_32->w * (y + 1)) + (x + 1);

	int factor[4];
	factor[0] = 7;
	factor[1] = 3;
	factor[2] = 5;
	factor[3] = 1;

	for (int i = 0; i < 4; i++)
	{
		clamp_dither(Surface_32, err, pixel_index[i], factor[i]);
	}
}

void clamp_dither(SDL_Surface *Surface_32,
					union Pxl_err *err,
					int pixel_idx,
					int factor)
{	
	// pointer arrays so I can run a loop through them dependably
	uint8_t* pxl_surface [4];
	union Pxl_info_32 abgr;
	memcpy(&abgr, (Pxl_info_32*)Surface_32->pixels + pixel_idx, sizeof(Pxl_info_32));

	pxl_surface[0] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->a);
	pxl_surface[1] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->b);
	pxl_surface[2] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->g);
	pxl_surface[3] = &(((Pxl_info_32*)Surface_32->pixels + (pixel_idx))->r);

	// take palettized error and clamp values to max/min, then add error to appropriate pixel
	for (int i = 0; i < 4; i++)
	{
		// clamp 255 or 0
		int total_error = (abgr.arr[i] + err->arr[i] * factor / 16);

		if (total_error > 255)
		{	*pxl_surface[i] = 255;	}
		else
		if (total_error < 0)
		{	*pxl_surface[i] = 0;	}
		// if not clamp, then store error info
		else
			*pxl_surface[i] = total_error;
	}
}

// Convert FRM color to standard 32bit? maybe?
SDL_Surface* Load_FRM_Image(char *File_Name)
{
	SDL_Palette myPalette;
	SDL_PixelFormat pxlFMT_Pal;
	SDL_PixelFormat pxlFMT_UnPal;

	// File open stuff
	FILE *File_ptr = fopen(File_Name, "rb");
	fseek(File_ptr, 0x3E, SEEK_SET);

	uint16_t frame_width = 0;
	fread(&frame_width, 2, 1, File_ptr);
	frame_width = B_Endian::write_u16(frame_width);
	uint16_t frame_height = 0;
	fread(&frame_height, 2, 1, File_ptr);
	frame_height = B_Endian::write_u16(frame_height);
	uint32_t frame_size = 0;
	fread(&frame_size, 4, 1, File_ptr);
	frame_size = B_Endian::write_u32(frame_size);
	fseek(File_ptr, 0x4A, SEEK_SET);
	
	// 8 bit palleted stuff here
	myPalette.ncolors = 256;
	myPalette.colors = loadPalette("default");
	pxlFMT_Pal = *SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
	pxlFMT_Pal.palette = &myPalette;
	pxlFMT_Pal.BitsPerPixel = 8;
	pxlFMT_Pal.BytesPerPixel = 1;

	SDL_Surface* Paletted_Surface = SDL_CreateRGBSurface(0, frame_width, frame_height, 8, 0,0,0,0);
	Paletted_Surface = SDL_ConvertSurface(Paletted_Surface, &pxlFMT_Pal, 0);

	if (!Paletted_Surface) {
		printf("Error: %s\n", SDL_GetError());
	}

	// Read in the pixels from the FRM file to the surface
	fread(Paletted_Surface->pixels, frame_size, 1, File_ptr);

	// 32 bit pixel format stuff below
	SDL_Surface* Output_Surface = SDL_CreateRGBSurface(0, frame_width, frame_height, 32, 0, 0, 0, 0);
	pxlFMT_UnPal = *SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	pxlFMT_UnPal.palette = NULL;
	pxlFMT_UnPal.BitsPerPixel = 32;
	pxlFMT_UnPal.BytesPerPixel = 4; // BytesPerPixel = BitsPerPixel/8 (raised to nearest power of 2)

	// TODO: need to figure out what command is best to create a blank surface
	Output_Surface = SDL_ConvertSurface(Paletted_Surface, &pxlFMT_UnPal, 0);

	if (!Output_Surface) {
		printf("Error: %s", SDL_GetError());
	}
	return Output_Surface;
}

SDL_Surface* Display_Palettized_Image(SDL_Surface* Surface)
{
	uint16_t frame_width = Surface->w;
	uint16_t frame_height = Surface->h;

	SDL_PixelFormat pxlFMT_UnPal;
	SDL_Surface* Output_Surface;
	//Output_Surface = SDL_CreateRGBSurface(0, frame_width, frame_height, 32, 0, 0, 0, 0);
	pxlFMT_UnPal = *SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
	pxlFMT_UnPal.palette = NULL;
	pxlFMT_UnPal.BitsPerPixel = 32;
	pxlFMT_UnPal.BytesPerPixel = 4;
	Output_Surface = SDL_ConvertSurface(Surface, &pxlFMT_UnPal, 0);

	if (!Output_Surface) {
		printf("Error: %s", SDL_GetError());
	}
	return Output_Surface;
}

//// Might need to use templates or #define for something like this
//struct Pxl_Info* Pxl_Size_Selector(SDL_Surface* surface)
//{
//	if (surface->format->BitsPerPixel == 8)
//	{
//		struct Pxl_Info_8* Pxl_Infoa;
//		return (Pxl_Info*)Pxl_Infoa;
//	}
//	else
//	{
//		k = (surface->w)*(surface->h) / 4;
//	}
//}

void Palette_to_Texture()
{
	char buffer[512];
	FILE *palette_test;
	palette_test = fopen("Palette_Shader.vert", "rb");

	fread(buffer, sizeof(palette_test), 1, palette_test);
}