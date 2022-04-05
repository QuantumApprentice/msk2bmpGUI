#include "Load_Files.h"
#include "tinyfiledialogs.h"
#include "Image2Texture.h"
#include "FRM_Convert.h"
#include <stdio.h>
#include <string.h>
#include <SDL_image.h>

void Load_Files(LF F_Prop[], int counter)
{
	char *ptr = tinyfd_openFileDialog(
		"Open files...",
		"",
		3,
		F_Prop[counter].FilterPattern1,
		NULL,
		1);
	
	if (!ptr) {
		/*	tinyfd_messageBox(
			"Error",
			"No file opened...",
			"ok",
			"error",
			0);		*/
	}
	else {
		memcpy(F_Prop[counter].Opened_File, ptr, 256);
		F_Prop[counter].c_name = strrchr(F_Prop[counter].Opened_File, '/\\') + 1;
		F_Prop[counter].extension = strrchr(F_Prop[counter].Opened_File, '.') + 1;

		printf("extension: %s\n", F_Prop[counter].extension);
		// TODO change strncmp to more secure varient when I figure out what that is :P
		if (!(strncmp (F_Prop[counter].extension, "FRM", 4)))
		{
			FRM_Load(F_Prop);
		}
		else
		{
			F_Prop[counter].image = IMG_Load(F_Prop[counter].Opened_File);
		}

		if (F_Prop[counter].image == NULL)
		{
			printf("Unable to open image file %s! SDL Error: %s\n",
				F_Prop[counter].Opened_File,
				SDL_GetError());
		}
		else
		{// Set display window to open
			F_Prop[counter].file_open_window = true;
		}
	}
}

void FRM_Load(LF *F_Prop)
{
	BMP_Color_Convert(F_Prop);

}