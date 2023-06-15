#include <stdio.h>
#include <string.h>
#include <SDL_image.h>
#include <Windows.h>

#include <filesystem>
#include <cstdint>
#include <system_error>
#include <execution>
#include <string_view>

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

bool open_multiple_files(std::set <std::filesystem::path> path_set,
                         LF* F_Prop, shader_info* shaders,
                         int* counter, int* window_number_focus)
{
    char buffer[MAX_PATH] = {};
    snprintf(buffer, MAX_PATH, "%s\nIs this a group of sequential animation frames?", tinyfd_utf16to8((*path_set.begin()).parent_path().c_str()));
    //returns 1 for yes, 2 for no, 0 for cancel
    int type = tinyfd_messageBox("Animation? or Single Images?",
                                 buffer, "yesnocancel", "question", 2);

    if (type == 2) {
        for (const std::filesystem::path& path : path_set) {
            F_Prop[*counter].file_open_window =
                Drag_Drop_Load_Files(tinyfd_utf16to8(path.c_str()),
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

        F_Prop[num].file_open_window = Drag_Drop_Load_Animation(path_set, &F_Prop[num]);

        if (!does_window_exist) {
            (*window_number_focus) = (*counter);
            (*counter)++;
        }

        return true;
    }
    else if (type == 0) {
        return false;
    }
}

char** handle_subdirectory_char(const std::filesystem::path& directory)
{
    char** array_of_paths = NULL;
    char* single_path = NULL;
    std::error_code error;
    int i = 0;

    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error) {
            printf("generic error message");
            return array_of_paths;
        }
        if (is_subdirectory) {
            //do nothing for now
            continue;
        }
        else {
            int str_length = strlen((char*)file.path().c_str());
            single_path = (char*)malloc(sizeof(char) * str_length);
            snprintf(single_path, str_length, "%s", file.path().c_str());

            array_of_paths[i] = single_path;
            i++;
        }
    }

    return array_of_paths;

}

std::vector <std::filesystem::path> handle_subdirectory_vec(const std::filesystem::path& directory)
{
    std::vector <std::filesystem::path> animation_images;
    std::error_code error;
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error) {
            //TODO: convert to tinyfd_filedialog() popup warning
            printf("error when checking if file_name is directory");
            return animation_images;
        }
        if (is_subdirectory) {
            //TODO: handle different directions in subdirectories?
            //handle_subdirectory(file.path());
            continue;
        }
        else {
            animation_images.push_back(file);
        }
    }

    LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    QueryPerformanceCounter(&StartingTime);

    //std::sort(animation_images.begin(), animation_images.end());                                        // ~50ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end());                   // ~50ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(), 
    //         [](std::filesystem::path& a, std::filesystem::path& b) 
    //          { return (a.filename() < b.filename()); });                                               // ~35ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(), 
    //         [](std::filesystem::path& a, std::filesystem::path& b) 
    //          { return (strcmp(a.filename().string().c_str(), b.filename().string().c_str())); });      // ~75ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(), 
    //         [](std::filesystem::path& a, std::filesystem::path& b) 
    //          { return (wcscmp(a.c_str(), b.c_str()) < 0); });                                          // ~10ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(), 
    //        [](std::filesystem::path& a, std::filesystem::path& b) 
    //         { return (a.native() < b.native()); });                                                    // ~17ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //    [](std::filesystem::path& a, std::filesystem::path& b)
    //{ return (a < b); });                                                                               // ~47ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(), 
    //         [](std::filesystem::path& a, std::filesystem::path& b) 
    //          { const wchar_t* a_file = wcspbrk(a.c_str(), L"/\\");
    //            const wchar_t* b_file = wcspbrk(b.c_str(), L"/\\");
    //            return (wcscmp(a_file, b_file) < 0); });                                                // ~13ms
    //size_t parent_path_size = directory.native().size();
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //           [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
    //            { 
    //              const wchar_t* a_file = a.c_str() + parent_path_size;
    //              const wchar_t* b_file = b.c_str() + parent_path_size;
    //              return (wcscmp(a_file, b_file) < 0);
    //            });                                                                                     // ~1ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //           [](std::filesystem::path& a, std::filesystem::path& b)
    //            {
    //                std::wstring_view a_v = a.native();
    //                std::wstring_view b_v = b.native();
    //                a_v = a_v.substr(a_v.find_last_of(L"\\/"));
    //                b_v = b_v.substr(b_v.find_last_of(L"\\/"));
    //                return (a_v < b_v);
    //            });                                                                                   // ~9ms
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //           [](std::filesystem::path& a, std::filesystem::path& b)
    //            {
    //                std::wstring_view a_v = a.native();
    //                std::wstring_view b_v = b.native();
    //                a_v = a_v.substr(a_v.find_last_of(L"\\/"));
    //                b_v = b_v.substr(b_v.find_last_of(L"\\/"));
    //                return (wcscmp(a_v.data(), b_v.data()) < 0);
    //            });                                                                                     // ~9ms
    //size_t parent_path_size = directory.native().size();
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //            [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
    //             {
    //                 std::wstring_view a_v = a.native();
    //                 std::wstring_view b_v = b.native();
    //                 a_v = a_v.substr(parent_path_size);
    //                 b_v = b_v.substr(parent_path_size);
    //                 return (a_v < b_v);
    //             });                                                                                    // ~2ms
    //size_t parent_path_size = directory.native().size();
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //            [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
    //             {
    //                 const std::wstring_view a_v = a.native().substr(parent_path_size);
    //                 const std::wstring_view b_v = b.native().substr(parent_path_size);
    //                 return (a_v < b_v);
    //             });                                                                                    // ~3ms



    QueryPerformanceCounter(&EndingTime);
    ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    ElapsedMicroseconds.QuadPart *= 1000000;
    ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

    printf("wstring_view 1_line parent_path_size(a_v < b_v) time: %d\n", ElapsedMicroseconds.QuadPart);

    return animation_images;
}

