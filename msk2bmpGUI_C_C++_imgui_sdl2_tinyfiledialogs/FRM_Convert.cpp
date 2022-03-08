#include "FRM_Convert.h"
#include "B_Endian.h"
#include <cstdint>
#include <fstream>
#include <vector>
#include <SDL.h>

#pragma pack(push, 1)
typedef struct {
	uint32_t version = 0;					// 0x0000
	uint16_t FPS = 0;						// 0x0004
	uint16_t Action_Frame = 0;				// 0x0006
	uint16_t Frames_Per_Orientation = 0;	// 0x0008
	int16_t  Shift_Orient_x[6];				// 0x000A
	int16_t  Shift_Orient_y[6];				// 0x0016
	uint32_t Frame_0_Offset[6];				// 0x0022
	uint32_t Frame_Area;					// 0x003A
	uint16_t Frame_0_Width;					// 0x003E
	uint16_t Frame_0_Height;				// 0x0040
	uint32_t Frame_0_Size;					// 0x0042
	uint16_t Shift_Offset_x;				// 0x0046
	uint16_t Shift_Offset_y;				// 0x0048

} FRM_Header;
#pragma pack(pop)

//std::vector<SDL_Color> PaletteColors;
SDL_Color PaletteColors[256];
uint8_t convert_colors(uint8_t bytes) {
	if (bytes <= 64) {
		return 4 * bytes;
	}
	else {
		return bytes;
	}
}

SDL_Color* loadPalette()
{
	std::ifstream f("color.pal", 
		std::ios::in | std::ios::binary);
	if (!f.is_open()) {
		printf("Error opening color.pal.\n");
		return false;
	}
	uint8_t r, g, b;
	//PaletteColors.resize(256);
	printf("Palette size: %d\n", 256); // PaletteColors.size());
	for (int i = 0; i < 256; i++) //PaletteColors.size(); i++)
	{
		uint8_t bytes[4];

		f.read((char*)bytes, 3);
		r = convert_colors(bytes[0]);
		g = convert_colors(bytes[1]);
		b = convert_colors(bytes[2]);
		//r = B_Endian::read_u8(f);
		//g = B_Endian::read_u8(f);
		//b = B_Endian::read_u8(f);
		printf("RGB: %d, %d, %d\n", r, g, b);
		PaletteColors[i] = SDL_Color{ r, g, b };
	}



	return PaletteColors;
}


SDL_Surface* FRM_Convert(SDL_Surface *surface)
{
	loadPalette();
	struct rgba
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	} rgba_;
	
	//SDL_Surface another_Temp_Surface;
	//SDL_Surface* Temp_Surface = &another_Temp_Surface;
	
	SDL_Palette myPalette;
	SDL_PixelFormat pxlFMT;
	SDL_Surface* Temp_Surface;
	myPalette.ncolors = 256;
	myPalette.colors = PaletteColors;

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
		memcpy(&rgba_, (rgba*)surface->pixels + i, sizeof(rgba));
		int w_smallest = INT_MAX;
		int w_PaletteColor;
		w_PaletteColor = SDL_MapRGB(&pxlFMT, rgba_.r, rgba_.g, rgba_.b);
		//for (int j = 0; j < 256; j++)
		//{
		//	int s = (rgba_.r - PaletteColors[j].r);
		//	int t = (rgba_.g - PaletteColors[j].g);
		//	int u = (rgba_.b - PaletteColors[j].b);
		//	int v = (rgba_.a - PaletteColors[j].a);
		//	s *= s;
		//	t *= t;
		//	u *= u;
		//	v *= v;
		//	int w = sqrt(s + t + u + v);
		//	if (w < w_smallest) { 
		//		w_smallest = w; 
		//		w_PaletteColor = j;
		//	}
		//	printf("stuv: %d %d %d %d", s, t, u, v);
		//}
		((uint8_t*)Temp_Surface->pixels)[i]  = w_PaletteColor;
		//printf("w_PaletteColor: %d\n", w_PaletteColor);
	}
	
	SDL_SaveBMP_RW(Temp_Surface, SDL_RWFromFile("temp1.bmp", "wb"), 1);

	return Temp_Surface;
}