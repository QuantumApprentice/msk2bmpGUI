#include "Load_Files.h"
#include "tinyfiledialogs.h"
#include <stdio.h>
#include <string.h>
#include <SDL_image.h>

void Load_Files(LF &F_Prop)
{
	F_Prop.Opened_File = tinyfd_openFileDialog(
		"Open files...",
		"",
		2,
		F_Prop.FilterPattern1,
		NULL,
		1);
	
	if (!F_Prop.Opened_File) {
		/*	tinyfd_messageBox(
			"Error",
			"No file opened...",
			"ok",
			"error",
			0);		*/
	}
	else {
		F_Prop.c_name = strrchr(F_Prop.Opened_File, '/\\') + 1;
		F_Prop.image1 = IMG_Load(F_Prop.Opened_File);
		if (F_Prop.image1 == NULL)
		{
			printf("Unable to open image file %s! SDL Error: %s\n",
				F_Prop.Opened_File,
				SDL_GetError());
		}
		else
		{//Convert surface to screen format
			F_Prop.file_open_window[0][0] = true;
		}
	}
}