void next_prev_file(const std::filesystem::path& directory, char* next, char* prev, char* first, char* last, char* current)
{
    size_t parent_path_size = directory.native().size();
    wchar_t* w_current = tinyfd_utf8to16(current);
    //std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
//           [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
//            { 
//              const wchar_t* a_file = a.c_str() + parent_path_size;
//              const wchar_t* b_file = b.c_str() + parent_path_size;
//              return (wcscmp(a_file, b_file) < 0);
//            });
    const wchar_t* w_next;
    const wchar_t* w_prev;
    const wchar_t* w_frst;
    const wchar_t* w_last;
    const wchar_t* iter_file;
    std::error_code error;
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error) {
            //TODO: convert to tinyfd_filedialog() popup warning
            printf("error when checking if file_name is directory");
        }
        if (is_subdirectory) {
            //TODO: handle different directions in subdirectories?
            //handle_subdirectory(file.path());
            continue;
        }
        else {

            iter_file = (file.path().c_str() + parent_path_size);
            int cmp = wcscmp(iter_file, w_current);

            if (cmp < 0) {
                w_prev = iter_file;
            }
            else if (cmp > 0) {
                w_next = iter_file;
            }


        }
    }
}


std::set <std::filesystem::path> handle_subdirectory(const std::filesystem::path& directory)
{
    std::set <std::filesystem::path> animation_images;
    std::error_code error;
    for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error) {
            //TODO: convert to tinyfd_filedialog() popup warning
            printf("error when checking if file_name is directory");
            return animation_images;
        }
        if (is_subdirectory) {
            //TODO: handle different directions in subdirectories
            //handle_subdirectory(file.path());
            continue;
        }
        else {
            //char buffer[MAX_PATH] = {};


            //char* temp_name = strrchr((char*)(file.path().u8string().c_str()), '/\\');
            //snprintf(buffer, MAX_PATH, "%s", temp_name + 1);
            //std::filesystem::path temp_path(buffer);
            //animation_images.insert(temp_path);

            std::string u8file_path = file.path().u8string();
            char* temp_name = strrchr((char*)(u8file_path.c_str()), '/');
            if (!temp_name) {
                temp_name = strrchr((char*)(u8file_path.c_str()), '\\');
            }
            animation_images.insert(temp_name+1);


            //animation_images.insert(std::filesystem::relative(file.path(), directory));
        }
    }
    //std::sort(animation_images.begin(), animation_images.end());

    return animation_images;
}

