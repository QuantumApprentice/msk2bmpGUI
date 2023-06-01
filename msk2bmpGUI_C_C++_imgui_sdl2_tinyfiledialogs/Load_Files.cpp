#include <stdio.h>
#include <string.h>
#include <SDL_image.h>
#include <Windows.h>

#include <filesystem>
#include <cstdint>
#include <system_error>

#include "Load_Files.h"
#include "Load_Animation.h"
#include "Load_Settings.h"
#include "tinyfiledialogs.h"
#include "Image2Texture.h"
#include "FRM_Convert.h"
#include "MSK_Convert.h"
#include "Edit_Image.h"

#include "display_FRM_OpenGL.h"


char* Program_Directory()
{
    wchar_t buff[MAX_PATH];
    //char* utf8_buff = (char*)malloc(MAX_PATH*sizeof(char));

    GetModuleFileNameW(NULL, buff, MAX_PATH);

    char* utf8_buff = strdup(tinyfd_utf16to8(buff));
    //memcpy(utf8_buff, tinyfd_utf16to8(buff), MAX_PATH);

    char* ptr = strrchr(utf8_buff, '/\\') + 1;
    *ptr = '\0';

    //MessageBoxW(NULL,
    //    buff,
    //    L"string",
    //    MB_ABORTRETRYIGNORE);

    return utf8_buff;
}




void handle_file_drop(char* file_name, LF* F_Prop, int* counter, shader_info* shaders)
{
    F_Prop->file_open_window = Drag_Drop_Load_Files(file_name, F_Prop, &F_Prop->img_data, shaders);

    if (F_Prop->c_name) {
        (*counter)++;
    }
}

bool open_multiple_files(std::vector <std::filesystem::path> path_vector,
                         LF* F_Prop, shader_info* shaders,
                         int* counter, int* window_number_focus)
{
    char buffer[MAX_PATH] = {};
    snprintf(buffer, MAX_PATH, "%s\nIs this a group of sequential animation frames?", tinyfd_utf16to8(path_vector[0].parent_path().c_str()));
    //returns 1 for yes, 2 for no, 0 for cancel
    int type = tinyfd_messageBox("Animation? or Single Images?",
                                 buffer, "yesnocancel", "question", 2);

    if (type == 2) {
        for (int i = 0; i < path_vector.size(); i++)
        {
            F_Prop[*counter].file_open_window =
                Drag_Drop_Load_Files(tinyfd_utf16to8(path_vector[i].c_str()),
                                    &F_Prop[*counter],
                                    &F_Prop[*counter].img_data,
                                     shaders);
            (*counter)++;
        }
        return true;
    }
    else if (type == 1) {
        bool does_window_exist = (*window_number_focus > -1);
        int num = does_window_exist ? *window_number_focus : *counter;

        F_Prop[num].file_open_window = Drag_Drop_Load_Animation(path_vector, &F_Prop[num]);

        if (!does_window_exist) {
            (*counter)++;
            (*window_number_focus) = 0;
        }

        return true;
    }
    else if (type == 0) {
        return false;
    }
}


std::optional<std::vector <std::filesystem::path>> handle_subdirectory(const std::filesystem::path& directory)
{
    std::vector <std::filesystem::path> animation_images;
    std::error_code error;
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error) {
            //TODO: convert to tinyfd_filedialog() popup warning
            printf("error when checking if file_name is directory");
            return std::nullopt;
        }
        if (is_subdirectory) {
            //TODO: handle different directions in subdirectories
            //handle_subdirectory(file.path());
            continue;
        }
        else {
            animation_images.push_back(file);
        }
    }
    std::sort(animation_images.begin(), animation_images.end());

    return animation_images;
}

