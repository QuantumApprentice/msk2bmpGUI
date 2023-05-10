#include <stdio.h>
#include <string.h>
#include <SDL_image.h>
#include <filesystem>
#include <cstdint>
#include <system_error>

#include "Load_Files.h"
#include "Load_Settings.h"
#include "tinyfiledialogs.h"
#include "Image2Texture.h"
#include "FRM_Convert.h"
#include "MSK_Convert.h"
#include "Edit_Image.h"

#include "display_FRM_OpenGL.h"

//TODO: make a define switch for linux when I move to there
bool handle_directory(char* file_name, LF* F_Prop, int* counter, shader_info* shaders)
{
    char buffer[MAX_PATH];
    wchar_t* temp_ptr = tinyfd_utf8to16(file_name);
    std::filesystem::path path(temp_ptr);

    std::error_code error;
    bool is_directory = std::filesystem::is_directory(path, error);
    if (error) {
        //TODO: convert to tinyfdfiledialog() popup warning
        printf("error checking if file_name is directory");
        //TODO: handle differently, error means not just no directory, but something failed
        //std::optional?
        return false;
    }

    if (is_directory) {
        for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(path))
        {
            //is_directory = std::filesystem::is_directory(path, error);

            bool is_subdirectory = file.is_directory(error);
            if (error) {
                //TODO: convert to tinyfdfiledialog() popup warning
                printf("error checking if file_name is directory");
                continue;
            }
            if (is_subdirectory) {
                //TODO: possibly handle different directions in subdirectories
                continue;
            }
            //temp_ptr = tinyfd_utf8to16(file.path());

            F_Prop->file_open_window =
                Drag_Drop_Load_Files(file.path(),
                                    &F_Prop[*counter],
                                    &F_Prop->img_data,
                                     shaders);
            (*counter)++;
        }
        return true;
    }
    else {
        return false;
    }
}


void identify_extension(LF* F_Prop, user_info* usr_info)
{
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

    if (usr_info != NULL) {
        std::filesystem::path file_path(F_Prop->Opened_File);
        //TODO: snprintf
        strncpy(usr_info->default_load_path, file_path.parent_path().string().c_str(), MAX_PATH);
    }
    //TODO: remove this printf
    printf("extension: %s\n", F_Prop->extension);
}

bool Drag_Drop_Load_Files(std::filesystem::path file_name, LF* F_Prop, image_data* img_data, shader_info* shaders)
{
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", tinyfd_utf16to8(file_name.c_str()));
    F_Prop->c_name = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    identify_extension(F_Prop, NULL);

    return File_Type_Check(F_Prop, shaders, img_data);

}



bool Load_Files(LF* F_Prop, image_data* img_data, struct user_info* usr_info, shader_info* shaders)
{
    char load_path[MAX_PATH];
    snprintf(load_path, MAX_PATH, "%s\\", usr_info->default_load_path);
    char * FilterPattern1[5] = { "*.bmp" , "*.png", "*.frm", "*.msk", "*.jpg" };

    char *FileName = tinyfd_openFileDialog(
                     "Open files...",
                     load_path,
                     5,
                     FilterPattern1,
                     NULL,
                     1);

    if (FileName) {
        //if (strlen(FileName) < MAX_PATH) {
        //    memcpy(F_Prop->Opened_File, FileName, strlen(FileName));
        //}
        snprintf(F_Prop->Opened_File, MAX_PATH, "%s", FileName);
        F_Prop->c_name = strrchr(F_Prop->Opened_File, '/\\') + 1;
        F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

        identify_extension(F_Prop, usr_info);

        return File_Type_Check(F_Prop, shaders, img_data);
    }
    else {
        return false;
    }
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

        F_Prop->type = OTHER;

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