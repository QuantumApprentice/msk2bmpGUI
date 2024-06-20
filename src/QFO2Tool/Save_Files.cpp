#include <stdio.h>
#include <cstdint>
#include <SDL.h>
#include <SDL_image.h>
#include <filesystem>

#ifdef QFO2_WINDOWS
#include <Windows.h>
#elif defined(QFO2_LINUX)

#endif

#include <sys/types.h>
// #include <sys/stat.h>

#include "Save_Files.h"
#include "town_map_tiles.h"

#include "B_Endian.h"
#include "tinyfiledialogs.h"
#include "imgui.h"
#include "Load_Settings.h"
#include "MSK_Convert.h"
#include "platform_io.h"
#include "Edit_TILES_LST.h"
#include "tiles_pattern.h"

void write_cfg_file(user_info *user_info, char *exe_path);

char *Save_FRM_Image_OpenGL(image_data *img_data, user_info *user_info)
{
    int width = img_data->width;
    int height = img_data->height;
    int size = width * height;

    FRM_Header header;
    header.version = B_Endian::write_u32(4);
    header.Frames_Per_Orient = B_Endian::write_u16(1);

    // img_data->FRM_dir[0].frame_data[0]->Frame_Height = B_Endian::write_u16(height);
    // img_data->FRM_dir[0].frame_data[0]->Frame_Width  = B_Endian::write_u16(width);
    // img_data->FRM_dir[0].frame_data[0]->Frame_Size   = B_Endian::write_u32(size);

    FRM_Frame frame;
    frame.Frame_Height = B_Endian::write_u16(height);
    frame.Frame_Width = B_Endian::write_u16(width);
    frame.Frame_Size = B_Endian::write_u32(size);
    frame.Shift_Offset_x = 0;
    frame.Shift_Offset_y = 0;

    FILE *File_ptr = NULL;
    char *Save_File_Name;
    const char *lFilterPatterns[2] = {"", "*.FRM"};
    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        "temp001.FRM",
        2,
        lFilterPatterns,
        nullptr);
    if (Save_File_Name == NULL)
    {
    }
    else
    {

#ifdef QFO2_WINDOWS
        // parse Save_File_Name to isolate the directory and save in default_save_path for Windows (w/wide character support)
        wchar_t *w_save_name = tinyfd_utf8to16(Save_File_Name);
        std::filesystem::path p(w_save_name);
        strncpy(user_info->default_save_path, p.parent_path().string().c_str(), MAX_PATH);

        _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
        // parse Save_File_Name to isolate the directory and save in default_save_path for Linux
        std::filesystem::path p(Save_File_Name);
        strncpy(user_info->default_save_path, p.parent_path().string().c_str(), MAX_PATH);

        File_ptr = fopen(Save_File_Name, "wb");
#endif

        if (!File_ptr)
        {
            tinyfd_messageBox(
                "Error",
                "Can not open this file in write mode",
                "ok",
                "error",
                1);
            return NULL;
        }
        else
        {
            fwrite(&header, sizeof(FRM_Header), 1, File_ptr);
            fwrite(&frame, sizeof(FRM_Frame), 1, File_ptr);

            // create buffer from texture and original FRM_data
            uint8_t *blend_buffer = blend_PAL_texture(img_data);

            // write to file
            fwrite(blend_buffer, size, 1, File_ptr);
            // TODO: also want to add animation frames?

            fclose(File_ptr);
            free(blend_buffer);
        }
    }
    return Save_File_Name;
}

const char *Set_Save_Ext(image_data *img_data, int current_dir, int num_dirs)
{
    if (num_dirs > 1)
    {
        Direction *dir_ptr = NULL;
        dir_ptr = &img_data->FRM_dir[current_dir].orientation;
        assert(dir_ptr != NULL && "Not FRM or OTHER?");
        if (*dir_ptr > -1)
        {
            switch (*dir_ptr)
            {
            case (NE):
                return ".FR0";
            case (E):
                return ".FR1";
            case (SE):
                return ".FR2";
            case (SW):
                return ".FR3";
            case (W):
                return ".FR4";
            case (NW):
                return ".FR5";
            }
        }
    }
    return ".FRM";
}

//used by Save_FRM_Animation_OpenGL()
int Set_Save_Patterns(const char*** filter, image_data *img_data)
{
    int num_dirs = 0;
    for (int i = 0; i < 6; i++)
    {
        if (img_data->FRM_dir[i].orientation > -1)
        {
            num_dirs++;
        }
    }
    if (num_dirs < 6)
    {
        static const char *temp[7] = {"*.FR0", "*.FR1", "*.FR2", "*.FR3", "*.FR4", "*.FR5"};
        *filter = temp;
        return 6;
    }
    else
    {
        static const char *temp[1] = {"*.FRM"};
        *filter = temp;
        return 1;
    }
}

bool Save_Single_FRx_Animation_OpenGL(image_data *img_data, char *c_name, int dir)
{
    FILE *File_ptr = NULL;
    wchar_t *w_save_name = NULL;
    char *Save_File_Name = Set_Save_File_Name(img_data, c_name);
    if (!Save_File_Name)
    {
        return false;
    }

#ifdef QFO2_WINDOWS
    w_save_name = tinyfd_utf8to16(Save_File_Name);
    _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
    File_ptr = fopen(Save_File_Name, "wb");
#endif

    if (!File_ptr)
    {
        tinyfd_messageBox("Error",
                          "Unable to open this file in write mode",
                          "ok", "error", 1);
        return false;
    }

    bool success = Save_Single_Dir_Animation_OpenGL(img_data, File_ptr, img_data->display_orient_num);
    if (!success)
    {
        tinyfd_messageBox("Error",
                          "Problem saving file.",
                          "ok", "error", 1);
        return false;
    }
    else
    {
        fclose(File_ptr);
        return true;
    }
}

bool Save_Single_Dir_Animation_OpenGL(image_data *img_data, FILE *File_ptr, int dir)
{
    int size = 0;
    uint32_t total_frame_size = 0;
    FRM_Frame frame_data;
    memset(&frame_data, 0, sizeof(FRM_Frame));

    fseek(File_ptr, sizeof(FRM_Header), SEEK_SET);

    // Write out all the frame data
    for (int frame_num = 0; frame_num < img_data->FRM_dir[dir].num_frames; frame_num++)
    {
        size = img_data->FRM_dir[dir].frame_data[frame_num]->Frame_Size;
        total_frame_size += size + sizeof(FRM_Frame);

        memcpy(&frame_data, img_data->FRM_dir[dir].frame_data[frame_num], sizeof(FRM_Frame));
        B_Endian::flip_frame_endian(&frame_data);

        // write to file
        fwrite(&frame_data, sizeof(FRM_Frame), 1, File_ptr);
        fwrite(&img_data->FRM_dir[dir].frame_data[frame_num]->frame_start, size, 1, File_ptr);
    }

    // If directions are split up then write to header for each one and close file
    img_data->FRM_hdr->Frame_Area = total_frame_size;

    FRM_Header header = {};
    memcpy(&header, img_data->FRM_hdr, sizeof(FRM_Header));
    B_Endian::flip_header_endian(&header);
    fseek(File_ptr, 0, SEEK_SET);
    fwrite(&header, sizeof(FRM_Header), 1, File_ptr);

    return true;
}

