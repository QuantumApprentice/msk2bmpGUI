#include "FRM_Convert.h"
#include "FRM_Animate.h"
#include "Save_Files.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <SDL.h>

void Save_FRM(SDL_Surface *f_surface);
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


SDL_Surface* FRM_Convert(SDL_Surface *surface)
{
	struct abgr
	{
		uint8_t a;
		uint8_t b;
		uint8_t g;
		uint8_t r;
	} abgr_;
	
	//SDL_Surface another_Temp_Surface;
	//SDL_Surface* Temp_Surface = &another_Temp_Surface;
	
	SDL_Palette myPalette;
	SDL_PixelFormat pxlFMT;
	SDL_Surface* Temp_Surface;
	myPalette.ncolors = 256;
	myPalette.colors = loadPalette("default"); //PaletteColors;

	pxlFMT = *SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);
	pxlFMT.palette = &myPalette;
	pxlFMT.BitsPerPixel = 8;
	pxlFMT.BytesPerPixel = 1;
	int k = (surface->w)*(surface->h);

	//Temp_Surface = SDL_CreateRGBSurfaceFrom(surface->pixels, surface->w, surface->h, surface->format->BitsPerPixel, surface->pitch, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	Temp_Surface = SDL_ConvertSurface(surface, &pxlFMT, 0);
	//Temp_Surface = SDL_CreateRGBSurface(0, surface->w, surface->h, 8, 0, 0, 0, 0);

	if (!Temp_Surface) {
		printf("Error: %s", SDL_GetError());
	}

	for (int i = 0; i < k; i++) 
	{
		memcpy(&abgr_, (abgr*)surface->pixels + i, sizeof(abgr));
		int w_smallest = INT_MAX;
		uint8_t w_PaletteColor;
		w_PaletteColor = SDL_MapRGBA(&pxlFMT, abgr_.r, abgr_.g, abgr_.b, abgr_.a);
		//for (int j = 0; j < 256; j++)
		//{
		//	int s = (abgr_.r - PaletteColors[j].r);
		//	int t = (abgr_.g - PaletteColors[j].g);
		//	int u = (abgr_.b - PaletteColors[j].b);
		//	int v = (abgr_.a - PaletteColors[j].a);
		//	s *= s;
		//	t *= t;
		//	u *= u;
		//	v *= v;
		//	int w = sqrt(s + t + u + v);
		//	if (w < w_smallest) { 
		//		w_smallest = w; 
		//		w_PaletteColor = j;
		//	}
		//	//printf("stuv: %d %d %d %d", s, t, u, v);
		//}
		if (i == 100)		{ printf("loop #: %d\n", i); }
		if (i == 1000)		{ printf("loop #: %d\n", i); }
		if (i == 10000)		{ printf("loop #: %d\n", i); }
		if (i == 100000)	{ printf("loop #: %d\n", i); }
		if (i == 1000000)	{ printf("loop #: %d\n", i); }
		if (i == 2000000)	{ printf("loop #: %d\n", i); }
		((uint8_t*)Temp_Surface->pixels)[i]  = w_PaletteColor;
		//printf("w_PaletteColor: %d\n", w_PaletteColor);
	}
	
	//SDL_SaveBMP_RW(Temp_Surface, SDL_RWFromFile("temp1.bmp", "wb"), 1);
	Save_FRM(Temp_Surface);

	return Temp_Surface;
}

void Save_FRM(SDL_Surface *f_surface) {


	Save_Files(f_surface);




}