//TODO: make a define switch for linux when I move to there
std::optional<bool> handle_directory_drop(char* file_name, LF* F_Prop, int* window_number_focus, int* counter,
                                          shader_info* shaders)
{
    char buffer[MAX_PATH];
    std::filesystem::path path(tinyfd_utf8to16(file_name));
    std::vector <std::filesystem::path> animation_images;

    std::error_code error;
    bool is_directory = std::filesystem::is_directory(path, error);
    if (error) {
        tinyfd_notifyPopup("Error checking if dropped file is a directory",
                           "This usually happens when the text encoding is not UTF8/16.",
                           "info");
        printf("error checking if file_name is directory");
        return std::nullopt;
    }

    if (is_directory) {
        bool is_subdirectory = false;
        for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(path))
        {
            is_subdirectory = file.is_directory(error);
            if (error) {
                //TODO: convert to tinyfd_filedialog() popup warning
                printf("error when checking if file_name is directory");
                return std::nullopt;
            }
            if (is_subdirectory) {
                //handle different directions in subdirectories (1 level so far)
                std::optional<std::vector <std::filesystem::path>> images = handle_subdirectory(file.path());
                if (images.has_value() && !(*images).empty()) {
                    open_multiple_files(*images, F_Prop, shaders, counter, window_number_focus);
                }

                continue;
            }
            else {
                animation_images.push_back(file);
            }
        }
        if (is_subdirectory) {
            return true;
        }
        else {
            if (animation_images.empty()) {
                return false;
            }
            else {
                return open_multiple_files(animation_images, F_Prop, shaders, counter, window_number_focus);
            }
        }
    }
    else {
        return false;
    }
}


void prep_extension(LF* F_Prop, user_info* usr_info)
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
        snprintf(F_Prop->extension, 5, "%s", buff);
        //F_Prop->extension = buff;
    }

    if (usr_info != NULL) {
        std::filesystem::path file_path(F_Prop->Opened_File);
        snprintf(usr_info->default_load_path, MAX_PATH, "%s", file_path.parent_path().string().c_str());
        //strncpy(usr_info->default_load_path, file_path.parent_path().string().c_str(), MAX_PATH);
    }
    //TODO: remove this printf
    printf("extension: %s\n", F_Prop->extension);
}

bool Drag_Drop_Load_Files(char* file_name, LF* F_Prop, image_data* img_data, shader_info* shaders)
{
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", file_name);
    F_Prop->c_name = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    prep_extension(F_Prop, NULL);

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

        prep_extension(F_Prop, usr_info);

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

        F_Prop->img_data.type = FRM;

        draw_FRM_to_framebuffer(shaders->palette,
                               shaders->render_FRM_shader,
                               &shaders->giant_triangle,
                                img_data);
    }
    else if(!(strncmp (F_Prop->extension, "MSK", 4)))
    {
        F_Prop->file_open_window = Load_MSK_Tile_OpenGL(F_Prop->Opened_File, img_data);

        F_Prop->img_data.type = MSK;

        draw_MSK_to_framebuffer(shaders->palette,
                                shaders->render_FRM_shader,
                               &shaders->giant_triangle,
                                img_data);

    }
    //do this for all other more common (generic) image types
    //TODO: add another type for other generic image types?
    else
    {
        F_Prop->img_data.ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir)*6);
        F_Prop->img_data.ANM_dir->frame_data = (ANM_Frame*)malloc(sizeof(ANM_Frame));
        F_Prop->img_data.ANM_dir->frame_data->frame_start = IMG_Load(F_Prop->Opened_File);

        if (F_Prop->img_data.ANM_dir->frame_data->frame_start) {
            F_Prop->img_data.width  = F_Prop->img_data.ANM_dir->frame_data->frame_start->w;
            F_Prop->img_data.height = F_Prop->img_data.ANM_dir->frame_data->frame_start->h;}

        //F_Prop->IMG_Surface     = IMG_Load(F_Prop->Opened_File);
        //F_Prop->img_data.width  = F_Prop->IMG_Surface->w;
        //F_Prop->img_data.height = F_Prop->IMG_Surface->h;

        F_Prop->img_data.type = OTHER;
        
        Image2Texture(F_Prop->img_data.ANM_dir->frame_data->frame_start,
                     &F_Prop->img_data.render_texture,
                     &F_Prop->file_open_window);
        //Image2Texture(F_Prop->IMG_Surface,
        //             &F_Prop->img_data.render_texture,
        //             &F_Prop->file_open_window);
        //F_Prop->img_data.height = F_Prop->IMG_Surface->h;
        //F_Prop->img_data.width  = F_Prop->IMG_Surface->w;
    }

    //if ((F_Prop->IMG_Surface == NULL) && F_Prop->img_data.type != FRM && F_Prop->img_data.type != MSK)
    if ((F_Prop->img_data.ANM_dir->frame_data->frame_start == NULL) && F_Prop->img_data.type != FRM && F_Prop->img_data.type != MSK)
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