char* Set_Save_File_Name(image_data* img_data, char* name)
{
    char *Save_File_Name;
    int num_patterns = 6;
    static const char *const lFilterPatterns[6] = {"*.FR0", "*.FR1", "*.FR2", "*.FR3", "*.FR4", "*.FR5"};
    const char *ext = Set_Save_Ext(img_data, img_data->display_orient_num, num_patterns);
    int buffsize = strlen(name) + 5;
    char *temp_name = (char *)malloc(sizeof(char) * buffsize);
    snprintf(temp_name, buffsize, "%s%s", name, ext);

    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        temp_name,
        num_patterns,
        lFilterPatterns,
        nullptr);
    free(temp_name);

    return Save_File_Name;
}

char *Save_FRx_Animation_OpenGL(image_data *img_data, char *default_save_path, char *name)
{
    char *Save_File_Name = Set_Save_File_Name(img_data, name);

    if (Save_File_Name != NULL)
    {
        // parse Save_File_Name to isolate the directory and save in default_save_path
        std::filesystem::path p(Save_File_Name);
        strncpy(default_save_path, p.parent_path().string().c_str(), MAX_PATH);

        for (int i = 0; i < 6; i++)
        {
            // Check if current direction contains valid data
            Direction dir = img_data->FRM_dir[i].orientation;
            if (dir < 0)
            {
                continue;
            }
            assert((dir > -1) && "Why is dir==-1 ?");

            FILE *File_ptr = nullptr;
            // If directions are split up then open new file for each direction
            Save_File_Name[strlen(Save_File_Name) - 1] = '0' + dir;

#ifdef QFO2_WINDOWS
            wchar_t *w_save_name = tinyfd_utf8to16(Save_File_Name);
            _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
            File_ptr = fopen(Save_File_Name, "wb");
#endif

            if (!File_ptr)
            {
                tinyfd_messageBox(
                    "Error",
                    "Unable to open this file in write mode",
                    "ok",
                    "error",
                    1);
                return NULL;
            }
            // else {
            //     fseek(File_ptr, sizeof(FRM_Header), SEEK_SET);
            // }

            bool success = Save_Single_Dir_Animation_OpenGL(img_data, File_ptr, dir);
            if (!success)
            {
                return NULL;
            }

            fclose(File_ptr);
            File_ptr = nullptr;
        }
    }

    return Save_File_Name;
}

char *Save_FRM_Animation_OpenGL(image_data *img_data, user_info *usr_info, char *name)
{
    FILE *File_ptr = NULL;
    char *Save_File_Name;
    const char ** lFilterPatterns = NULL;
    int num_patterns = Set_Save_Patterns(&lFilterPatterns, img_data);
    const char *ext = Set_Save_Ext(img_data, img_data->display_orient_num, num_patterns);
    int buffsize = strlen(name) + 5;

    char *temp_name = (char *)malloc(sizeof(char) * buffsize);
    snprintf(temp_name, buffsize, "%s%s", name, ext);

    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        temp_name,
        num_patterns,
        lFilterPatterns,
        nullptr);
    free(temp_name);

    if (Save_File_Name != NULL)
    {
        if (num_patterns > 1)
        {
            Save_File_Name[strlen(Save_File_Name) - 1] = '0';
        }

        FRM_Header header = {};
        img_data->FRM_hdr->version = 4;

#ifdef QFO2_WINDOWS
        // Windows w/wide character support
        // parse Save_File_Name to isolate the directory and save in default_save_path
        wchar_t *w_save_name = tinyfd_utf8to16(Save_File_Name);
        std::filesystem::path p(w_save_name);
        _wfopen_s(&File_ptr, w_save_name, L"wb"); // wide
#elif defined(QFO2_LINUX)
        std::filesystem::path p(Save_File_Name);
        File_ptr = fopen(Save_File_Name, "wb");
#endif

        strncpy(usr_info->default_save_path, p.parent_path().string().c_str(), MAX_PATH);
        fseek(File_ptr, sizeof(FRM_Header), SEEK_SET);

        if (!File_ptr)
        {
            tinyfd_messageBox(
                "Error",
                "Can not open this file in write mode",
                "ok",
                "error",
                1);
            return NULL;
        }
        else
        {

            FRM_Frame frame_data;
            memset(&frame_data, 0, sizeof(FRM_Frame));
            int size = 0;
            uint32_t total_frame_size = 0;

            for (int i = 0; i < 6; i++)
            {
                // Check if current direction contains valid data
                Direction dir = img_data->FRM_dir[i].orientation;
                if (dir < 0)
                {
                    continue;
                }
                // If directions are split up then open new file for each direction
                if (num_patterns > 1)
                {
                    Save_File_Name[strlen(Save_File_Name) - 1] = '0' + dir;

                    if (!File_ptr)
                    {
#ifdef QFO2_WINDOWS
                        // Windows w/wide character support
                        w_save_name = tinyfd_utf8to16(Save_File_Name);
                        _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
                        File_ptr = fopen(Save_File_Name, "wb");
#endif

                        if (!File_ptr)
                        {
                            tinyfd_messageBox(
                                "Error",
                                "Unable to open this file in write mode",
                                "ok",
                                "error",
                                1);
                            return NULL;
                        }
                        else
                        {
                            fseek(File_ptr, sizeof(FRM_Header), SEEK_SET);
                        }
                    }
                }
                else
                {
                    img_data->FRM_hdr->Frame_0_Offset[i] = total_frame_size;
                }

                // Write out all the frame data
                for (int frame_num = 0; frame_num < img_data->FRM_dir[i].num_frames; frame_num++)
                {
                    size = img_data->FRM_dir[i].frame_data[frame_num]->Frame_Size;
                    total_frame_size += size + sizeof(FRM_Frame);

                    memcpy(&frame_data, img_data->FRM_dir[i].frame_data[frame_num], sizeof(FRM_Frame));
                    B_Endian::flip_frame_endian(&frame_data);

                    // write to file
                    fwrite(&frame_data, sizeof(FRM_Frame), 1, File_ptr);
                    fwrite(&img_data->FRM_dir[i].frame_data[frame_num]->frame_start, size, 1, File_ptr);
                }

                // If directions are split up then write to header for each one and close file
                if (num_patterns > 1)
                {
                    img_data->FRM_hdr->Frame_Area = total_frame_size;
                    memcpy(&header, img_data->FRM_hdr, sizeof(FRM_Header));
                    B_Endian::flip_header_endian(&header);
                    fseek(File_ptr, 0, SEEK_SET);
                    fwrite(&header, sizeof(FRM_Header), 1, File_ptr);
                    fclose(File_ptr);

                    File_ptr = nullptr;
                    total_frame_size = 0;
                }
            }
            // If full FRM then write header this way instead
            if (num_patterns == 1)
            {
                img_data->FRM_hdr->Frame_Area = total_frame_size;
                memcpy(&header, img_data->FRM_hdr, sizeof(FRM_Header));
                B_Endian::flip_header_endian(&header);
                fseek(File_ptr, 0, SEEK_SET);
                fwrite(&header, sizeof(FRM_Header), 1, File_ptr);
                fclose(File_ptr);
            }
        }
    }
    return Save_File_Name;
}

