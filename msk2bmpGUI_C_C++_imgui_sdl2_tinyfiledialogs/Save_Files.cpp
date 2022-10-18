#include <stdio.h>
#include <cstdint>
#include <SDL.h>
#include <filesystem>
#include <Windows.h>

#include "B_Endian.h"
#include "Save_Files.h"
#include "tinyfiledialogs.h"
#include "imgui-docking/imgui.h"
#include "Load_Settings.h"
#include "Save_Mask.h"

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

    Split_to_Tiles(PAL_surface, user_info, true, &FRM_Header);

    tinyfd_messageBox("Save Map Tiles", "Tiles Exported Successfully", "Ok", "info", 1);
}

void Save_Map_Mask(SDL_Surface* MSK_surface, struct user_info* user_info) {
    //TODO: export mask tiles using msk2bmp2020 code
    tinyfd_messageBox("Error", "Unimplemented, working on it", "Ok", "error", 1);
    FRM_Header* header = 0;
    Split_to_Tiles(MSK_surface, user_info, false, header);


}

void Split_to_Tiles(SDL_Surface *surface, struct user_info* user_info, bool type, FRM_Header* FRM_Header)
{
    int num_tiles_x = surface->w / 350;
    int num_tiles_y = surface->h / 300;
    int q = 0;

    char Save_File_Name[MAX_PATH];
    char path[MAX_PATH];
    char* alt_path;
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
            //check for existing file first
            //TODO: Make this section a helper function
            wchar_t* w_save_name = Create_File_Name(false, Save_File_Name, path, q);

            if (_wfopen_s(&File_ptr, w_save_name, L"rb")) {
                fclose(File_ptr);
                int choice =
                tinyfd_messageBox(
                        "Warning",
                        "File already exists,\n"
                        "Overwrite?",
                        "yesnocancel",
                        "warning",
                        2);
                if      (choice == 0) { return; }
                else if (choice == 1) {}
                else if (choice == 2) {
                    alt_path = tinyfd_selectFolderDialog(NULL, path);
                    //strncpy(path, alt_path, MAX_PATH);
                    //wchar_t* w_save_name = Create_File_Name(false, Save_File_Name, path, q);
                    wchar_t* w_save_name = Create_File_Name(false, Save_File_Name, alt_path, q);
                }
            }

            if (type == FRM)
            {
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
                    //save header
                    fwrite(&FRM_Header, sizeof(FRM_Header), 1, File_ptr);

                    int pixel_pointer = y * 300 * surface->pitch + x * 350;
                    for (int pixel_i = 0; pixel_i < 350; pixel_i++)
                    {
                        //write out one row of pixels in each loop
                        fwrite((uint8_t*)surface->pixels + pixel_pointer, 350, 1, File_ptr);
                        pixel_pointer += surface->pitch;
                    }
                    fclose(File_ptr);
                }
            }
            else
            if (type == MSK) {
                //const int total = (num_tiles_x*num_tiles_y);
                //SDL_Surface* Surface_Array = new SDL_Surface[total];
                SDL_Surface * binary_bitmap = 
                    SDL_CreateRGBSurfaceWithFormat(
                        SDL_SWSURFACE,
                        350, 300, 1, 
                        SDL_PIXELFORMAT_INDEX1MSB);

                int pixel_pointer = y * 300 * surface->pitch + x * 350;
                //TODO: split the surface up into 350x300 pixel surfaces
                //      and pass them to Save_Mask()
            //    for (int pixel_i = 0; pixel_i < 350; pixel_i++)
            //    {
                    //SDL_BlitSurface();
                    //Save_Mask();
            //        memccpy()
            //    }

            //    Save_Mask(false, );
            }
        }
    }
}

// Help create a filename based on the directory and export file type
wchar_t* Create_File_Name(bool type, char* Save_File_Name, char* path, int q)
{

    //-------save file
    //TODO: move q++ outside if check
    if (type == FRM) {
        snprintf(Save_File_Name, MAX_PATH, "%s\\data\\art\\intrface\\wrldmp%02d.FRM", path, q++);
    }
    else
    if (type == MSK) {
        snprintf(Save_File_Name, MAX_PATH, "%s\\data\\wrldmp%02d.MSK", path, q++);
    }

    // Wide character stuff follows...
     return tinyfd_utf8to16(Save_File_Name);
}