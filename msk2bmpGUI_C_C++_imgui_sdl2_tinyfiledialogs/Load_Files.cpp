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
#include "Edit_Image.h"

bool Load_Files(LF* F_Prop, user_info* user_info, SDL_PixelFormat* pxlFMT)
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

    //TODO: swap the if check so it returns false in an else instead
    if (!FileName) { return false; }

    memcpy(F_Prop->Opened_File, FileName, MAX_PATH);
    F_Prop->c_name = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    std::filesystem::path p(FileName);
    strncpy(user_info->default_load_path, p.parent_path().string().c_str(), MAX_PATH);
    //TODO: remove this printf
    printf("extension: %s\n", F_Prop->extension);
    // TODO change strncmp to more secure varient when I figure out what that is :P
    if (!(strncmp (F_Prop->extension, "FRM", 4)))
    {
        ////The old way to load FRM images using SDL_surfaces
        //F_Prop->image = Load_FRM_Image(F_Prop->Opened_File, pxlFMT);
        F_Prop->type = FRM;

        //The new way to load FRM images using openGL
        //init framebuffers and textures
        F_Prop->file_open_window = load_FRM_OpenGL(F_Prop->Opened_File, &F_Prop->img_data);

    }
    else if(!(strncmp (F_Prop->extension, "MSK", 4)))
        {
        //TODO: add something that tracks if it's MSK files when loading
        //      Also add something that _handles_ MSK files
            F_Prop->type = MSK;
            
            F_Prop->edit_image_window = true;
            F_Prop->file_open_window  = true;

            F_Prop->IMG_Surface = Load_MSK_Image(FileName);
            F_Prop->Map_Mask = Create_Map_Mask(F_Prop->IMG_Surface,
                                              &F_Prop->Optimized_Mask_Texture,
                                              &F_Prop->edit_image_window);
            SDL_BlitSurface(F_Prop->IMG_Surface, NULL, F_Prop->Map_Mask, NULL);

            printf(SDL_GetError());
            Image2Texture(F_Prop->Map_Mask,
                         &F_Prop->Optimized_Mask_Texture,
                         &F_Prop->edit_image_window);

            printf(SDL_GetError());
            Prep_Image(F_Prop, pxlFMT, false, &F_Prop->edit_map_mask);
            printf(SDL_GetError());

            //SDL_FreeSurface(temp);
            //SDL_to_OpenGl(F_Prop->Map_Mask, &F_Prop->Optimized_Texture);
        }

    //do this for all other more common (generic) image types
    else
    {   
        F_Prop->IMG_Surface = IMG_Load(F_Prop->Opened_File);
        F_Prop->img_data.width  = F_Prop->IMG_Surface->w;
        F_Prop->img_data.height = F_Prop->IMG_Surface->h;

        F_Prop->type = other;
    }
    //TODO: add another type for other generic image types?
    //      (other than MSK files)
    //TODO: add a check for MSK files here
    if ((F_Prop->IMG_Surface == NULL) && F_Prop->type != FRM)
    {
        printf("Unable to open image file %s! SDL Error: %s\n",
            F_Prop->Opened_File,
            SDL_GetError());
        return false;
    }
    // Set display window to open
    return true;
}

void Load_Edit_MSK(LF* F_Prop, user_info* user_info)
{
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\", user_info->default_load_path);
    char * FilterPattern1[1] = { "*.msk" };

    char *FileName = tinyfd_openFileDialog(
        "Open files...",
        buffer,
        1,
        FilterPattern1,
        NULL,
        1);

    if (!FileName) { return; }
    else {
        if (!(strncmp(F_Prop->extension, "MSK", 4)))
        {
            tinyfd_messageBox("Error",
                "This window only opens .MSK files,\n"
                "Please load other file types from\n"
                "the main menu.",
                "ok",
                "warning",
                1);
            return;
        }
        else
        {
            F_Prop->Map_Mask = Load_MSK_Image(FileName);
            Image2Texture(F_Prop->Map_Mask,
                &F_Prop->Optimized_Mask_Texture,
                &F_Prop->edit_image_window);
        }
    }
}