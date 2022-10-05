#include <stdio.h>
#include <string.h>
#include <SDL_image.h>
#include <filesystem>

#include "Load_Files.h"
#include "Load_Settings.h"
#include "tinyfiledialogs.h"
#include "Image2Texture.h"
#include "FRM_Convert.h"

void Load_Files(LF F_Prop[], user_info* user_info, int counter, SDL_Color* palette)
{
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\", user_info->default_load_path);
    char * FilterPattern1[3] = { "*.bmp" , "*.png", "*.frm" };

    char *ptr = tinyfd_openFileDialog(
        "Open files...",
        buffer,
        3,
        FilterPattern1,
        NULL,
        1);

    if (!ptr) { return; }
    else {
        memcpy(F_Prop[counter].Opened_File, ptr, MAX_PATH);
        F_Prop[counter].c_name = strrchr(F_Prop[counter].Opened_File, '/\\') + 1;
        F_Prop[counter].extension = strrchr(F_Prop[counter].Opened_File, '.') + 1;

        std::filesystem::path p(ptr);
        strncpy(user_info->default_load_path, p.parent_path().string().c_str(), MAX_PATH);

        printf("extension: %s\n", F_Prop[counter].extension);
        // TODO change strncmp to more secure varient when I figure out what that is :P
        if (!(strncmp (F_Prop[counter].extension, "FRM", 4)))
        {
            F_Prop[counter].image = Load_FRM_Image(F_Prop[counter].Opened_File,
                                                   palette);
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