char *Save_IMG_SDL(SDL_Surface *b_surface, user_info *user_info)
{
    char *Save_File_Name;
    const char *lFilterPatterns[2] = {"*.BMP", ""};
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\temp001.bmp", user_info->default_save_path);

    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        buffer,
        2,
        lFilterPatterns,
        nullptr);

    if (!Save_File_Name)
    {
    }
    else
    {
        // TODO: check for existing file first
        SDL_SaveBMP(b_surface, Save_File_Name);
        // TODO: add support for more file formats (GIF in particular)
        // IMG_SavePNG();

        // parse Save_File_Name to isolate the directory and store in default_save_path
        std::filesystem::path p(Save_File_Name);
        strncpy(user_info->default_save_path, p.parent_path().string().c_str(), MAX_PATH);
    }
    return Save_File_Name;
}

//checks if msk2bmpGUI.cfg exists,
//if it doesn't, creates the file (including folder)
//then it writes current settings to cfg file
bool check_and_write_cfg_file(user_info *user_info, char *exe_path)
{
    char cfg_filepath_buffer[MAX_PATH];
    char cfg_path_buffer[MAX_PATH];
    snprintf(cfg_filepath_buffer, sizeof(cfg_filepath_buffer), "%s%s", exe_path, "config/msk2bmpGUI.cfg");
    snprintf(cfg_path_buffer, sizeof(cfg_path_buffer), "%s%s", exe_path, "config/");

    FILE *cfg_file_ptr = NULL;

#ifdef QFO2_WINDOWS
    // Windows w/wide character support
    _wfopen_s(&config_file_ptr, tinyfd_utf8to16(path_buffer), L"rb");
#elif defined(QFO2_LINUX)
    cfg_file_ptr = fopen(cfg_filepath_buffer, "rb");
#endif

    if (!cfg_file_ptr)
    {
        // if the directory its supposed to be in exists...
        if (io_isdir(cfg_path_buffer))
        {
            // make the file
            cfg_file_ptr = fopen(cfg_filepath_buffer, "wb");
            printf("error opening cfg file: %s\n", strerror(errno));
        }
        else
        {
            // create the directory first, then make the file
            if (io_make_dir(cfg_path_buffer))
            {
                cfg_file_ptr = fopen(cfg_filepath_buffer, "wb");
                if (!cfg_file_ptr) {
                    printf("error opening cfg file: %s\n", strerror(errno));
                    return false;
                }
            }
            else
            {
                printf("error opening cfg file: %s\n", strerror(errno));
                return false;
            }
        }
    }

    write_cfg_file(user_info, exe_path);
    fclose(cfg_file_ptr);
    return true;
}

//Ask user where the default Fallout 2 path is,
//then store path in both default_game_path and default_save_path if default_save_path is '\0'
//then write user_info out to config file
void Set_Default_Game_Path(user_info *usr_info, char *exe_path)
{
    //Set the default Fallout 2 game path
    bool path_set = export_auto(usr_info, exe_path, NULL, UNK);
    if (!path_set) {
        return;
    }
}

// Fallout map tile size hardcoded in engine to 350x300 pixels WxH
#define MAP_TILE_W (350)
#define MAP_TILE_H (300)
#define MAP_TILE_SIZE (350 * 300)

void Save_FRM_Tiles_OpenGL(LF *F_Prop, user_info *user_info, char *exe_path)
{
    FRM_Header FRM_Header = {};
    FRM_Header.version = (4);
    FRM_Header.FPS = (1);
    FRM_Header.Frames_Per_Orient = (1);
    FRM_Header.Frame_Area = (MAP_TILE_SIZE);

    B_Endian::flip_header_endian(&FRM_Header);

    // TODO: also need to test index 255 to see what color it shows in the engine (appears to be black on the menu)
    // TODO: need to color pick for transparency and maybe use index 255 for white instead (depending on if it works or not)
    Split_to_Tiles_OpenGL(&F_Prop->edit_data, user_info, FRM, &FRM_Header, exe_path);

    tinyfd_messageBox("Save Map Tiles", "Tiles Exported Successfully", "Ok", "info", 1);
}

// wrapper to save MSK tiles
void Save_MSK_Tiles_OpenGL(image_data *img_data, struct user_info *user_info, char *exe_path)
{
    // tinyfd_messageBox("Error", "Unimplemented, working on it", "Ok", "error", 1);
    Split_to_Tiles_OpenGL(img_data, user_info, MSK, NULL, exe_path);
}

uint8_t *blend_PAL_texture(image_data *img_data)
{
    int img_size = img_data->width * img_data->height;

    // copy edited texture to buffer, combine with original image
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);

    // create a buffer
    uint8_t *texture_buffer = (uint8_t *)malloc(img_size);
    uint8_t *blend_buffer = (uint8_t *)malloc(img_size);

    // read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    // combine edit data w/original image
    for (int i = 0; i < img_size; i++) {
        // TODO: add a switch for 255 to set blend_buffer[i] to 0
        if (texture_buffer[i] != 0) {
            blend_buffer[i] = texture_buffer[i];
        }
        else {
            blend_buffer[i] = img_data->FRM_dir->frame_data[0]->frame_start[i]; // img_data->FRM_data[i];
        }
    }
    free(texture_buffer);
    return blend_buffer;
}

// Split the img_data into tiles
// Save each tile with a formatted name
// Name format -- CreateFileName()
// User options: Split_to_Tiles_OpenGL()
//       1) set /Fallout 2/ directory location
//           and export all tiles to correct folder
//           under that location automatically
//       2) export directly to the folder the user
//           points to, all tiles, automatically
//       3) ask user if folders/directories need
//           to be created
//       4) give user ability to change default
//           game path from menubar (also here?)
//       5) game path or last used save path is
//           stored in user_info & msk2bmpGUI.cfg file

