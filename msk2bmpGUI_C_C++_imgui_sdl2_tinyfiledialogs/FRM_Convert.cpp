#include "FRM_Convert.h"
#include "FRM_Animate.h"
#include "B_Endian.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <SDL.h>

struct Pxl_Info_32
{
	uint8_t a;
	uint8_t b;
	uint8_t g;
	uint8_t r;
}abgr{};

struct Pxl_Err
{
	int a;
	int b;
	int g;
	int r;
}err{ 0 };

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
	printf("Palette size: %d\n", 256);

	for (int i = 0; i < 256; i++)
	{
		uint8_t bytes[4];

		f.read((char*)bytes, 3);
		r = convert_colors(bytes[0]);
		g = convert_colors(bytes[1]);
		b = convert_colors(bytes[2]);
		//r = B_Endian::read_u8(f);
		//g = B_Endian::read_u8(f);
		//b = B_Endian::read_u8(f);
		//printf("RGB: %d, %d, %d\n", r, g, b);
		PaletteColors[i] = SDL_Color{ r, g, b };
	}
	return PaletteColors;
}

// Converts the color space to Fallout's paletted format
SDL_Surface* FRM_Color_Convert(SDL_Surface *surface)
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
	int k = 0;
	k = (surface->w)*(surface->h);

	myPalette.ncolors = 256;
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
	bool SDL = true;

	if (SDL == true) {
		SDL_Color_Match(Surface_32,
			k,
			&pxlFMT_Pal,
			Surface_8);
	}
	else
	{
		Euclidian_Distance_Color_Match(Surface_32, Surface_8);
	}
	return Surface_8;
}

void SDL_Color_Match(SDL_Surface* Surface_32,
					int k, 
					SDL_PixelFormat* pxlFMT_Pal,
					SDL_Surface* Surface_8)
{
	uint8_t w_PaletteColor;
	//struct Pxl_Info_32 {
	//	uint8_t a;
	//	uint8_t b;
	//	uint8_t g;
	//	uint8_t r;
	//}abgr{};

	// Convert image color to indexed palette
	for (int i = 0; i < k; i++)
	{
		memcpy(&abgr, (Pxl_Info_32*)Surface_32->pixels + i, sizeof(Pxl_Info_32));
		w_PaletteColor = SDL_MapRGBA(pxlFMT_Pal, abgr.r, abgr.g, abgr.b, abgr.a);


		if (i == 100) { printf("loop #: %d\n", i); }
		if (i == 1000) { printf("loop #: %d\n", i); }
		if (i == 10000) { printf("loop #: %d\n", i); }
		if (i == 100000) { printf("loop #: %d\n", i); }
		if (i == 1000000) { printf("loop #: %d\n", i); }
		if (i == 2000000) { printf("loop #: %d\n", i); }
		((uint8_t*)Surface_8->pixels)[i] = w_PaletteColor;
	}
}

void Euclidian_Distance_Color_Match(
				SDL_Surface* Surface_32,
				SDL_Surface* Surface_8)
{
	uint8_t w_PaletteColor;
	int w_smallest;


	int s;
	int t;
	int u;
	int v;
	int w;

	int pixel_idx = 0;

	for (int y = 0; y < Surface_32->h-1; y++)
	{
		for (int x = 0; x < Surface_32->w-1; x++)
		{
			int i = (Surface_32->w * y) + x;
			w_smallest = INT_MAX;
			memcpy(&abgr, (Pxl_Info_32*)Surface_32->pixels + i, sizeof(Pxl_Info_32));

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

				if (j == 255) 
				{
					err.r = abgr.r - PaletteColors[w_PaletteColor].r;
					err.g = abgr.g - PaletteColors[w_PaletteColor].g;
					err.b = abgr.b - PaletteColors[w_PaletteColor].b;
					err.a = abgr.a - PaletteColors[w_PaletteColor].a;

					if (x + 1 < Surface_32->w)
					{
						pixel_idx = (Surface_32->w * (y + 0)) + (x + 1);
						
						clamp_function(Surface_32, &err, pixel_idx, 7);
		
					}
					if ((x - 1 < 0) && (y + 1 < Surface_32->h))
					{
						pixel_idx = (Surface_32->w * (y + 1)) + (x - 1);

						clamp_function(Surface_32, &err, pixel_idx, 3);
					}
					if (y + 1 < Surface_32->h)
					{
						pixel_idx = (Surface_32->w * (y + 1)) + (x + 0);

						clamp_function(Surface_32, &err, pixel_idx, 5);
					}
					if ((x + 1 < Surface_32->w) && (y + 1 < Surface_32->h))
					{
					pixel_idx = (Surface_32->w * (y + 1)) + (x + 1);

					clamp_function(Surface_32, &err, pixel_idx, 1);
					}
				}
			}
			if (i == 100) { printf("loop #: %d\n", i); }
			if (i == 1000) { printf("loop #: %d\n", i); }
			if (i == 10000) { printf("loop #: %d\n", i); }
			if (i == 100000) { printf("loop #: %d\n", i); }
			if (i == 1000000) { printf("loop #: %d\n", i); }
			if (i == 2000000) { printf("loop #: %d\n", i); }
			((uint8_t*)Surface_8->pixels)[i] = w_PaletteColor;
		}
	}
}

void clamp_function(SDL_Surface *Surface_32, 
					struct Pxl_Err *err, 
					int pixel_idx,
					int factor)

{	
	memcpy(&abgr, (Pxl_Info_32*)Surface_32->pixels + pixel_idx, sizeof(Pxl_Info_32));
	//------------------ r
	if ((abgr.r + err->r * factor/16) > 255)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->r = 255;
	}
	else 
	if ((abgr.r + err->r * factor/16) < 0)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->r = 0;
	}
	else
	((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->r += err->r * factor / 16;
	
	//------------------ g
	if ((abgr.g + err->g * factor / 16) > 255)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->g = 255;
	}
	else
	if ((abgr.g + err->g * factor / 16) < 0)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->g = 0;
	}
	else
	((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->g += err->g * factor / 16;
	//------------------ b
	if ((abgr.b + err->b * factor / 16) > 255)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->b = 255;
	}
	else
	if ((abgr.b + err->b * factor / 16) < 0)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->b = 0;
	}
	else
	((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->b += err->b * factor / 16;
	//------------------ a
	if ((abgr.a + err->a * factor / 16) > 255)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->a = 255;
	}
	else
	if ((abgr.a + err->a * factor / 16) < 0)
	{
		((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->a = 0;
	}
	else
	((Pxl_Info_32*)Surface_32->pixels + (pixel_idx))->a += err->a * factor / 16;
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
	SDL_Surface* Output_Surface 
		= SDL_CreateRGBSurface(0, frame_width, frame_height, 32, 0, 0, 0, 0);
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