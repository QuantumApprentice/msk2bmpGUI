#include <stdio.h>
#include <string.h>
#include <SDL_image.h>
#include <filesystem>

#include "Load_Files.h"
#include "Load_Settings.h"
#include "tinyfiledialogs.h"
#include "Image2Texture.h"
#include "FRM_Convert.h"
#include "MSK_Convert.h"

void Load_Files(LF* F_Prop, user_info* user_info, SDL_PixelFormat* pxlFMT)
{
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\", user_info->default_load_path);
    char * FilterPattern1[4] = { "*.bmp" , "*.png", "*.frm", "*.msk" };

    char *FileName = tinyfd_openFileDialog(
        "Open files...",
        buffer,
        4,
        FilterPattern1,
        NULL,
        1);

    if (!FileName) { return; }
    else {
        memcpy(F_Prop->Opened_File, FileName, MAX_PATH);
        F_Prop->c_name = strrchr(F_Prop->Opened_File, '/\\') + 1;
        F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

        std::filesystem::path p(FileName);
        strncpy(user_info->default_load_path, p.parent_path().string().c_str(), MAX_PATH);

        printf("extension: %s\n", F_Prop->extension);
        // TODO change strncmp to more secure varient when I figure out what that is :P
        if (!(strncmp (F_Prop->extension, "FRM", 4)))
        {
            F_Prop->image = Load_FRM_Image(F_Prop->Opened_File,
                                                   pxlFMT);
            F_Prop->type = FRM;
        }
        else if(!(strncmp (F_Prop->extension, "MSK", 4)))
        {
            //TODO: add something that tracks if it's MSK files when loading
            //      Also add something that _handles_ MSK files
            F_Prop->type = MSK;
            //F_Prop->Map_Mask = Load_MSK_Image(FileName);
            SDL_BlitSurface(Load_MSK_Image(FileName), NULL, F_Prop->Map_Mask, NULL);
        }
        else
        {
            F_Prop->image = IMG_Load(F_Prop->Opened_File);
            //TODO: add another type for other generic image types?
            //      (other than MSK files)
            F_Prop->type = other;
        }


        if (F_Prop->image == NULL)
        {
            printf("Unable to open image file %s! SDL Error: %s\n",
                F_Prop->Opened_File,
                SDL_GetError());
        }
        //else
        //{// Set display window to open
        //	F_Prop->file_open_window = true;
        //}
    }
}