bool export_auto(user_info *usr_info, char *exe_path, char *save_path, img_type save_type)
{
    char dest[3][27]{
        {"/data/art/intrface"},     //WRLDMPxx.FRM
        {"/data/data"},             //wrldmpXX.msk
        {"/data/art/tiles"},        //town map tiles
        };
    int choice = tinyfd_messageBox("Set Default Game Path...",
                                   "Do you want to set your default Fallout 2 game path?\n"
                                   "(Automatically overwrites game files)",
                                   "yesnocancel", "question", 2);
    if (choice == 0)
    {                   // cancel
        return false;
    }
    if (choice == 1)
    {                   // Set default FO2 directory, auto_export = true
        usr_info->auto_export = 1;
        char *current_save_path;
        current_save_path = tinyfd_selectFolderDialog(
            "Select your modded Fallout 2 base directory...",
            usr_info->default_save_path);
        if (!current_save_path)
        {
            return false;
        }
        // TODO: maybe check if fallout2.exe is in the default game path set here?
        if (save_path) {        //generate new save_path from current_save_path + dest[], return in buffer
            snprintf(save_path, MAX_PATH, "%s%s", current_save_path, dest[save_type]);
        }
        strncpy(usr_info->default_game_path, current_save_path, MAX_PATH);

        //if default_save_path isn't set, set it using default_game_path
        if (!strcmp(usr_info->default_save_path, "\0"))
        {
            strcpy(usr_info->default_save_path, usr_info->default_game_path);
        }

        bool file_exists = check_and_write_cfg_file(usr_info, exe_path);
        if (!file_exists)
        {
        //TODO: replace printf()'s with popup windows
            printf("error opening cfg file: %s\n", strerror(errno));
        }
    }
    if (choice == 2)
    {                   // Manual - chosen instead of selecting default path from previous popup
        char *current_save_path;
        current_save_path = tinyfd_selectFolderDialog(
            "Select directory to save to...",
            usr_info->default_save_path);
        if (!current_save_path)
        {
            return false;
        }
        if (save_path) {        //generate new save_path from current_save_path and return in buffer
            strncpy(save_path, current_save_path, MAX_PATH);
        }
        // store current_save_path in default_save_path for future use
        strncpy(usr_info->default_save_path, current_save_path, MAX_PATH);

        bool file_exists = check_and_write_cfg_file(usr_info, exe_path);
        if (!file_exists)
        {
        //TODO: replace printf()'s with popup windows
            printf("error opening cfg file: %s\n", strerror(errno));
        }
    }
    return true;
}

bool export_manual(user_info *usr_info, char *save_path, char* exe_path)
{
    usr_info->auto_export = 2;

    char *current_save_path;
    current_save_path = tinyfd_selectFolderDialog(
        "Select directory to save to...",
        usr_info->default_save_path);
    if (!current_save_path) {
        return false;
    }
    //generate new save_path from current_save_path and return in buffer
    strncpy(save_path, current_save_path, MAX_PATH);

    // store current_save_path in default_save_path for future use
    strncpy(usr_info->default_save_path, current_save_path, MAX_PATH);

    bool file_exists = check_and_write_cfg_file(usr_info, exe_path);
    if (!file_exists)
    {
        //TODO: replace printf()'s with popup windows
        printf("error opening cfg file: %s\n", strerror(errno));
    }

    return true;
}

bool auto_export_question(user_info *usr_info, char *exe_path, char *save_path, img_type save_type)
{
    char dest[3][24] {
        {"/data/art/intrface"},     //WRLDMPxx.FRM
        {"/data/data"},             //wrldmpXX.msk
        {"/data/art/tiles"},        //town map tiles
    };
    if (usr_info->auto_export == not_set) {       // ask user if they want auto/manual
        int auto_choice = tinyfd_messageBox(
            //TODO: simplify this text
                    "Automatic? or Manual?",
                    "Tiles (map/town/msk) are only located\n"
                    "in specific places in the game files.\n\n"
                    "For rapid testing in game, you can export tiles\n"
                    "automatically and bypass this screen.\n"
                    "by setting the modded Fallout 2 directory.\n\n"
                    "Yes --- Auto:   (set Fallout 2 directory)\n"
                    "No  --- Manual: (select a folder)\n\n"
                    "You can change this setting in the config menu.",
                    "yesnocancel", "question", 2);
        if (auto_choice == CANCEL) {            // cancel
            return false;
        }
        if (auto_choice == YES) {               // Auto - chosen from previous popup
            if (!strcmp(usr_info->default_game_path, "") || (usr_info->auto_export == not_set))
            {
                return export_auto(usr_info, exe_path, save_path, save_type);
            }
            else
            {
                snprintf(save_path, MAX_PATH, "%s%s", usr_info->default_game_path, dest[save_type]);
                return true;
            }
        }
        if (auto_choice == NO) {                // Manual
            return export_manual(usr_info, save_path, exe_path);
        }
    }
    if (usr_info->auto_export == auto_all) {                   // Auto   - set by user
        if (!strcmp(usr_info->default_game_path, "")) {
            return export_auto(usr_info, exe_path, save_path, save_type);
        }
        else {
            snprintf(save_path, MAX_PATH, "%s%s", usr_info->default_game_path, dest[save_type]);
            return true;
        }
    }
    if (usr_info->auto_export == manual) {                   // Manual - set by user
        return export_manual(usr_info, save_path, exe_path);
    }
    return false; // again, shouldn't be able to reach this line
}

