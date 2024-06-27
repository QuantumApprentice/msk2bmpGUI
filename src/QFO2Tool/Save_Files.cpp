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

//TODO: remove SDL stuff
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
                    // Split buffer into 350x300 pixel tiles and write to file
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

//Save town map tiles to gamedir/manual
//TODO: add offset for tile cutting
// town_tile* export_TMAP_tiles(user_info* usr_info,
tt_arr_handle* export_TMAP_tiles(user_info* usr_info,
                       image_data* img_data,
                       int x, int y)
{
    char save_path[MAX_PATH];
    int tile_num = 0;

    bool success = auto_export_question(usr_info, usr_info->exe_directory, save_path, TILE);
    if (!success) {
        return nullptr;
    }

    // create the filename for the current list of tiles
    // assigns final save path string to Full_Save_File_Path
    char* name = tinyfd_inputBox(
                "Tile Name...",
                "Please type a default tile name for these,\n"
                "exporting will append a tile number to this name.\n\n"
                "Tile names can only be 8 characters total,\n"
                "and currently 3 of those characters are taken\n"
                "up by the numbering system.\n"
                "(Which leaves 5 for you to work with).",
                "new__");
    if (name == nullptr) {
        return nullptr;
    }

    //TODO:  swap this entire tinyfd question for ImGui popup
    //game engine/mapper only takes 8 character tile-names
    if (strlen(name) > 5) {
        printf("name too long?");
        return nullptr;
    }

    // check for existing file first unless "Auto" selected?
    //TODO: need to verify "Auto" setting
    ////////////////if (!usr_info->auto_export) {}///////////////////////////////////////////////////////////////
    char Full_Save_File_Path[MAX_PATH];
    snprintf(Full_Save_File_Path, MAX_PATH, "%s/%s%03d.%s", save_path, name, tile_num, "FRM");
    check_file(save_path, Full_Save_File_Path, name, tile_num, FRM);
    if (Full_Save_File_Path[0] == '\0') {
        return nullptr;
    }

    //TODO: remove this call, replaced by
    // town_tile* new_tiles = crop_TMAP_tile_ll(x, y, img_data, save_path, name);

    tt_arr_handle* handle = crop_TMAP_tile_arr(x, y, img_data, save_path, name);


    return handle;
    // return new_tiles;
}

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
