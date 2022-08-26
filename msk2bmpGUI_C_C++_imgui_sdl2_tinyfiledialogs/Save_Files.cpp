#include <stdio.h>
#include <cstdint>
#include <SDL.h>
#include <filesystem>

#include "Save_Files.h"
#include "tinyfiledialogs.h"
#include "B_Endian.h"
#include "imgui-docking/imgui.h"
#include "Load_Settings.h"

void Set_Default_Path(user_info* user_info);
void write_cfg_file(user_info* user_info);

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


char* Save_FRM(SDL_Surface *f_surface)
{
	FRM_Header FRM_Header;
	FRM_Header.Frame_0_Height = B_Endian::write_u16(f_surface->h);
	FRM_Header.Frame_0_Width  = B_Endian::write_u16(f_surface->w);
	FRM_Header.Frame_Area	 = B_Endian::write_u32(f_surface->h * f_surface->w);
	FRM_Header.Frame_0_Size	 = B_Endian::write_u32(f_surface->h * f_surface->w);

	FILE * File_ptr = NULL;
	char * Save_File_Name;
	char * lFilterPatterns[2] = { "", "*.FRM" };
	Save_File_Name = tinyfd_saveFileDialog(
		"default_name",
		"temp001.FRM",
		2,
		lFilterPatterns,
		nullptr
	);
	if (Save_File_Name == NULL) {}
	else
	{
		if (!File_ptr) {
			tinyfd_messageBox(
				"Error",
				"Can not open this file in write mode",
				"ok",
				"error",
				1);
			//return 1;
		}

        fopen_s(&File_ptr, Save_File_Name, "wb");
        fwrite(&FRM_Header, sizeof(FRM_Header), 1, File_ptr);
		fwrite(f_surface->pixels, (f_surface->h * f_surface->w), 1, File_ptr);

		fclose(File_ptr);
	}

	return Save_File_Name;
}

void Save_FRM_tiles(SDL_Surface *PAL_surface, user_info* user_info)
{
	FRM_Header FRM_Header;
	FRM_Header.Frame_0_Height = B_Endian::write_u16(300);
	FRM_Header.Frame_0_Width =  B_Endian::write_u16(350);
	FRM_Header.Frame_Area =		B_Endian::write_u32(300 * 350);
	FRM_Header.Frame_0_Size =   B_Endian::write_u32(300 * 350);

	int num_tiles_x = PAL_surface->w / 350;
	int num_tiles_y = PAL_surface->h / 300;
	int q = 0;

	char Save_File_Name[256];
    char * folder_name = user_info->default_save_path;
	FILE * File_ptr = NULL;

    folder_name = tinyfd_selectFolderDialog(NULL, folder_name);

	if (folder_name == NULL) { return; }

	for (int y = 0; y < num_tiles_y; y++)
	{
		for (int x = 0; x < num_tiles_x; x++)
		{
			//-------save file
			sprintf(Save_File_Name, "%s\\wrldmp%02d.FRM", folder_name, q++);
			fopen_s(&File_ptr, Save_File_Name, "wb");

			if (!File_ptr) {
				tinyfd_messageBox(
					"Error",
					"Can not open this file in write mode",
					"ok",
					"error",
					1);
			}
			//save header
			fwrite(&FRM_Header, sizeof(FRM_Header), 1, File_ptr);

			int pixel_pointer = y * 300 * PAL_surface->pitch + x * 350;
			for (int pixel_i = 0; pixel_i < 350; pixel_i++)
			{
				//write out one row of pixels in each loop
				fwrite((uint8_t*)PAL_surface->pixels + pixel_pointer, 350, 1, File_ptr);
				pixel_pointer += PAL_surface->pitch;
			}
			fclose(File_ptr);
		}
	}
}

char* Save_IMG(SDL_Surface *b_surface, user_info* user_info)
{
	//FILE * File_ptr;
#define BUFFSIZE 256
	char * Save_File_Name;
	char * lFilterPatterns[2] = { "*.BMP", "" };
    char buffer[BUFFSIZE];
    snprintf(buffer, BUFFSIZE, "%s\\temp001.bmp", user_info->default_save_path);

	Save_File_Name = tinyfd_saveFileDialog(
		"default_name",
		buffer,
		2,
		lFilterPatterns,
		nullptr
	);

	if (!Save_File_Name) {}
	else
	{
		SDL_SaveBMP(b_surface, Save_File_Name);
        strcpy(user_info->default_save_path, Save_File_Name);
	}
	return Save_File_Name;
}

char* check_cfg_file(char* folder_name, user_info* user_info)
{
	FILE * config_file_ptr = NULL;
	fopen_s(&config_file_ptr, "config\\msk2bmpGUI.cfg", "rt");

	if (!config_file_ptr) {

		folder_name = tinyfd_selectFolderDialog(NULL, folder_name);

        write_cfg_file(user_info);
		return folder_name;
	}
	else
	{
        fclose(config_file_ptr);
		if (strcmp(folder_name, ""))
		{
			folder_name = tinyfd_selectFolderDialog(NULL, folder_name);
			return folder_name;
		}
		else
		{
			folder_name = tinyfd_selectFolderDialog(NULL, NULL);
			return folder_name;
		}
	}
}

void Set_Default_Path(user_info* user_info)
{
    char *ptr = user_info->default_game_path;
    if (ptr) {
        ptr = check_cfg_file(ptr, user_info);
        if (!ptr) { return; }

        strcpy(user_info->default_game_path, ptr);
        if (!strcmp(user_info->default_save_path, "\0")) {
            strcpy(user_info->default_save_path, ptr);
        }
        write_cfg_file(user_info);
    }
    else {
        ptr = user_info->default_save_path;

        if (!ptr) {
            ptr = check_cfg_file(ptr, user_info);
            if (!ptr) { return; }
            strcpy(user_info->default_game_path, ptr);
            strcpy(user_info->default_save_path, ptr);
        }
        else {
            strcpy(user_info->default_game_path, ptr);
        }
        write_cfg_file(user_info);
    }
}