// save_type is the file type being saved to (not the img_type coming in from img_data)
// img_type: UNK = -1, MSK = 0, FRM = 1, FR0 = 2, FRx = 3, OTHER = 4
void Split_to_Tiles_OpenGL(image_data *img_data, struct user_info *usr_info,
                           img_type save_type, FRM_Header *frm_header,
                           char *exe_path)
{
    int img_width  = img_data->width;
    int img_height = img_data->height;
    int img_size   = img_width * img_height;

    int num_tiles_x = img_width  / MAP_TILE_W;
    int num_tiles_y = img_height / MAP_TILE_H;
    int tile_num = 0;

    char save_path[MAX_PATH];
    char Full_Save_File_Path[MAX_PATH];

    // create basic frame information for saving
    // every tile has the same width/height/size
    FRM_Frame frame_data;
    memset(&frame_data, 0, sizeof(FRM_Frame));
    frame_data.Frame_Width  = (MAP_TILE_W);
    frame_data.Frame_Height = (MAP_TILE_H);
    frame_data.Frame_Size   = (MAP_TILE_SIZE);
    B_Endian::flip_frame_endian(&frame_data);

    FILE *File_ptr = NULL;

    bool success = auto_export_question(usr_info, exe_path, save_path, save_type);
    if (!success) {
        return;
    }

    // create buffers for use in tiling
    uint8_t *blend_buffer = NULL;
    uint8_t *texture_buffer = (uint8_t *)malloc(img_size);
    if (save_type == FRM) {                                     //exporting as FRM
        // create buffer from texture and original FRM_data
        blend_buffer = blend_PAL_texture(img_data);
    }
    else if (save_type == MSK) {                                //exporting as MSK
        // copy edited texture to buffer, combine with original image
        glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
        // read pixels into buffer
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);
    }

    // split buffer into tiles and write to files
    for (int y = 0; y < num_tiles_y; y++)
    {
        for (int x = 0; x < num_tiles_x; x++)
        {
            // create the filename for the current tile
            // assigns final save path string to Full_Save_File_Path
            Create_File_Name(Full_Save_File_Path, "WRLDMP", save_type, save_path, tile_num);

            // check for existing file first unless "Auto" selected?
            ////////////////if (!usr_info->auto_export) {}///////////////////////////////////////////////////////////////
            if (x < 1 && y < 1) {
                check_file(save_path, Full_Save_File_Path, "WRLDMP", tile_num, save_type);
            }
            if (Full_Save_File_Path[0] == '\0') {
                return;
            }

#ifdef QFO2_WINDOWS
            wchar_t *w_save_name = tinyfd_utf8to16(filename_buffer);
            _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
            File_ptr = fopen(Full_Save_File_Path, "wb");
#endif

            if (!File_ptr)
            {
                tinyfd_messageBox(
                    "Error",
                    "Can not open this file in write mode.\n"
                    "Make sure the default game path is set.",
                    "ok", "error", 1);
                return;
            }
            else
            {
                // FRM = 1, MSK = 0
                if (save_type == FRM) {
                    // Split buffer int 350x300 pixel tiles and write to file
                    // save header
                    fwrite(frm_header, sizeof(FRM_Header), 1, File_ptr);
                    fwrite(&frame_data, sizeof(FRM_Frame), 1, File_ptr);

                    int tile_pointer = (y * img_width * MAP_TILE_H) + (x * MAP_TILE_W);
                    int row_pointer = 0;

                    for (int i = 0; i < MAP_TILE_H; i++) {
                        // write out one row of pixels in each loop
                        fwrite(blend_buffer + tile_pointer + row_pointer, MAP_TILE_W, 1, File_ptr);
                        row_pointer += img_width;
                    }
                }
                ///////////////////////////////////////////////////////////////////////////
                if (save_type == MSK) {
                    // Split the surface up into 350x300 pixel buffer
                    //       and pass them to Save_MSK_Image_OpenGL()

                    // create buffers
                    uint8_t *tile_buffer = (uint8_t *)malloc(MAP_TILE_SIZE);

                    int tile_pointer  = (y * img_width * MAP_TILE_H) + (x * MAP_TILE_W);
                    int img_row_pntr  = 0;
                    int tile_row_pntr = 0;

                    for (int i = 0; i < MAP_TILE_H; i++) {
                        // copy out one row of pixels in each loop to the buffer
                        memcpy(tile_buffer + tile_row_pntr, texture_buffer + tile_pointer + img_row_pntr, MAP_TILE_W);

                        img_row_pntr  += img_width;
                        tile_row_pntr += MAP_TILE_W;
                    }

                    Save_MSK_Image_OpenGL(tile_buffer, File_ptr, MAP_TILE_W, MAP_TILE_H);
                    free(tile_buffer);
                }
                fclose(File_ptr);
            }
            tile_num++;
        }
    }
    if (blend_buffer)
    {
        free(blend_buffer);
    }
    if (texture_buffer)
    {
        free(texture_buffer);
    }
}




/////////////////Bakerstaunch version with simplified logic/////////////////
// Note - I've changed the parameter order
// and used separate parameters for the tile
// top and left

// // This function requires src_tile_left to be >-80
// // and <src_width, otherwise, we could write more
// // than intended when we're handling the trimming
// // on the left and right edge of the tile  i.e.
// // the two trimmed variables below could extend
// // past the bounds of the row being written to
// void crop_single_tileB(uint8_t *dst,
//         uint8_t *src, int src_width, int src_height,
//         int src_tile_top, int src_tile_left)
// {
//     assert(src_tile_left > -80);
//     assert(src_tile_left < src_width);
//     for (int row = 0; row < 36; ++row) {
//         int row_left  = tile_mask[row*2+0];
//         int row_right = tile_mask[row*2+1];

//         uint8_t *dst_row_ptr = dst + row * 80;

//         int src_row = src_tile_top + row;
//         if (src_row < 0 || src_row >= src_height) {
//             // above top or bottom of the src image
//             // so we have no pixels to copy, set to 0
//             memset(dst_row_ptr + row_left, 0, row_right - row_left);
//             // continue could be used here for an early return,
//             // but given it's in the middle of the loop it seems
//             // clearer to use an else
//         } else {
//             int src_offset = src_row * src_width + src_tile_left;
//             int src_row_left  = src_tile_left + row_left;
//             int src_row_right = src_tile_left + row_right;

//             // put 0s in the left side of dst when we're off
//             // the left edge of the src image
//             if (src_row_left < 0) {
//                 int trimmed = -src_row_left;
//                 // technically this could set more than
//                 // row_right - row_left bytes, but that's okay, as it
//                 // won't go past the end of the row in the destination
//                 // buffer as long as src_tile_left is > -80
//                 memset(dst_row_ptr + row_left, 0, trimmed);
//                 row_left += trimmed;

//                 // the following line is not needed as we currently
//                 // don't use src_row_left later in the function;
//                 // however it has been left here in case it's needed
//                 // in the future as part of the job of this if block
//                 // is to make sure the left side is within bounds
//                 // when we read it
//                 src_row_left = 0;
//             }

//             // put 0s in the right side of dst when we're off
//             // the right edge of the src image; it's unlikely
//             // this will also run when we're trimming the left
//             // however for thin source images it's possible
//             if (src_row_right > src_width) {
//                 int trimmed = src_row_right - src_width;
//                 row_right -= trimmed;
//                 // technically this could set more than
//                 // row_right - row_left bytes, but that's okay, as it
//                 // won't go past the end of the row in the destination
//                 // buffer as long as src_tile_left is < src_width
//                 memset(dst_row_ptr + row_right, 0, trimmed);

//                 // similar to above, we don't need the following line
//                 // at the moment
//                 src_row_right = src_width;
//             }

//             // we need this check as a safeguard against negative
//             // sizes which could be the result of the row's pixels
//             // entirely being in a trimmed area
//             if (row_right - row_left > 0) {
//                 memcpy(dst_row_ptr + row_left,
//                     src + src_offset + row_left,
//                     row_right - row_left);
//             }
//         }
//     }
// }

