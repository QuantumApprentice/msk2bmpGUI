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

#include "display_FRM_OpenGL.h"

bool Load_Files(LF* F_Prop, image_data* img_data, struct user_info* user_info, shader_info* shaders)
{
    char load_path[MAX_PATH];
    snprintf(load_path, MAX_PATH, "%s\\", user_info->default_load_path);
    char * FilterPattern1[4] = { "*.bmp" , "*.png", "*.frm", "*.msk" };

    char *FileName = tinyfd_openFileDialog(
                     "Open files...",
                     load_path,
                     4,
                     FilterPattern1,
                     NULL,
                     1);

    //TODO: swap the if check so it returns false in an else instead
    if (!FileName) { return false; }

    memcpy(F_Prop->Opened_File, FileName, MAX_PATH);
    F_Prop->c_name    = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    //TODO: clean up this function to work better for extensions
    //      maybe also support for wide characters
    if (F_Prop->extension[0] > 96) {
        int i = 0;
        char buff[5];
        while (F_Prop->extension[i] != '\0')
        {
            buff[i] = toupper(F_Prop->extension[i]);
            i++;
        }
        buff[i] = '\0';
        F_Prop->extension = buff;
    }

    std::filesystem::path p(FileName);
    strncpy(user_info->default_load_path, p.parent_path().string().c_str(), MAX_PATH);
    //TODO: remove this printf
    printf("extension: %s\n", F_Prop->extension);

    return File_Type_Check(F_Prop, shaders, img_data);

}

bool File_Type_Check(LF* F_Prop, shader_info* shaders, image_data* img_data)
{
    // TODO change strncmp to more secure varient when I figure out what that is :P
    if (!(strncmp (F_Prop->extension, "FRM", 4)))
    {
        //The new way to load FRM images using openGL
        F_Prop->file_open_window = load_FRM_OpenGL(F_Prop->Opened_File, img_data);

        F_Prop->type = FRM;

        draw_FRM_to_framebuffer(shaders->palette,
                               &shaders->render_FRM_shader,
                               &shaders->giant_triangle,
                               img_data);
    }
    else if(!(strncmp (F_Prop->extension, "MSK", 4)))
    {
        F_Prop->file_open_window = Load_MSK_Tile_OpenGL(F_Prop->Opened_File, img_data);

        F_Prop->type = MSK;

        draw_MSK_to_framebuffer(shaders->palette,
                               &shaders->render_FRM_shader,
                               &shaders->giant_triangle,
                               img_data);

    }
    //do this for all other more common (generic) image types
    //TODO: add another type for other generic image types?
    else
    {   
        F_Prop->IMG_Surface     = IMG_Load(F_Prop->Opened_File);
        F_Prop->img_data.width  = F_Prop->IMG_Surface->w;
        F_Prop->img_data.height = F_Prop->IMG_Surface->h;

        F_Prop->type = other;

        Image2Texture(F_Prop->IMG_Surface,
                     &F_Prop->img_data.render_texture,
                     &F_Prop->file_open_window);
        F_Prop->img_data.height = F_Prop->IMG_Surface->h;
        F_Prop->img_data.width  = F_Prop->IMG_Surface->w;
    }

    if ((F_Prop->IMG_Surface == NULL) && F_Prop->type != FRM && F_Prop->type != MSK)
    {
        printf("Unable to open image file %s! SDL Error: %s\n",
            F_Prop->Opened_File,
            SDL_GetError());
        return false;
    }
    // Set display window to open
    return true;
}

//void Load_Edit_MSK_SDL(LF* F_Prop, user_info* user_info)
//{
//    char buffer[MAX_PATH];
//    snprintf(buffer, MAX_PATH, "%s\\", user_info->default_load_path);
//    char * FilterPattern1[1] = { "*.msk" };
//
//    char *FileName = tinyfd_openFileDialog(
//        "Open files...",
//        buffer,
//        1,
//        FilterPattern1,
//        NULL,
//        1);
//
//    if (!FileName) { return; }
//    else {
//        if (!(strncmp(F_Prop->extension, "MSK", 4)))
//        {
//            tinyfd_messageBox("Error",
//                "This window only opens .MSK files,\n"
//                "Please load other file types from\n"
//                "the main menu.",
//                "ok",
//                "warning",
//                1);
//            return;
//        }
//        else
//        {
//            F_Prop->Map_Mask = Load_MSK_Tile_SDL(FileName);
//            Image2Texture(F_Prop->Map_Mask,
//                &F_Prop->Optimized_Mask_Texture,
//                &F_Prop->edit_image_window);
//        }
//    }
//}