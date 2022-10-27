#include <stdio.h>
#include <cstdint>
#include <SDL.h>
#include <filesystem>
#include <Windows.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "B_Endian.h"
#include "Save_Files.h"
#include "tinyfiledialogs.h"
#include "imgui-docking/imgui.h"
#include "Load_Settings.h"
#include "MSK_Convert.h"

void Set_Default_Path(user_info* user_info);
void write_cfg_file(user_info* user_info);

char* Save_FRM(SDL_Surface *f_surface, user_info* user_info)
{
    FRM_Header FRM_Header;
    FRM_Header.version                = B_Endian::write_u32(4);
    FRM_Header.Frames_Per_Orientation = B_Endian::write_u16(1);
    FRM_Header.Frame_0_Height         = B_Endian::write_u16(f_surface->h);
    FRM_Header.Frame_0_Width          = B_Endian::write_u16(f_surface->w);
    FRM_Header.Frame_Area             = B_Endian::write_u32(f_surface->h * f_surface->w);
    FRM_Header.Frame_0_Size           = B_Endian::write_u32(f_surface->h * f_surface->w);

    FILE * File_ptr = NULL;
    char * Save_File_Name;
    char * lFilterPatterns[2] = { "", "*.FRM" };
    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        "temp001.FRM",
        2,
        lFilterPatterns,
        nullptr
    );
    if (Save_File_Name == NULL) {}
    else
    {
        //parse Save_File_Name to isolate the directory and save in default_save_path
        wchar_t* w_save_name = tinyfd_utf8to16(Save_File_Name);

        //std::filesystem::path p(Save_File_Name);
        std::filesystem::path p(w_save_name);

        strncpy(user_info->default_save_path, p.parent_path().string().c_str(), MAX_PATH);

        //TODO: check for existing file first
        //fopen_s(&File_ptr, Save_File_Name, "wb");
        _wfopen_s(&File_ptr, w_save_name, L"wb");

        if (!File_ptr) {
            tinyfd_messageBox(
                "Error",
                "Can not open this file in write mode",
                "ok",
                "error",
                1);
            return NULL;
        }
        else {
            fwrite(&FRM_Header, sizeof(FRM_Header), 1, File_ptr);
            //TODO: some image sizes are coming out weird again :(
            fwrite(f_surface->pixels, (f_surface->h * f_surface->w), 1, File_ptr);
            //TODO: also want to add animation frames

            fclose(File_ptr);
        }
    }

    return Save_File_Name;
}

char* Save_IMG(SDL_Surface *b_surface, user_info* user_info)
{
    char * Save_File_Name;
    char * lFilterPatterns[2] = { "*.BMP", "" };
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\temp001.bmp", user_info->default_save_path);

    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        buffer,
        2,
        lFilterPatterns,
        nullptr
    );

    if (!Save_File_Name) {}
    else
    {
        //TODO: check for existing file first
        SDL_SaveBMP(b_surface, Save_File_Name);

        //parse Save_File_Name to isolate the directory and store in default_save_path
        std::filesystem::path p(Save_File_Name);
        strncpy(user_info->default_save_path, p.parent_path().string().c_str(), MAX_PATH);
    }
    return Save_File_Name;
}

char* check_cfg_file(char* folder_name, user_info* user_info)
{
    FILE * config_file_ptr = NULL;
    //wide character stuff
    _wfopen_s(&config_file_ptr, L"config\\msk2bmpGUI.cfg", L"rb");

    if (!config_file_ptr) {
        folder_name = tinyfd_selectFolderDialog(NULL, folder_name);

        write_cfg_file(user_info);
        return folder_name;
    }
    else
    {
        fclose(config_file_ptr);
        if (strcmp(folder_name, ""))
        {
            folder_name = tinyfd_selectFolderDialog(NULL, folder_name);
            return folder_name;
        }
        else
        {
            folder_name = tinyfd_selectFolderDialog(NULL, NULL);
            return folder_name;
        }
    }
}