// //TODO: needs more tweeking and testing
// //Bakerstaunch vector clearing version w/SSE2 instructions
// #include <emmintrin.h>
// int crop_single_tile_vector_clear(
//         __restrict__ uint8_t *dst, __restrict__ uint8_t *src,
//         int src_width, int src_height,
//         int src_tile_top, int src_tile_left)
// {
//     __m128i ZERO = _mm_setzero_si128();
//     int copied_pixels = 0;
//     for (int row = 0; row < 36; ++row) {
//         int row_left  = tile_mask[row*2+0];
//         int row_right = tile_mask[row*2+1];
//         uint8_t *dst_row_ptr = dst + row * 80;
//         // clear the row with transparent pixels
//         __m128i *dst_row_vec_ptr = (__m128i *)dst_row_ptr;
//         _mm_storeu_si128(dst_row_vec_ptr+0, ZERO);
//         _mm_storeu_si128(dst_row_vec_ptr+1, ZERO);
//         _mm_storeu_si128(dst_row_vec_ptr+2, ZERO);
//         _mm_storeu_si128(dst_row_vec_ptr+3, ZERO);
//         _mm_storeu_si128(dst_row_vec_ptr+4, ZERO);
//         int src_row = src_tile_top + row;
//         if (src_row < 0 || src_row >= src_height) {
//             // above top or bottom of the src image
//             // and we've already cleared the row so
//             // just go to the next row
//             continue;
//         }
//         int src_row_left  = src_tile_left + row_left;
//         // if we're off the left side of the image, increase
//         // row_left by the amount we're off the left side
//         if (src_row_left < 0) {
//             // note src_row_left is negative which makes
//             // this subtraction an addition
//             row_left -= src_row_left;
//         }
//         int src_row_right = src_tile_left + row_right;
//         // if we're off the right side of the image, decrease
//         // row_right by the amount we're off the right side
//         if (src_row_right > src_width) {
//             row_right -= src_row_right - src_width;
//         }
//         int amount_to_copy = row_right - row_left;
//         // we need this check as a safeguard against negative
//         // sizes which could be the result of the row's pixels
//         // entirely being in a trimmed area
//         if (amount_to_copy > 0) {
//             int src_offset = src_row * src_width + src_tile_left;
//             memcpy(dst_row_ptr + row_left,
//                 src + src_offset + row_left,
//                 amount_to_copy);
//             copied_pixels += amount_to_copy;
//         }
//     }
//     return copied_pixels;
// }

// //TODO: fix so pxl_offs is 2 ints and not ImVec2
// //single tile crop using memcpy
// void crop_single_tile(int img_w, int img_h,
//                     uint8_t* tile_buff,
//                     uint8_t* frm_pxls,
//                     int x, int y)
// {
//     for (int row = 0; row < 36; row++)
//     {
//         int lft = tile_mask[row * 2];
//         int rgt = tile_mask[row * 2 + 1];
//         int offset = rgt-lft;
//         int buf_pos = ((row)*80)   + lft;
//         int pxl_pos = ((row)*img_w + lft)
//                       + y*img_w + x;
//     // printf("x: %d, rgt: %d, total: %d\n", x, rgt, x+rgt);
//     //prevent RIGHT pixels outside the image from copying over
//         if ((x + rgt) > img_w) {
//             //set offset amount of row to 0
//             //this is outside the right of the image
//             offset = img_w - (x + lft);
//             memset(tile_buff+buf_pos+offset, 0, 80-(offset));
//             if (offset < 0) {
//                 //if the starting position is also outside the image
//                 //set the whole row to 0
//                 memset(tile_buff+buf_pos, 0, 80-lft);
//                 continue;
//             }
//         }
//     //prevent LEFT pixels outside image from copying over
//         if ((x+lft) < 0) {
//             //set the part of the row not being copied to 0
//             memset(tile_buff+buf_pos, 0, -(x+lft));
//             //move pointers to account for part being skipped
//             buf_pos += 0 - (x+lft);
//             pxl_pos += 0 - (x+lft);
//             offset = rgt + (x);
//             if (offset < 0) {
//                 //skip if ending position offset is outside image
//                 continue;
//             }
//         }
//     //prevent TOP & BOTTOM pixels outside image from copying over
//         if (row+y < 0 || row+y >= img_h) {
//             //just set the buffer lines to 0 and move on
//             memset(tile_buff+buf_pos, 0, rgt-lft);
//             continue;
//         }
//         // printf("offset: %d\n", offset);
//         memcpy(tile_buff+buf_pos, frm_pxls+pxl_pos, offset);
//     }
// }

// //crop town map tiles from a full image
// //TODO: need to add offsets to x & y
// char* crop_TMAP_tiles(int offset_x, int offset_y, image_data *img_data, char* file_path, char* name)
// {
//     char full_file_path[MAX_PATH];
//     uint8_t tile_buff[80 * 36] = {0};
//     int img_w = img_data->width;
//     int img_h = img_data->height;

//     FRM_Header header = {};
//     header.version = 4; // not sure why 4? but vanilla game frm tiles have this
//     header.FPS = 1;
//     header.Frames_Per_Orient = 1;
//     header.Frame_Area = 80 * 36 + sizeof(FRM_Frame);
//     B_Endian::flip_header_endian(&header);

//     FRM_Frame frame = {};
//     frame.Frame_Height = 36;
//     frame.Frame_Width  = 80;
//     frame.Frame_Size   = 80 * 36;
//     B_Endian::flip_frame_endian(&frame);

//     uint8_t *frm_pxls = img_data->FRM_data + sizeof(FRM_Header) + sizeof(FRM_Frame);

//     int origin_x = -48 + offset_x;
//     int origin_y =   0 + offset_y;
//     int tile_num =   0;
//     int row_cnt  =   0;
//     bool running = true;
//     while (running)
//     {
//         //TODO: clean this up, was used for testing different methods
//         // crop_single_tileB(tile_buff, frm_pxls, img_w, img_h,
//         //         origin.y, origin.x);
//         crop_single_tile(img_w, img_h, tile_buff, frm_pxls, origin_x, origin_y);
//         // crop_single_tile_vector_clear(tile_buff, frm_pxls, img_w, img_h,
//         //         origin.y, origin.x);

//         snprintf(full_file_path, MAX_PATH, "%s/%s%03d.FRM", file_path, name, tile_num);
//         // printf("making tile #%03d\n", tile_num);
//         save_TMAP_tile(full_file_path, tile_buff, &header, &frame);
//         tile_num++;

//         //increment one tile position
//         origin_x +=  48;
//         origin_y += -12;

//         if ((origin_y <= -36) || (origin_x >= img_w)) {
//             //increment row and reset tile position
//             row_cnt++;
//             origin_x = -16*row_cnt - 48;
//             origin_y =  36*row_cnt;
//         }
//         while ((origin_y >= img_h) || (origin_x <= -80)) {
//             //increment one tile position until in range of pixels
//             origin_x +=  48;
//             origin_y += -12;
//             //or until outside both width and height of image
//             if ((origin_x >= img_w)) {
//                 running = false;
//                 break;
//             }
//         }
//         // printf("row: %d, tile: %d, origin.x: %.0f, origin.y: %.0f\n", row_cnt, tile_num, origin.x, origin.y);
//     }

