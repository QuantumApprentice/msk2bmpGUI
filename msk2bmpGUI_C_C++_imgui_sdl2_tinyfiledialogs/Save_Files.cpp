#include <stdio.h>
#include <cstdint>
#include <SDL.h>
#include "Save_Files.h"
#include "tinyfiledialogs.h"
#include "B_Endian.h"

#pragma pack(push, 1)
typedef struct {
	uint32_t version = B_Endian::write_u32(4);					// 0x0000
	uint16_t FPS = 0;						// 0x0004
	uint16_t Action_Frame = 0;				// 0x0006
	uint16_t Frames_Per_Orientation = B_Endian::write_u16(1);	// 0x0008
	int16_t  Shift_Orient_x[6]{};			// 0x000A
	int16_t  Shift_Orient_y[6]{};			// 0x0016
	uint32_t Frame_0_Offset[6]{};			// 0x0022
	uint32_t Frame_Area;					// 0x003A
	uint16_t Frame_0_Width;					// 0x003E
	uint16_t Frame_0_Height;				// 0x0040
	uint32_t Frame_0_Size;					// 0x0042
	uint16_t Shift_Offset_x = 0;			// 0x0046
	uint16_t Shift_Offset_y = 0;			// 0x0048
	//uint8_t  Color_Index = 0;				// 0x004A
} FRM_Header;
#pragma pack(pop)

char* Save_Files(SDL_Surface *f_surface)
{
	FRM_Header FRM_Stuff;
	FRM_Stuff.Frame_0_Height = B_Endian::write_u16(f_surface->h);
	FRM_Stuff.Frame_0_Width  = B_Endian::write_u16(f_surface->w);
	FRM_Stuff.Frame_Area	 = B_Endian::write_u32(f_surface->h * f_surface->w);
	FRM_Stuff.Frame_0_Size	 = B_Endian::write_u32(f_surface->h * f_surface->w);

	FILE * File_ptr;
	char * Save_File_Name;
	char * lFilterPatterns[2] = { "*.BMP", "*.FRM" };
	Save_File_Name = tinyfd_saveFileDialog(
		"default_name",
		"temp001.FRM",
		2,
		lFilterPatterns,
		nullptr
	);
	if (!Save_File_Name) {};

	fopen_s(&File_ptr, Save_File_Name, "wb");

	if (!File_ptr) {
		tinyfd_messageBox(
			"Error",
			"Can not open this file in write mode",
			"ok",
			"error",
			1);
		//return 1;
	}
	
	fwrite(&FRM_Stuff, sizeof(FRM_Stuff), 1, File_ptr);
	fwrite(f_surface->pixels, (f_surface->h * f_surface->w), 1, File_ptr);

	fclose(File_ptr);

	//printf("a: %s", Save_File_Name);
	return Save_File_Name;

}