void Set_Default_Path(user_info* user_info)
{
    char *ptr = user_info->default_game_path;
    if (ptr) {
        ptr = check_cfg_file(ptr, user_info);
        if (!ptr) { return; }

        strcpy(user_info->default_game_path, ptr);
        if (!strcmp(user_info->default_save_path, "\0")) {
            strcpy(user_info->default_save_path, ptr);
        }
        write_cfg_file(user_info);
    }
    else {
        ptr = user_info->default_save_path;

        if (!ptr) {
            ptr = check_cfg_file(ptr, user_info);
            if (!ptr) { return; }
            strcpy(user_info->default_game_path, ptr);
            strcpy(user_info->default_save_path, ptr);
        }
        else {
            strcpy(user_info->default_game_path, ptr);
        }
        write_cfg_file(user_info);
    }
}

void Save_FRM_tiles(SDL_Surface *PAL_surface, user_info* user_info)
{
    FRM_Header FRM_Header;
    FRM_Header.version                = B_Endian::write_u32(4);
    FRM_Header.Frames_Per_Orientation = B_Endian::write_u16(1);
    FRM_Header.Frame_0_Height         = B_Endian::write_u16(300);
    FRM_Header.Frame_0_Width          = B_Endian::write_u16(350);
    FRM_Header.Frame_Area             = B_Endian::write_u32(300 * 350);
    FRM_Header.Frame_0_Size           = B_Endian::write_u32(300 * 350);

    Split_to_Tiles(PAL_surface, user_info, FRM, &FRM_Header);

    tinyfd_messageBox("Save Map Tiles", "Tiles Exported Successfully", "Ok", "info", 1);
}

void Save_Map_Mask(SDL_Surface* MSK_surface, struct user_info* user_info) {
    //TODO: export mask tiles using msk2bmp2020 code
    tinyfd_messageBox("Error", "Unimplemented, working on it", "Ok", "error", 1);
    Split_to_Tiles(MSK_surface, user_info, MSK, NULL);
}

void Split_to_Tiles(SDL_Surface *surface, struct user_info* user_info, img_type type, FRM_Header* frm_header)
{
    int num_tiles_x = surface->w / 350;
    int num_tiles_y = surface->h / 300;
    int tile_num = 0;
    char path[MAX_PATH];
    char Save_File_Name[MAX_PATH];


    FILE * File_ptr = NULL;


    if (!strcmp(user_info->default_game_path, "")) {
        Set_Default_Path(user_info);
        if (!strcmp(user_info->default_game_path, "")) { return; }
    }
    strncpy(path, user_info->default_game_path, MAX_PATH);

    for (int y = 0; y < num_tiles_y; y++)
    {
        for (int x = 0; x < num_tiles_x; x++)
        {
            char buffer[MAX_PATH];
            strncpy_s(buffer, MAX_PATH, Create_File_Name(type, path, tile_num, Save_File_Name), MAX_PATH);

            //check for existing file first
            check_file(type, File_ptr, path, buffer, tile_num, Save_File_Name);
            if (buffer == NULL) { return; }

            wchar_t* w_save_name = tinyfd_utf8to16(buffer);
            _wfopen_s(&File_ptr, w_save_name, L"wb");

            if (!File_ptr) {
                tinyfd_messageBox(
                    "Error",
                    "Can not open this file in write mode.\n"
                    "Make sure the default game path is set.",
                    "ok",
                    "error",
                    1);
                return;
            }
            else {
                // FRM = 1, MSK = 0
                if (type == FRM)
                {
                    //save header
                    fwrite(frm_header, sizeof(FRM_Header), 1, File_ptr);

                    int pixel_pointer = surface->pitch * y * 300 + x * 350;
                    for (int pixel_i = 0; pixel_i < 300; pixel_i++)
                    {
                        //write out one row of pixels in each loop
                        fwrite((uint8_t*)surface->pixels + pixel_pointer, 350, 1, File_ptr);
                        pixel_pointer += surface->pitch;
                    }
                    fclose(File_ptr);
                }
///////////////////////////////////////////////////////////////////////////
                if (type == MSK)
                {
                //Split the surface up into 350x300 pixel surfaces
                //      and pass them to Save_Mask()
                    Save_MSK_Image(surface, File_ptr, x, y);

///////////////////////////////////////////////////////////////////////////
                ///*Blit combination not supported :(
                                    /// looks like SDL can't convert anything to binary bitmap
                //SDL_Rect tile = { surface->pitch*y * 300, x * 350,
                                    // 350, 300 };
                                    //SDL_Rect dst = { 0,0, 350, 300 };
                                    //SDL_PixelFormat* pxlfmt = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX1MSB);
                                    //binary_bitmap = SDL_ConvertSurface(surface, pxlfmt, 0);
                                    //binary_bitmap = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_INDEX1MSB, 0);
                                    //printf(SDL_GetError());
                                    //int error = SDL_BlitSurface(surface, &tile, binary_bitmap, &dst);
                                    //if (error != 0)
                                    //{
                                    //    printf(SDL_GetError());
                                    //}
                }
            }
        }
    tile_num++;
    }
}