//     char* new_tile_list = generate_new_tile_list(name, tile_num);
//     // printf("%s", new_tile_list);

//     return new_tile_list;
// }

//Save town map tiles to gamedir/manual
//TODO: add offset for tile cutting
town_tile* export_TMAP_tiles(user_info* usr_info, char* exe_path,
                       image_data* img_data,
                       int x, int y)
{
    char save_path[MAX_PATH];
    char Full_Save_File_Path[MAX_PATH];
    int tile_num = 0;

    bool success = auto_export_question(usr_info, exe_path, save_path, TILE);
    if (!success) {
        return nullptr;
    }

    // create the filename for the current list of tiles
    // assigns final save path string to Full_Save_File_Path
    char* name = tinyfd_inputBox(
                "Tile Name...",
                "Please type a default tile name for these,\n"
                "exporting will append a tile number to this name.\n",
                "newtile_");
    if (name == nullptr) {
        return nullptr;
    }
    //TODO: check if game engine will take longer than 8-character names
    //      if not, then limit this to 8 (name length + tile digits)
    //      possibly give bypass?
    if (strlen(name) >= 32) {
        printf("name too long?");
        return nullptr;
    }
    snprintf(Full_Save_File_Path, MAX_PATH, "%s/%s%03d.%s", save_path, name, tile_num, "FRM");

    // check for existing file first unless "Auto" selected?
    //TODO: need to verify "Auto" setting
    ////////////////if (!usr_info->auto_export) {}///////////////////////////////////////////////////////////////
    check_file(save_path, Full_Save_File_Path, name, tile_num, FRM);
    if (Full_Save_File_Path[0] == '\0') {
        return nullptr;
    }

    //TODO: old methods of exporting tiles, remove these two lines
    // char* new_TMAP_list = crop_TMAP_tiles(x, y, img_data, save_path, name);
    // add_TMAP_tiles_to_lst(usr_info, &new_TMAP_list, save_path);

    town_tile* new_tiles = crop_TMAP_tile_ll(x, y, img_data, name);
    add_TMAP_tiles_to_lst_tt(usr_info, new_tiles, save_path);

    TMAP_tiles_make_row(new_tiles, usr_info);

    // return new_TMAP_list;
    // free(new_TMAP_list);
    return new_tiles;
}

// void Split_to_Tiles_SDL(SDL_Surface *surface, struct user_info* usr_info, img_type type, FRM_Header* frm_header)
//{
//     int num_tiles_x = surface->w / TILE_W;
//     int num_tiles_y = surface->h / TILE_H;
//     int tile_num = 0;
//     char path[MAX_PATH];
//     char Save_File_Name[MAX_PATH];
//
//     FILE * File_ptr = NULL;
//
//
//     if (!strcmp(user_info->default_game_path, "")) {
//         Set_Default_Path(user_info);
//         if (!strcmp(user_info->default_game_path, "")) { return; }
//     }
//     strncpy(path, user_info->default_game_path, MAX_PATH);
//
//     for (int y = 0; y < num_tiles_y; y++)
//     {
//         for (int x = 0; x < num_tiles_x; x++)
//         {
//             char buffer[MAX_PATH];
//             strncpy_s(buffer, MAX_PATH, Create_File_Name(type, path, tile_num, Save_File_Name), MAX_PATH);
//
//             //check for existing file first
//             check_file(type, File_ptr, path, buffer, tile_num, Save_File_Name);
//             if (buffer == NULL) { return; }
//
//             wchar_t* w_save_name = tinyfd_utf8to16(buffer);
//             _wfopen_s(&File_ptr, w_save_name, L"wb");
//
//             if (!File_ptr) {
//                 tinyfd_messageBox(
//                     "Error",
//                     "Can not open this file in write mode.\n"
//                     "Make sure the default game path is set.",
//                     "ok",
//                     "error",
//                     1);
//                 return;
//             }
//             else {
//                 // FRM = 1, MSK = 0
//                 if (type == FRM)
//                 {
//                     //save header
//                     fwrite(frm_header, sizeof(FRM_Header), 1, File_ptr);
//
//                     int pixel_pointer = surface->pitch * y * TILE_H + x * TILE_W;
//                     for (int pixel_i = 0; pixel_i < TILE_H; pixel_i++)
//                     {
//                         //write out one row of pixels in each loop
//                         fwrite((uint8_t*)surface->pixels + pixel_pointer, TILE_W, 1, File_ptr);
//                         pixel_pointer += surface->pitch;
//                     }
//                     fclose(File_ptr);
//                 }
/////////////////////////////////////////////////////////////////////////////
//                if (type == MSK)
//                {
//                //Split the surface up into 350x300 pixel surfaces
//                //      and pass them to Save_Mask()
//                    Save_MSK_Image_SDL(surface, File_ptr, x, y);
//
/////////////////////////////////////////////////////////////////////////////
//                ///*Blit combination not supported :(
//                                    /// looks like SDL can't convert anything to binary bitmap
//                //SDL_Rect tile = { surface->pitch*y * 300, x * 350,
//                                    // 350, 300 };
//                                    //SDL_Rect dst = { 0,0, 350, 300 };
//                                    //SDL_PixelFormat* pxlfmt = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX1MSB);
//                                    //binary_bitmap = SDL_ConvertSurface(surface, pxlfmt, 0);
//                                    //binary_bitmap = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_INDEX1MSB, 0);
//                                    //printf(SDL_GetError());
//                                    //int error = SDL_BlitSurface(surface, &tile, binary_bitmap, &dst);
//                                    //if (error != 0)
//                                    //{
//                                    //    printf(SDL_GetError());
//                                    //}
//                }
//            }
//        }
//    tile_num++;
//    }
//}

