#include <stdio.h>
#include <cstdint>
#include <SDL.h>
#include <filesystem>
#include <Windows.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "Save_Files.h"

#include "B_Endian.h"
#include "tinyfiledialogs.h"
#include "imgui-docking/imgui.h"
#include "Load_Settings.h"
#include "MSK_Convert.h"

void Set_Default_Path(user_info* user_info);
void write_cfg_file(user_info* user_info);

char* Save_FRM_SDL(SDL_Surface *f_surface, user_info* user_info)
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
            //issue is the alignment, SDL surfaces aligned at 4 pixels
            fwrite(f_surface->pixels, (f_surface->h * f_surface->w), 1, File_ptr);
            //TODO: also want to add animation frames

            fclose(File_ptr);
        }
    }

    return Save_File_Name;
}

char* Save_FRM_OpenGL(image_data* img_data, user_info* user_info)
{
    int width = img_data->width;
    int height = img_data->height;
    int size = width * height;

    FRM_Header FRM_Header;
    FRM_Header.version = B_Endian::write_u32(4);
    FRM_Header.Frames_Per_Orientation = B_Endian::write_u16(1);
    FRM_Header.Frame_0_Height = B_Endian::write_u16(height);
    FRM_Header.Frame_0_Width  = B_Endian::write_u16(width);
    FRM_Header.Frame_Area     = B_Endian::write_u32(size);
    FRM_Header.Frame_0_Size   = B_Endian::write_u32(size);

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

            //create buffer from texture and original FRM_data
            uint8_t* blend_buffer = blend_PAL_texture(img_data);

            //write to file
            fwrite(blend_buffer, size, 1, File_ptr);
            //TODO: also want to add animation frames

            fclose(File_ptr);
            free(blend_buffer);
        }
    }
    return Save_File_Name;
}

char* Save_IMG_SDL(SDL_Surface *b_surface, user_info* user_info)
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

//Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define TILE_W      (350)
#define TILE_H      (300)
#define TILE_SIZE   (350*300)

void Save_FRM_tiles_SDL(SDL_Surface *PAL_surface, user_info* user_info)
{
    FRM_Header FRM_Header;
    FRM_Header.version                = B_Endian::write_u32(4);
    FRM_Header.Frames_Per_Orientation = B_Endian::write_u16(1);
    FRM_Header.Frame_0_Height         = B_Endian::write_u16(TILE_H);
    FRM_Header.Frame_0_Width          = B_Endian::write_u16(TILE_W);
    FRM_Header.Frame_Area             = B_Endian::write_u32(TILE_SIZE);
    FRM_Header.Frame_0_Size           = B_Endian::write_u32(TILE_SIZE);

    //TODO: also need to test index 255 to see what color it shows in the engine
    //TODO: also need to create a toggle for transparency and maybe use index 255 for white instead (depending on if it works or not)
    Split_to_Tiles_SDL(PAL_surface, user_info, FRM, &FRM_Header);

    tinyfd_messageBox("Save Map Tiles", "Tiles Exported Successfully", "Ok", "info", 1);
}

void Save_FRM_Tiles_OpenGL(LF* F_Prop, user_info* user_info)
{
    FRM_Header FRM_Header;
    FRM_Header.version = B_Endian::write_u32(4);
    FRM_Header.Frames_Per_Orientation = B_Endian::write_u16(1);
    FRM_Header.Frame_0_Height = B_Endian::write_u16(TILE_H);
    FRM_Header.Frame_0_Width  = B_Endian::write_u16(TILE_W);
    FRM_Header.Frame_Area     = B_Endian::write_u32(TILE_SIZE);
    FRM_Header.Frame_0_Size   = B_Endian::write_u32(TILE_SIZE);

    //TODO: also need to test index 255 to see what color it shows in the engine (appears to be black on the menu)
    //TODO: also need to create a toggle for transparency and maybe use index 255 for white instead (depending on if it works or not)
    Split_to_Tiles_OpenGL(&F_Prop->edit_data, user_info, FRM, &FRM_Header);

    tinyfd_messageBox("Save Map Tiles", "Tiles Exported Successfully", "Ok", "info", 1);
}

void Save_MSK_Tiles_SDL(SDL_Surface* MSK_surface, struct user_info* user_info)
{
    //TODO: export mask tiles using msk2bmp2020 code
    tinyfd_messageBox("Error", "Unimplemented, working on it", "Ok", "error", 1);
    Split_to_Tiles_SDL(MSK_surface, user_info, MSK, NULL);
}
//wrapper to save MSK tiles
void Save_MSK_Tiles_OpenGL(image_data* img_data, struct user_info* user_info)
{
    //tinyfd_messageBox("Error", "Unimplemented, working on it", "Ok", "error", 1);
    Split_to_Tiles_OpenGL(img_data, user_info, MSK, NULL);
}