void check_file(img_type type, FILE* File_ptr, char* path, char* buffer, int tile_num, char* Save_File_Name)
{
    char * alt_path;
    char * lFilterPatterns[3] = { "*.FRM", "*.MSK", "" };

    wchar_t* w_save_name = tinyfd_utf8to16(buffer);
    errno_t error = _wfopen_s(&File_ptr, w_save_name, L"rb");

    //handles the case where the file exists
    if (error == 0) {
        fclose(File_ptr);

        int choice =
            tinyfd_messageBox(
                "Warning",
                "File already exists,\n"
                "Overwrite?",
                "yesnocancel",
                "warning",
                2);
        if      (choice == 0) { 
                    buffer = { 0 };
                    return; }
        else if (choice == 1) {}
        else if (choice == 2) {
            //TODO: check if this works
            alt_path = tinyfd_selectFolderDialog(NULL, path);
            strncpy_s(buffer, MAX_PATH, Create_File_Name(type, alt_path, tile_num, Save_File_Name), MAX_PATH);
        }
    }
    //handles the case where the DIRECTORY doesn't exist
    else {
        char* ptr;
        char temp[MAX_PATH];
        strncpy_s(temp, MAX_PATH, buffer, MAX_PATH);
        ptr = strrchr(temp, '\\');
        *ptr = '\0';

///////////////////////////////////////////////////////////////////////////
//another way to check if directory exists?
//#include <sys/stat.h> //stat 
//#include <stdbool.h>  //bool type 
        //bool file_exists(const char* filename) 
        //{ 
        //    struct stat buffer; 
        //    return (stat(filename, &buffer) == 0); 
        //}
///////////////////////////////////////////////////////////////////////////


        struct __stat64 stat_info; 
        error = _wstat64(tinyfd_utf8to16(temp), &stat_info);
        if (error == 0 && (stat_info.st_mode & _S_IFDIR) != 0)
        { 
            /* dir_path exists and is a directory */ 
            return;
        }
        else {
            int choice =
                tinyfd_messageBox(
                    "Warning",
                    "Directory doesnt exist. And escaping the apostrophe doesnt work :(\n"
                    "Choose a different location?",
                    "okcancel",
                    "warning",
                    2);
            if (choice == 0) {
                buffer = { 0 };
                return;
            }
            if (choice == 1) {
                //char name[13];
                //if (type == MSK) { strcpy(name, "wrldmp00.MSK"); }
                //if (type == FRM) { strcpy(name, "wrldmp00.FRM"); }
                //snprintf(path, MAX_PATH, name, MAX_PATH);

                char* save_file = tinyfd_saveFileDialog(
                    "Warning",
                    buffer,
                    2,
                    lFilterPatterns,
                    nullptr);

                if (save_file == NULL) {
                    buffer = { 0 };
                    return;
                }
                else {
                    //TODO: check if this works
                    strncpy_s(buffer, MAX_PATH, save_file, MAX_PATH);
                }
            }
        }
    }

}

// Help create a filename based on the directory and export file type
// bool type: FRM = 1, MSK = 0
char* Create_File_Name(img_type type, char* path, int tile_num, char* Save_File_Name)
{

    //-------save file
    //TODO: move tile_num++ outside if check
    if (type == FRM) {
        snprintf(Save_File_Name, MAX_PATH, "%s\\data\\art\\intrface\\wrldmp%02d.FRM", path, tile_num);
    }
    else
    if (type == MSK) {
        snprintf(Save_File_Name, MAX_PATH, "%s\\data\\data\\wrldmp%02d.MSK", path, tile_num);
    }

    // Wide character stuff follows...
     //return tinyfd_utf8to16(Save_File_Name);
    //just return the filename?
    return Save_File_Name;
}