// checks if the file/folder? already exists before saving
// sets Save_File_Name[0] = '\0'; if user clicks cancel
// when prompted to overwrite a file
void check_file(char *save_path, char* save_path_name, const char* name, int tile_num, img_type type)
{
    FILE *File_ptr = NULL;
    char *alt_path;
    const char *lFilterPatterns[3] = {"*.FRM", "*.MSK", ""};

#ifdef QFO2_WINDOWS
    // Windows w/wide character support
    wchar_t *w_save_name = tinyfd_utf8to16(Save_File_Name);
    errno_t error = _wfopen_s(&File_ptr, w_save_name, L"rb");
    if (error == 0)
#elif defined(QFO2_LINUX)
    File_ptr = fopen(save_path_name, "rb");
    if (File_ptr != NULL)
#endif
    {
        fclose(File_ptr);
        // handles the case where the file exists
        char* ptr = strrchr(save_path_name, '\\/');
        char buff[MAX_PATH + 72];
        snprintf(buff, MAX_PATH + 72, "%s%s",
                ptr+1, " already exists,\n\n"
                "YES - Overwrite?\n"
                "NO  - Select a different folder?\n");

        int choice =
            tinyfd_messageBox(
                "Warning",
                buff,
                "yesnocancel",
                "warning",
                2);
        if (choice == 0) {                  // cancel
            save_path_name[0] = '\0';
            return;
        }
        else if (choice == 1) {}            // yes = overwrite
        else if (choice == 2) {             // no  = choose new folder
            alt_path = tinyfd_selectFolderDialog(
                "Select directory to save to...",
                save_path);

            Create_File_Name(save_path_name, name, type, alt_path, tile_num);

            strncpy(save_path, alt_path, MAX_PATH);
            check_file(save_path, save_path_name, name, tile_num, type);
        }
    }

    // If saving to game folder, appropriate directories are
    // appended to the Save_File_Name string
    // handles the case where the DIRECTORY doesn't exist
    else
    {
        if (io_isdir(save_path)) {
            return;
        }

        // create the directory?
        int choice =
            tinyfd_messageBox(
                "Warning",
                "Directory does not exist.\n\n"
                "If exporting in Auto mode, the requisite Fallout 2\n"
                "directories may not exist.\n\n"
                "Create the missing directories?",
                "yesnocancel", "warning", 2);
        if (choice == 0) {          // Cancel =  null out buffer and return
            save_path_name[0] = '\0';
            return;
        }
        if (choice == 1) {          // Yes = Create the folders to write to
            if (io_make_dir(save_path)) {
                check_file(save_path, save_path_name, name, tile_num, type);
                return;
            }
        }
        if (choice == 2) {          // No = (don't overwrite) open a new saveFileDialog() and pick a new savespot
            alt_path = tinyfd_selectFolderDialog(
                "Select directory to save to...",
                save_path);
            if (alt_path == NULL) {
                save_path_name[0] = '\0';
                return;
            }

#ifdef QFO2_WINDOWS
            // TODO: maybe change to strncpy because the _s version is dumb
            strncpy_s(save_path, MAX_PATH, alt_path, MAX_PATH);
#elif defined(QFO2_LINUX)
            strncpy(save_path, alt_path, MAX_PATH);
#endif
            Create_File_Name(save_path_name, name, type, alt_path, tile_num);
            check_file(save_path, save_path_name, name, tile_num, type);
        }
    }
}

// Create a filename based on the directory and export file type
//TODO: clean up this function, buff_size is not used
// img_type type: UNK = -1, MSK = 0, FRM = 1, FR0 = 2, FRx = 3, OTHER = 4
void Create_File_Name(char *return_buffer, const char* name, img_type save_type, char *save_path, int tile_num)
{
    char ext[2][4] = {
        {"MSK"},
        {"FRM"}};

    //-------create file path string based on save_path, tile_num, save_type
    if (strcmp(name, "WRLDMP") == 0){
        snprintf(return_buffer, MAX_PATH, "%s/%s%02d.%s", save_path, name, tile_num, ext[save_type]);
    } else {
        //TODO: may need some way to expand or contract
        //      the number of digits in the tile name
        snprintf(return_buffer, MAX_PATH, "%s/%s%03d.%s", save_path, name, tile_num, ext[save_type]);
    }

    // printf("%s\n%s\n", return_buffer, ext[save_type]);
}

void Save_Full_MSK_OpenGL(image_data *img_data, user_info *usr_info)
{
    if (usr_info->save_full_MSK_warning)
    {
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
    uint8_t *texture_buffer = (uint8_t *)malloc(texture_size);
    // copy edited texture to buffer, combine with original image
    glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
    // read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    // get filename
    FILE *File_ptr = NULL;
    char *Save_File_Name;
    const char *lFilterPatterns[2] = {"*.MSK", ""};
    char save_path[MAX_PATH];

    snprintf(save_path, MAX_PATH, "%s/temp001.MSK", usr_info->default_save_path);

    Save_File_Name = tinyfd_saveFileDialog(
        "default_name",
        save_path,
        2,
        lFilterPatterns,
        nullptr);

    if (Save_File_Name == NULL)
    {
        return;
    }

#ifdef QFO2_WINDOWS
    wchar_t *w_save_name = tinyfd_utf8to16(Save_File_Name);
    _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
    File_ptr = fopen(Save_File_Name, "wb");
#endif

    if (!File_ptr)
    {
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

// void Save_MSK_Image_SDL(SDL_Surface* surface, FILE* File_ptr, int x, int y)
//{
//     uint8_t out_buffer[13200] /*= { 0 }/* ceil(350/8) * 300 */;
//     uint8_t *outp = out_buffer;
//
//     int shift = 0;
//     uint8_t bitmask = 0;
//     bool mask_1_or_0;
//
//     int pixel_pointer = surface->pitch * y * TILE_H + x * TILE_W;
//     //don't need to flip for the MSK (maybe need to flip for bitmaps)
//     for (int pxl_y = 0; pxl_y < TILE_H; pxl_y++)
//     {
//         for (int pxl_x = 0; pxl_x < TILE_W; pxl_x++)
//         {
//             bitmask <<= 1;
//             mask_1_or_0 =
//                 *((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) > 0;
//             //*((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) & 1;
//             //*((uint8_t*)surface->pixels + (pxl_y * surface->pitch) + pxl_x * 4) > 0 ? 1 : 0;
//             bitmask |= mask_1_or_0;
//             if (++shift == 8)
//             {
//                 *outp = bitmask;
//                 ++outp;
//                 shift = 0;
//                 bitmask = 0;
//             }
//         }
//         bitmask <<= 2 /* final shift */;
//         *outp = bitmask;
//         ++outp;
//         shift = 0;
//         bitmask = 0;
//     }
//     writelines(File_ptr, out_buffer);
//     fclose(File_ptr);
// }

void Save_MSK_Image_OpenGL(uint8_t *tile_buffer, FILE *File_ptr, int width, int height)
{
    // int buff_size = ceil(width / 8.0f) * height;
    int buff_size = (width + 7) / 8 * height;

    // final output buffer
    uint8_t *out_buffer = (uint8_t *)malloc(buff_size);

    int shift = 0;
    uint8_t bitmask = 0;
    // bool mask_1_or_0;
    uint8_t *outp = out_buffer;
    for (int pxl_y = 0; pxl_y < height; pxl_y++)
    {
        for (int pxl_x = 0; pxl_x < width; pxl_x++)
        {
            // don't need to flip for MSK (maybe need to flip for bitmaps?)
            bitmask <<= 1;
            bitmask |= tile_buffer[pxl_x + pxl_y * width];
            if (++shift == 8)
            {
                *outp = bitmask;
                ++outp;
                shift = 0;
                bitmask = 0;
            }
        }
        // handle case where width doesn't fit 8-bits evenly
        if (shift > 0)
        {
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