uint8_t* blend_PAL_texture(image_data* img_data)
{
    int img_size = img_data->width * img_data->height;

    //copy edited texture to buffer, combine with original image
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);

    //create a buffer
    uint8_t* texture_buffer = (uint8_t*)malloc(img_size);
    uint8_t* blend_buffer   = (uint8_t*)malloc(img_size);

    //read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    //combine edit data w/original image
    for (int i = 0; i < img_size; i++)
    {
        //TODO: add a switch for 255 to set blend_buffer[i] to 0
        if (texture_buffer[i] != 0) {
            blend_buffer[i] = texture_buffer[i];
        }
        else {
            blend_buffer[i] = img_data->FRM_data[i];
        }
    }
    free(texture_buffer);
    return blend_buffer;
}

void Split_to_Tiles_OpenGL(image_data* img_data, struct user_info* user_info, img_type type, FRM_Header* frm_header)
{
    int img_width  = img_data->width;
    int img_height = img_data->height;
    int img_size   = img_width * img_height;

    int num_tiles_x = img_width  / TILE_W;
    int num_tiles_y = img_height / TILE_H;
    int tile_num = 0;
    char path[MAX_PATH];
    char Save_File_Name[MAX_PATH];

    FILE * File_ptr = NULL;

    if (!strcmp(user_info->default_game_path, "")) {
        Set_Default_Path(user_info);
        if (!strcmp(user_info->default_game_path, "")) { return; }
    }
    strncpy(path, user_info->default_game_path, MAX_PATH);

    //create buffers for use in tiling
    uint8_t* blend_buffer = NULL;
    uint8_t* texture_buffer = (uint8_t*)malloc(img_size);
    if (type == FRM) {
        //create buffer from texture and original FRM_data
        blend_buffer = blend_PAL_texture(img_data);
    }
    else if (type == MSK) {
        //copy edited texture to buffer, combine with original image
        glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
        //read pixels into buffer
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);
    }

    //split buffer into tiles and write to files
    for (int y = 0; y < num_tiles_y; y++)
    {
        for (int x = 0; x < num_tiles_x; x++)
        {
            char filename_buffer[MAX_PATH];
            strncpy_s(filename_buffer, MAX_PATH, Create_File_Name(type, path, tile_num, Save_File_Name), MAX_PATH);

            //check for existing file first
            check_file(type, File_ptr, path, filename_buffer, tile_num, Save_File_Name);
            if (filename_buffer == NULL) { return; }

            wchar_t* w_save_name = tinyfd_utf8to16(filename_buffer);
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
                if (type == FRM) {
                    //Split buffer int 350x300 pixel tiles and write to file
                    //save header
                    fwrite(frm_header, sizeof(FRM_Header), 1, File_ptr);

                    int tile_pointer = (y * img_width*TILE_H) + (x * TILE_W);
                    int row_pointer = 0;

                    for (int i = 0; i < TILE_H; i++)
                    {
                        //write out one row of pixels in each loop
                        fwrite(blend_buffer + tile_pointer + row_pointer, TILE_W, 1, File_ptr);
                        row_pointer += img_width;
                    }
                }
                ///////////////////////////////////////////////////////////////////////////
                if (type == MSK)
                {
                    //Split the surface up into 350x300 pixel surfaces
                    //      and pass them to Save_MSK_Image_OpenGL()

                    int img_size = img_data->width * img_data->height;

                    //create buffers
                    uint8_t* tile_buffer    = (uint8_t*)malloc(TILE_SIZE);

                    int tile_pointer  = (y * img_width * TILE_H) + (x * TILE_W);
                    int img_row_pntr  = 0;
                    int tile_row_pntr = 0;

                    for (int i = 0; i < TILE_H; i++)
                    {
                        //copy out one row of pixels in each loop to the buffer
                        //fwrite(blend_buffer + tile_pointer + row_pointer, TILE_W, 1, File_ptr);
                        memcpy(tile_buffer + tile_row_pntr, texture_buffer + tile_pointer + img_row_pntr, TILE_W);

                        img_row_pntr  += img_width;
                        tile_row_pntr += TILE_W;
                    }

                    Save_MSK_Image_OpenGL(tile_buffer, File_ptr, TILE_W, TILE_H);

                    free(tile_buffer);
                }
                fclose(File_ptr);
            }
            tile_num++;
        }
    }
    if (blend_buffer) {
        free(blend_buffer);
    }
    if (texture_buffer) {
        free(texture_buffer);
    }
}

void Split_to_Tiles_SDL(SDL_Surface *surface, struct user_info* user_info, img_type type, FRM_Header* frm_header)
{
    int num_tiles_x = surface->w / TILE_W;
    int num_tiles_y = surface->h / TILE_H;
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

                    int pixel_pointer = surface->pitch * y * TILE_H + x * TILE_W;
                    for (int pixel_i = 0; pixel_i < TILE_H; pixel_i++)
                    {
                        //write out one row of pixels in each loop
                        fwrite((uint8_t*)surface->pixels + pixel_pointer, TILE_W, 1, File_ptr);
                        pixel_pointer += surface->pitch;
                    }
                    fclose(File_ptr);
                }
