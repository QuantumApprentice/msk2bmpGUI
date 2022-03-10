#include "Load_Files.h"
#include "tinyfiledialogs.h"
#include <stdio.h>
#include <string.h>
#include <SDL_image.h>

void Load_Files(LF F_Prop[], int counter)
{
	F_Prop[counter].Opened_File = tinyfd_openFileDialog(
		"Open files...",
		"",
		2,
		F_Prop[counter].FilterPattern1,
		NULL,
		1);
	
	if (!F_Prop[counter].Opened_File) {
		/*	tinyfd_messageBox(
			"Error",
			"No file opened...",
			"ok",
			"error",
			0);		*/
	}
	else {
		F_Prop[counter].c_name = strrchr(F_Prop[counter].Opened_File, '/\\') + 1;
		F_Prop[counter].image = IMG_Load(F_Prop[counter].Opened_File);
		if (F_Prop[counter].image == NULL)
		{
			printf("Unable to open image file %s! SDL Error: %s\n",
				F_Prop[counter].Opened_File,
				SDL_GetError());
		}
		else
		{//Convert surface to screen format
			F_Prop[counter].file_open_window = true;
		}
	}
}