//TODO: make a define switch for linux when I move to there
std::optional<bool> handle_directory_drop(char* file_name, LF* F_Prop, int* window_number_focus, int* counter,
                                          shader_info* shaders)
{
    char buffer[MAX_PATH];
    std::filesystem::path path(tinyfd_utf8to16(file_name));
    std::set <std::filesystem::path> animation_images;

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
                //handle different directions (NE/SE/NW/SW...etc) in subdirectories (1 level so far)
                std::set <std::filesystem::path> images = handle_subdirectory(file.path());
                if (!images.empty()) {
                    open_multiple_files(images, F_Prop, shaders, counter, window_number_focus);
                }

                continue;
            }
            else {
                animation_images.insert(file);
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


void prep_extension(LF* F_Prop, user_info* usr_info, char* file_name)
{
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", file_name);
    F_Prop->c_name = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    //TODO: clean up this function to work better for extensions
    //      maybe also support for wide characters?
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

    ///////////////////////////////////////////////////////
    //create an array of files in this directory for navigating through
    std::filesystem::path file_path(F_Prop->Opened_File);
    //F_Prop->file_set = handle_subdirectory(file_path.parent_path());
    //F_Prop->file_vec = handle_subdirectory_vec(file_path.parent_path());



    ///////////////////////////////////////////////////////

    if (usr_info != NULL) {
        snprintf(usr_info->default_load_path, MAX_PATH, "%s", file_path.parent_path().string().c_str());
    }
    //TODO: remove this printf
    printf("extension: %s\n", F_Prop->extension);
}

bool Drag_Drop_Load_Files(char* file_name, LF* F_Prop, image_data* img_data, shader_info* shaders)
{
    prep_extension(F_Prop, NULL, file_name);

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
        prep_extension(F_Prop, usr_info, FileName);

        return File_Type_Check(F_Prop, shaders, img_data);
    }
    else {
        return false;
    }
}

bool FRx_check(char* ext)
{
    if (
           !(strncmp(ext, "FRM", 4))
        || !(strncmp(ext, "FR0", 4))
        || !(strncmp(ext, "FR1", 4))
        || !(strncmp(ext, "FR2", 4))
        || !(strncmp(ext, "FR3", 4))
        || !(strncmp(ext, "FR4", 4))
        || !(strncmp(ext, "FR5", 4))
        ) {
        return true;
    }
    else {
        return false;
    }

}

bool File_Type_Check(LF* F_Prop, shader_info* shaders, image_data* img_data)
{
    // TODO change strncmp to more secure varient when I figure out what that is :P
    //FRx_check checks extension to make sure it's one of the FRM variants (FRM, FR0, FR1...FR5)
    if (FRx_check(F_Prop->extension))
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
        if (!F_Prop->img_data.ANM_dir) {
            printf("Unable to allocate memory for ANM_dir: %d", __LINE__);
        }
        else {
            new(img_data->ANM_dir) ANM_Dir[6];
        }

        F_Prop->img_data.ANM_dir->frame_data = (ANM_Frame*)malloc(sizeof(ANM_Frame));
        if (!F_Prop->img_data.ANM_dir->frame_data) {
            printf("Unable to allocate memory for ANM_Frame: %d", __LINE__);
        }
        else {
            new(img_data->ANM_dir->frame_data) ANM_Frame;
        }

        F_Prop->img_data.ANM_dir->frame_data->frame_start = IMG_Load(F_Prop->Opened_File);

        if (F_Prop->img_data.ANM_dir->frame_data->frame_start) {
            F_Prop->img_data.width  = F_Prop->img_data.ANM_dir->frame_data->frame_start->w;
            F_Prop->img_data.height = F_Prop->img_data.ANM_dir->frame_data->frame_start->h;

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
        else {
            printf("Unable to load image: %s", F_Prop->Opened_File);
            return false;
        }
    }

    //if ((F_Prop->IMG_Surface == NULL) && F_Prop->img_data.type != FRM && F_Prop->img_data.type != MSK)
    if (F_Prop->img_data.ANM_dir != NULL) {
        if ((F_Prop->img_data.ANM_dir->frame_data->frame_start == NULL) && F_Prop->img_data.type != FRM && F_Prop->img_data.type != MSK)
        {
            printf("Unable to open image file %s! SDL Error: %s\n",
                F_Prop->Opened_File,
                SDL_GetError());
            return false;
        }
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