///////////////////////////////////////////////////////////////////////////
                if (type == MSK)
                {
                //Split the surface up into 350x300 pixel surfaces
                //      and pass them to Save_Mask()
                    Save_MSK_Image_SDL(surface, File_ptr, x, y);

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
                //Cancel =  null out buffer and return
                buffer = { 0 };
                return;
            }
            if (choice == 1) {
                //No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
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
    //TODO: is this necessary?
     //return tinyfd_utf8to16(Save_File_Name);
    //just return the filename?
    return Save_File_Name;
}

void Save_Full_MSK_OpenGL(image_data* img_data, user_info* usr_info)
{
    if (usr_info->save_full_MSK_warning) {
        tinyfd_messageBox(
            "Warning",
            "This is intended to allow you to save your progress only.\n"
            "Dimensions are currently not saved with this file format.\n\n"
            "To load a file saved this way,\n"
            "make sure to load the full map image first.\n\n"
            "To disable this warning,\n"
            "toggle this setting in the File menu.",
            "yesnocancel",
            "warning",
            2);
    }



    int texture_size = img_data->width * img_data->height;
    uint8_t* texture_buffer = (uint8_t*)malloc(texture_size);
    //copy edited texture to buffer, combine with original image
    glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
    //read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);


    //get filename
    FILE * File_ptr = NULL;
    char * Save_File_Name;
    char * lFilterPatterns[2] = { "*.MSK", "" };
    char save_path[MAX_PATH];

    snprintf(save_path, MAX_PATH, "%s\\temp001.MSK", usr_info->default_save_path);

    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        save_path,
        2,
        lFilterPatterns,
        nullptr
    );

    if (Save_File_Name == NULL) { return; }

    wchar_t* w_save_name = tinyfd_utf8to16(Save_File_Name);
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

    Save_MSK_Image_OpenGL(texture_buffer, File_ptr, img_data->width, img_data->height);

    fclose(File_ptr);
}

void Save_MSK_Image_SDL(SDL_Surface* surface, FILE* File_ptr, int x, int y)
{
    uint8_t out_buffer[13200] /*= { 0 }/* ceil(350/8) * 300 */;
    uint8_t *outp = out_buffer;

    int shift = 0;
    uint8_t bitmask = 0;
    bool mask_1_or_0;

    int pixel_pointer = surface->pitch * y * TILE_H + x * TILE_W;
    //don't need to flip for the MSK (maybe need to flip for bitmaps)
    for (int pxl_y = 0; pxl_y < TILE_H; pxl_y++)
    {
        for (int pxl_x = 0; pxl_x < TILE_W; pxl_x++)
        {
            bitmask <<= 1;
            mask_1_or_0 =
                *((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) > 0;
            //*((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) & 1;
            //*((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) > 0 ? 1 : 0;
            bitmask |= mask_1_or_0;
            if (++shift == 8)
            {
                *outp = bitmask;
                ++outp;
                shift = 0;
                bitmask = 0;
            }
        }
        bitmask <<= 2 /* final shift */;
        *outp = bitmask;
        ++outp;
        shift = 0;
        bitmask = 0;
    }
    writelines(File_ptr, out_buffer);
    fclose(File_ptr);
}

void Save_MSK_Image_OpenGL(uint8_t* tile_buffer, FILE* File_ptr, int width, int height)
{
    //int buff_size = ceil(width / 8.0f) * height;
    int buff_size = (width + 7) / 8 * height;

    //final output buffer
    uint8_t* out_buffer = (uint8_t*)malloc(buff_size);

    int shift = 0;
    uint8_t bitmask = 0;
    //bool mask_1_or_0;
    uint8_t* outp = out_buffer;
    for (int pxl_y = 0; pxl_y < height; pxl_y++)
    {
        for (int pxl_x = 0; pxl_x < width; pxl_x++)
        {
            //don't need to flip for MSK (maybe need to flip for bitmaps?)
            bitmask <<= 1;
            bitmask |= tile_buffer[pxl_x + pxl_y*width];
            if (++shift == 8)
            {
                *outp = bitmask;
                ++outp;
                shift = 0;
                bitmask = 0;
            }
        }
        //handle case where width doesn't fit 8-bits evenly
        if (shift > 0) {
            bitmask <<= (8 - shift); /* final shift at end of row */
            *outp = bitmask;
            ++outp;
            shift = 0;
            bitmask = 0;
        }
    }

    fwrite(out_buffer, buff_size, 1, File_ptr);
    free(out_buffer);
}
