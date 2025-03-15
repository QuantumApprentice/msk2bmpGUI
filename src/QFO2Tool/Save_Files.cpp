#include <stdio.h>
#include <cstdint>
#include <sys/types.h>
#include <stb_image_write.h>
#include <filesystem>

#ifdef QFO2_WINDOWS
#include <Windows.h>
#elif defined(QFO2_LINUX)

#endif


#include "Save_Files.h"

#include "B_Endian.h"
#include "imgui.h"
#include "Load_Settings.h"
#include "MSK_Convert.h"
#include "platform_io.h"
#include "Edit_TILES_LST.h"
#include "tiles_pattern.h"
#include "town_map_tiles.h"
#include "ImGui_Warning.h"

#include "tinyfiledialogs.h"    //TODO: remove/delete
#include <ImFileDialog.h>
#include <imgui_internal.h>


void write_cfg_file(user_info* user_info, char* exe_path);
uint8_t* texture_to_buff(GLuint texture, int bpp, int w, int h);

void init_IFD()
{
    //TODO: move this to some initializing function
    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;
        // https://github.com/dfranx/ImFileDialog
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt==0)?GL_BGRA:GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void*)(uint64_t)tex;
    };
    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        GLuint texID = (uint64_t)tex;
        glDeleteTextures(1, &texID);
    };
}

bool write_single_frame_FRM_SURFACE(Surface* src, FILE* dst, bool single_frame)
{
    int w = src->w;
    int h = src->h;
    int s = w*h;

    FRM_Frame frame;
    frame.Frame_Width    = w;
    frame.Frame_Height   = h;
    frame.Frame_Size     = s;
    frame.Shift_Offset_x = 0;
    frame.Shift_Offset_y = 0;
    if (!single_frame) {
        frame.Shift_Offset_x = src->x;
        frame.Shift_Offset_y = src->y;
    }

    B_Endian::flip_frame_endian(&frame);
    uint8_t* pxls = src->pxls;

    fwrite(&frame, sizeof(FRM_Frame), 1, dst);
    fwrite(pxls, s, 1, dst);

    return true;
}

const char* FRM_extension(Direction dir)
{
    switch (dir)
    {
    case NE:
        return "FR0";
    case E:
        return "FR1";
    case SE:
        return "FR2";
    case SW:
        return "FR3";
    case W:
        return "FR4";
    case NW:
        return "FR5";
    }
    //default
    return "FRM";
}


bool save_FRM_SURFACE(char* save_name, image_data* img_data, user_info* usr_info, Save_Info* sv_info, bool overwrite)
{
    if (strlen(save_name) < 1) {
        return false;
    }

    if (!strrchr(save_name, '.')) {
        Direction dir = (Direction)img_data->display_orient_num;
        const char* ext = FRM_extension(dir);
        char buff[MAX_PATH];
        strncpy(buff, save_name, MAX_PATH);
        snprintf(save_name, MAX_PATH, "%s.%s", buff, ext);
    }

    if (!overwrite) {
        if (io_file_exists(save_name)) {
            ImGui::OpenPopup("Match found");
            return false;
        }
    }

    FILE* file_ptr = NULL;

#ifdef QFO2_WINDOWS
    // parse Save_File_Name to isolate the directory and save in default_save_path for Windows (w/wide character support)
    wchar_t *w_save_name = tinyfd_utf8to16(save_name);

    _wfopen_s(&file_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)

    file_ptr = fopen(save_name, "wb");
#endif

    if (!file_ptr) {
        //TODO: log out to txt file
        set_popup_warning(
            "[ERROR] save_FRM_SURFACE()\n\n"
            "Unable to open file in write mode.\n"
        );
        printf("Error: Unable to open file in write mode: %s: %d", save_name, __LINE__);
        return false;
    }

    ANM_Dir* anm_dir = img_data->ANM_dir;

    int dir = img_data->display_orient_num;
    int num = img_data->display_frame_num;
    int w   = anm_dir[dir].frame_data[num]->w;
    int h   = anm_dir[dir].frame_data[num]->h;
    int s   = w*h;
    int fpo = sv_info->s_type == single_frm ? 1 : anm_dir[dir].num_frames;

    FRM_Header header;
    header.version           = 4;
    header.Frames_Per_Orient = fpo;
    header.Action_Frame      = sv_info->action_frame;   //TODO: this needs user input
    //Frame_Area = total size of all frames + frame headers (does not include FRM header)
    //           = (total size of all pxls) + (12*num_frames*num_dirs)
    //Oddly, it seems FRx images store the total for all 6 directions
    //      even though only 1 direction is in the FRM
    //get total size of frm pxls + frame
    int count   = 1;
    int num_dir = 1;
    header.Frame_0_Offset[0] = sizeof(FRM_Header);

//TODO: need to figure out what in the world
//      header.Shift_Orient_x/y does
//      and how to calculate it?
//looks like it might be an initial centering issue?
//need to have a centering hex? how in the world do
//I figure out what a 0,0 positioned FRM will look
//like in the mapper/game engine?

    if (sv_info->s_type == single_dir) {
        num_dir = 1;
        count = anm_dir[dir].num_frames;
        s = 0;
        for (int i = 0; i < count; i++) {
            s += 12;
            s += w*h;
        }
    } else
    if (sv_info->s_type == all_dirs) {
        num_dir = 6;
        count = anm_dir[dir].num_frames;
        s = 0;
        for (int i = 0; i < num_dir; i++) {
            header.Frame_0_Offset[i] = sizeof(FRM_Header) + s;
            for (int j = 0; j < count; j++) {
                s += 12;
                s += w*h;
            }
        }
    }

    header.Frame_Area = s;
    B_Endian::flip_header_endian(&header);
    fwrite(&header, sizeof(FRM_Header), 1, file_ptr);

    for (int i = 0; i < num_dir; i++) {
        if (num_dir < 6) {
            i = dir;
        }
        for (int j = 0; j < count; j++) {
            if (count < anm_dir[i].num_frames) {
                j = img_data->display_frame_num;
            }
            Surface* src = anm_dir[i].frame_data[j];
            write_single_frame_FRM_SURFACE(src, file_ptr, (count > 1) ? false : true);
        }
    }

    fclose(file_ptr);

    // printf("name: %s\n", save_name);

    return save_name;
}

bool ImDialog_save_FRM_SURFACE(image_data* img_data, user_info* usr_info, Save_Info* sv_info)
{
    init_IFD();

    static char save_name[MAX_PATH];
    const char* save_type;
    if (sv_info->s_type == single_frm) {
        save_type = "Export only selected \nframe as FRM.";
    }
    else if (sv_info->s_type == single_dir) {
        save_type = "Export all frames in \nselected direction as FRx.";
    }
    else if (sv_info->s_type == all_dirs) {
        save_type = "Export all frames in \nall directions as FRM.";
    }

    const char* ext_filter;
    if (ImGui::Button(save_type)) {
        if (sv_info->s_type == single_dir) {
            ext_filter = "FRx file (single direction only)"
            "(*.fr0;*.fr1;*.fr2;.fr3;*.fr4;*.fr5;)";
            "{.fr0,.FR0,.fr1,.FR1,.fr2,.FR2,.fr3,.FR3,.fr4,.FR4,.fr5,.FR5,}";
        } else {
            ext_filter = "FRM file (single image or all 6 directions)"
                // "(*.png;*.apng;*.jpg;*.jpeg;*.frm;*.fr0-5;*.msk;)"
                "(*.frm;){"
                // ".png,.jpg,.jpeg,"
                ".frm,.FRM,"
                // ".msk,.MSK,"
                "},.*";
        }

        char* folder = usr_info->default_save_path;
        ifd::FileDialog::Instance().Save("FRMSaveDialog", "Save File", ext_filter, folder);
    }


    static bool overwrite;
    static bool success = false;
    if (ImGui::BeginPopupModal("Match found"))
    {
        char* dup_name = strrchr(save_name, PLATFORM_SLASH)+1;
        ImGui::Text(
            "%s already exists,\n\n", dup_name
        );
        if (ImGui::Button("Overwrite?")) {
            overwrite = true;
            success   = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Select a different filename?")) {
            ImGui::CloseCurrentPopup();
            success      = false;
            overwrite    = false;
            save_name[0] = '\0';
            char* folder = usr_info->default_save_path;
            ifd::FileDialog::Instance().Save("FRMSaveDialog", "Save File", ext_filter, folder);
        }

        if (ImGui::Button("Cancel") ){
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            save_name[0] = '\0';
            success      = false;
            overwrite    = false;
            return false;
        }

        ImGui::EndPopup();
    }


    if (ifd::FileDialog::Instance().IsDone("FRMSaveDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(save_name, temp.c_str(), temp.length()+1);
            strncpy(usr_info->default_save_path, temp.c_str(), temp.length()+1);
            char* ptr = strrchr(usr_info->default_save_path, PLATFORM_SLASH);
            *ptr = '\0';
            success = true;
        }
        ifd::FileDialog::Instance().Close();
    }

    if (strlen(save_name) > 0 && success) {
        success = save_FRM_SURFACE(save_name, img_data, usr_info, sv_info, overwrite);
    }

    if (success) {
        save_name[0] = '\0';
        overwrite = false;
        success = false;
        return false;
    }

    return true;
}

const char *Set_Save_Ext(image_data* img_data, int current_dir, int num_dirs)
{
    if (num_dirs > 1)
    {
        Direction *dir_ptr = NULL;
        // dir_ptr = &img_data->FRM_dir[current_dir].orientation;
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


//TODO delete?
// char* Set_Save_File_Name(image_data* img_data, char* name)
// {
//     char* Save_File_Name;
//     int num_patterns = 6;
//     static const char* const lFilterPatterns[6] = {"*.FR0", "*.FR1", "*.FR2", "*.FR3", "*.FR4", "*.FR5"};
//     const char* ext = Set_Save_Ext(img_data, img_data->display_orient_num, num_patterns);
//     int buffsize = strlen(name) + 5;
//     char* temp_name = (char *)malloc(sizeof(char) * buffsize);
//     snprintf(temp_name, buffsize, "%s%s", name, ext);

//     Save_File_Name = tinyfd_saveFileDialog(
//         "default_name",
//         temp_name,
//         num_patterns,
//         lFilterPatterns,
//         nullptr);
//     free(temp_name);

//     return Save_File_Name;
// }


//TODO: delete?
char* Save_IMG_STB(Surface* b_surface, user_info* usr_nfo)
{
    char* Save_File_Name;
    const char* lFilterPatterns[2] = {"*.BMP", ""};
    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\temp001.bmp", usr_nfo->default_save_path);

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
        // SDL_SaveBMP(b_surface, Save_File_Name);

        // TODO: add support for more file formats (GIF in particular)
        // IMG_SavePNG();

        // parse Save_File_Name to isolate the directory and store in default_save_path
        std::filesystem::path p(Save_File_Name);
        strncpy(usr_nfo->default_save_path, p.parent_path().string().c_str(), MAX_PATH);
    }
    return Save_File_Name;
}

//checks if msk2bmpGUI.cfg exists,
//if it doesn't, creates the file (including folder)
//then it writes current settings to cfg file
bool check_and_write_cfg_file(user_info* user_info, char* exe_path)
{
    char cfg_filepath_buffer[MAX_PATH];
    char cfg_path_buffer[MAX_PATH];
    snprintf(cfg_filepath_buffer, sizeof(cfg_filepath_buffer), "%s%s", exe_path, "config/msk2bmpGUI.cfg");
    snprintf(cfg_path_buffer, sizeof(cfg_path_buffer), "%s%s", exe_path, "config/");

    FILE* cfg_file_ptr = NULL;

#ifdef QFO2_WINDOWS
    // Windows w/wide character support
    _wfopen_s(&cfg_file_ptr, tinyfd_utf8to16(cfg_filepath_buffer), L"rb");
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
void Set_Default_Game_Path(user_info* usr_info, char* exe_path)
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


// Create a filename based on the directory and export file type
// img_type type: UNK = -1, MSK = 0, FRM = 1, FR0 = 2, FRx = 3, OTHER = 4
void create_tile_name(char* dst, char* name, img_type save_type, char* path, int tile_num)
{
    char ext[2][4] = {
        {"MSK"},
        {"FRM"}};

    //-------create file path string based on path, tile_num, save_type
    snprintf(dst, MAX_PATH, "%s/%s%02d.%s", path, name, tile_num, ext[save_type]);

    // printf("%s\n%s\n", dst, ext[save_type]);
}


//called 2nd
bool save_tiles_SURFACE(char* base_path, char* save_name, char* save_path,
    uint8_t* selected, Surface* src, img_type type,
    struct user_info* usr_info, Save_Info* sv_info, bool overwrite)
{
    if (strlen(base_path) < 1) {
        return false;
    }
    if (strlen(save_name) < 1) {
        return false;
    }

    int img_w    = src->w;
    int img_h    = src->h;
    int img_size = img_w * img_h;

    int num_tiles_x = img_w / MAP_TILE_W;
    int num_tiles_y = img_h / MAP_TILE_H;
    uint8_t* pxls   = src->pxls;

    if (type == TILE) {
        type = FRM;
    }

    FRM_Header header        = {};
    header.version           = 4;
    header.FPS               = 1;
    header.Frames_Per_Orient = 1;
    header.Frame_Area        = MAP_TILE_SIZE;
    B_Endian::flip_header_endian(&header);


    // create basic frame information for saving
    // every Map TILE has the same width/height/size
    FRM_Frame frame_data = {};
    frame_data.Frame_Width  = MAP_TILE_W;
    frame_data.Frame_Height = MAP_TILE_H;
    frame_data.Frame_Size   = MAP_TILE_SIZE;
    B_Endian::flip_frame_endian(&frame_data);


    int tile_num   = 0;
    FILE* File_ptr = NULL;
    uint8_t tile_buffer[MAP_TILE_SIZE];
    // split buffer into tiles and write to files
    for (int y = 0; y < num_tiles_y; y++)
    {
        for (int x = 0; x < num_tiles_x; x++)
        {
            if (selected[tile_num] == 0) {
                tile_num++;
                continue;
            }
            // create the filename for the current tile
            // assigns final save path string to Full_Save_File_Path
            create_tile_name(save_path, save_name, type, base_path, tile_num);
            if (strlen(save_path) < 1) {
                return false;
            }

            // check for existing file first unless "Auto" selected?
            ////////////////if (!usr_info->auto_export) {}///////////////////////////////////////////////////////////////
            if (!overwrite) {
                if (io_file_exists(save_path)) {
                    ImGui::OpenPopup("Match found");
                    return false;
                }
            }

#ifdef QFO2_WINDOWS
            wchar_t* w_save_name = tinyfd_utf8to16(save_path);
            _wfopen_s(&File_ptr, w_save_name, L"wb");
#elif defined(QFO2_LINUX)
            File_ptr = fopen(save_path, "wb");
#endif

            if (!File_ptr) {
                //TODO: replace with set_popup_warning()
                set_popup_warning(
                    "[ERROR] save_tiles_SURFACE()\n\n"
                    "Can not open this file in write mode.\n"
                    // "Make sure the default game path is set."
                );
                printf("Error: Unable to open file in Write Mode: %s : %d", save_path, __LINE__);
                return false;
            }

            int tile_pointer  = (y * img_w * MAP_TILE_H) + (x * MAP_TILE_W);
            int img_row_pntr  = 0;
            int tile_row_pntr = 0;
            // Split buffer into 350x300 pixel tiles and write to file
            for (int i = 0; i < MAP_TILE_H; i++) {
                // copy out one row of pixels in each loop
                memcpy(&tile_buffer[tile_row_pntr], &src->pxls[tile_pointer + img_row_pntr], MAP_TILE_W);
                img_row_pntr  += img_w;
                tile_row_pntr += MAP_TILE_W;
            }
            // FRM = 1, MSK = 0
            if (type == FRM) {
                fwrite(&header,     sizeof(FRM_Header), 1, File_ptr);
                fwrite(&frame_data, sizeof(FRM_Frame),  1, File_ptr);
                fwrite(&tile_buffer, MAP_TILE_SIZE,     1, File_ptr);
            }
            ///////////////////////////////////////////////////////////////////////////
            if (type == MSK) {
                Save_MSK_Image_OpenGL(tile_buffer, File_ptr, MAP_TILE_W, MAP_TILE_H);
            }
            fclose(File_ptr);
            tile_num++;
        }
    }

    return true;
}


uint8_t* tile_grid(Surface* src, uint8_t* selected, int e)
{
    int tile_w = src->w / MAP_TILE_W;
    int tile_h = src->h / MAP_TILE_H;
    int total  = tile_w*tile_h;
    if (!selected) {
        selected = (uint8_t*)calloc(1, total*sizeof(uint8_t));
    }
    if (e == 0) {
        //set all tile entries to selected
        memset(selected,1,total);
    }
    for (int y = 0; y < tile_h; y++) {
        for (int x = 0; x < tile_w; x++) {
            if (x > 0) {ImGui::SameLine();}
            int cur_tile = y*tile_w + x;
            char num[3];
            snprintf(num, 3, "%02d", cur_tile);
            ImGui::PushID(cur_tile);
            if (ImGui::Selectable(num, selected[cur_tile] != 0, 0, ImVec2(50, 50)))
            {
                // Toggle clicked cell - clear all cells and set single selected
                if (e == 2) {
                    memset(selected,0,total);
                    selected[cur_tile] ^= 1;
                }
            }
            ImGui::PopID();
        }
    }

    return selected;
}


//called 1st
bool ImDialog_save_TILE_SURFACE(image_data* img_data, user_info* usr_info, Save_Info* sv_info)
{
    //TODO: move this to initialize at program start?
    init_IFD();

    // ImGui::Text(
    //     "Tiles (map/town/msk) are only located\n"
    //     "in specific places in the game files.\n\n"
    //     "For rapid testing in game, you can export tiles\n"
    //     "automatically and bypass this screen.\n"
    //     "by setting the modded Fallout 2 directory.\n\n"
    //     "Yes --- Auto:   (set Fallout 2 directory)\n"
    //     "No  --- Manual: (select a folder)\n\n"
    //     "You can change this setting in the config menu."
    // );

    Surface* src;
    img_type type;
    const char* output_type;
    if (img_data->type == MSK) {
        src = img_data->MSK_srfc;
        type = MSK;
        output_type = "Save worldmap MSK tiles";
    } else {
        src = img_data->ANM_dir[0].frame_data[0];
        type = FRM;
        output_type = "Save worldmap FRM tiles";
    }

    //don't move this below the image render
    //  thing will overlap weirdly if moved
    static int e;
    ImGui::RadioButton("All Tiles",   &e, 0);
    // ImGui::RadioButton("Tile Range",  &e, 1);
    ImGui::RadioButton("Single Tile", &e, 2);
    // if (e == 0) {}
    // else if (e == 1) {}
    // else if (e == 2) {}

    ImVec2 scaled = {
        src->w / 7,
        src->h / 6
    };
    ImVec2 img_pos = ImGui::GetCursorScreenPos();

    //image split into selectable tiles here?
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DrawList->AddImage(
        (ImTextureID)(uintptr_t)img_data->render_texture,
        img_pos, {img_pos.x + scaled.x, img_pos.y + scaled.y},
        ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
        ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));

    static uint8_t* selected;
    selected = tile_grid(src, selected, e);

    ImGui::Text(
        "World map tiles (FRM) and mask tiles (MSK)\n"
        "can technically have any name you choose.\n"
        "The only restriction is the total length\n"
        "of the name can't be more than 8 characters.\n"
        "This includes any sequential numbers\n"
        "attached to the base-name.\n\n"
        "This program automatically appends 2 digits\n"
        "to the end of the base-name defined here.\n"
    );
    static char save_name[23] = "WRLDMP";
    ImGui::InputText(
        "Name\n(max 6 characters)",
        save_name, 7);

    char* folder = usr_info->default_save_path;
    if (ImGui::Button(output_type)) {
        ifd::FileDialog::Instance().Open("FileSaveDialog", "Save Folder", "", false, folder);
    }

    static bool success = false;
    static bool overwrite;
    static char save_folder[MAX_PATH];
    static char save_path[MAX_PATH];
    if (ImGui::BeginPopupModal("Match found"))
    {
        char* dup_name = strrchr(save_path, PLATFORM_SLASH)+1;
        ImGui::Text(
            "%s already exists,\n\n", dup_name
        );
        if (ImGui::Button("Overwrite?")) {
            ImGui::CloseCurrentPopup();
            overwrite = true;
            success   = true;
        }
        if (ImGui::Button("Select a different folder?")) {
            ImGui::CloseCurrentPopup();
            save_folder[0] = '\0';
            save_path[0]   = '\0';
            overwrite      = false;
            success        = false;
            ifd::FileDialog::Instance().Open("FileSaveDialog", "Save File", "", false, folder);
        }

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            save_folder[0] = '\0';
            save_path[0]   = '\0';
            overwrite      = false;
            success        = false;
            free(selected);
            selected = NULL;
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }

    if (ifd::FileDialog::Instance().IsDone("FileSaveDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(save_folder, temp.c_str(), temp.length()+1);
            strncpy(usr_info->default_save_path, temp.c_str(), temp.length()+1);
            success = true;
        }
        ifd::FileDialog::Instance().Close();
    }

    if (strlen(save_folder) > 0 && success) {
        success = save_tiles_SURFACE(save_folder, save_name, save_path,
                    selected, src, type, usr_info, sv_info, overwrite);
    }
    if (success) {
        free(selected);
        selected = NULL;
        save_folder[0] = '\0';
        save_path[0]   = '\0';
        overwrite      = false;
        success        = false;
        return false;
    }

    return true;
}









// wrapper to save MSK tiles

//returns a buffer ripped from an openGL texture
//bpp = bytes per pixel
uint8_t* texture_to_buff(GLuint texture, int bpp, int w, int h)
{
    int size = w*h;
    int type = 0;
    switch (bpp)
    {
    case 1:
        type = GL_RED;
        break;
    case 3:
        type = GL_RGB;
        size *= 3;
        break;
    case 4:
        type = GL_RGBA;
        size *= 4;
        break;
    }
    if (type == 0) {
        printf("failed to set color type");
        return NULL;
    }

    uint8_t* buffer = (uint8_t*)malloc(size);

    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_PACK_ALIGNMENT, bpp);
    glGetTexImage(GL_TEXTURE_2D, 0, type, GL_UNSIGNED_BYTE, buffer);

    return buffer;
}

//TODO: do I need this blending system?
//      seems like it was used to get alpha channels back
//      into the FRM, but not sure how well it worked
//      --certainly doesn't work now
uint8_t *blend_PAL_texture(image_data* img_data)
{
    int img_size = img_data->width * img_data->height;

    // copy edited texture to buffer, combine with original image
    glBindTexture(GL_TEXTURE_2D, img_data->PAL_texture);

    // create a buffer
    uint8_t* texture_buffer = (uint8_t*)malloc(img_size);
    uint8_t* blend_buffer   = (uint8_t*)malloc(img_size);

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
            // blend_buffer[i] = img_data->FRM_dir->frame_data[0]->frame_start[i]; // img_data->FRM_data[i];
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
//TODO: replace with something that doesn't suck
bool export_auto(user_info* usr_info, char* exe_path, char* save_path, img_type save_type)
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
        char* current_save_path;
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
        char* current_save_path;
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

    char* current_save_path;
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

//TODO: re-implement this with ImFileDialog()
bool auto_export_question(user_info* usr_info, char* exe_path, char* save_path, img_type save_type)
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

//Save town map tiles to gamedir/manual
//TODO: add offset for tile cutting
tt_arr_handle* export_TMAP_tiles_POPUP(user_info* usr_info, image_data* img_data, Rect* offset)
{

    //TODO: re-implement this with ImFileDialog()
    // bool success = auto_export_question(usr_info, usr_info->exe_directory, save_path, TILE);
    // if (!success) {
    //     return nullptr;
    // }

    ImGui::Text(
        "Please type a default name for these tiles.\n"
        "Exporting will append a tile number to this name.\n\n"
        "Tile names can only be 8 characters total,\n"
        "and 3 of those characters are currently\n"
        "taken up by the numbering system.\n"
        "(Which leaves 5 for you to work with).\n"
        "ex: tile_000.FRM, tile_001.FRM, ... , tile_999.FRM\n"
    );
    //game engine/mapper only takes 8 character tile-names
    static char save_name[16] = "tile_";
    ImGui::InputText(
        "Name\n(max 5 characters)",
        save_name, 6);

    // create the filename for the current list of tiles
    // assigns final save path string to Full_Save_File_Path
    const char* ext_filter;
    if (ImGui::Button("Save as Town Map Tiles")) {
        ext_filter = "FRM file (single image or all 6 directions)"
            "(*.frm;){"
            ".frm,.FRM,"
            "},.*";

        char* folder = usr_info->default_save_path;
        ifd::FileDialog::Instance().Open("TMAPSaveDialog", "Save Folder", "", false, folder);
    }


    static char save_fldr[MAX_PATH];
    static char save_path[MAX_PATH];
    static bool success = false;
    static bool overwrite;
    if (ImGui::BeginPopupModal("Match found"))
    {
        char* dup_name = strrchr(save_path, PLATFORM_SLASH)+1;
        ImGui::Text(
            "%s already exists,\n\n", dup_name
        );
        if (ImGui::Button("Overwrite?")) {
            success   = true;
            overwrite = true;
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Select a different folder?")) {
            ImGui::CloseCurrentPopup();
            success      = false;
            overwrite    = false;
            save_path[0] = '\0';
            save_fldr[0] = '\0';
            char* folder = usr_info->default_save_path;
            ifd::FileDialog::Instance().Open("TMAPSaveDialog", "Save Folder", "", false, folder);
        }

        if (ImGui::Button("Cancel") ){
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            success      = false;
            overwrite    = false;
            save_path[0] = '\0';
            save_fldr[0] = '\0';
            return NULL;
        }

        ImGui::EndPopup();
    }

    if (ifd::FileDialog::Instance().IsDone("TMAPSaveDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            success = true;
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(save_fldr, temp.c_str(), temp.length()+1);
            strncpy(usr_info->default_save_path, temp.c_str(), temp.length()+1);
        }
        ifd::FileDialog::Instance().Close();
    }


    // char* name = NULL;
    // if (name == nullptr) {
    //     return nullptr;
    // }
    // check for existing file first unless "Auto" selected?
    //TODO: need to verify "Auto" setting
    ////////////////if (!usr_info->auto_export) {}///////////////////////////////////////////////////////////////
    // int tile_num = 0;
    // char Full_Save_File_Path[MAX_PATH];
    // snprintf(Full_Save_File_Path, MAX_PATH, "%s/%s%03d.%s", save_folder, name, tile_num, "FRM");
    // check_file(save_folder, Full_Save_File_Path, name, tile_num, FRM);
    // if (Full_Save_File_Path[0] == '\0') {
    //     return nullptr;
    // }

    int dir = img_data->display_orient_num;
    int num = img_data->display_frame_num;
    Surface* src = img_data->ANM_dir[dir].frame_data[num];

    tt_arr_handle* handle = NULL;
    if (strlen(save_fldr) > 0 && success) {
        handle = crop_TMAP_tile_arr_POPUP(offset, src, save_fldr, save_name, save_path, overwrite);
        if (!handle) {
            success = false;
        }
    }

    if (handle) {
        save_fldr[0] = '\0';
        save_path[0] = '\0';
        overwrite    = false;
        success      = false;
        return handle;
    }


    return NULL;
}

// checks if the file/folder? already exists before saving
// sets Save_File_Name[0] = '\0'; if user clicks cancel
// when prompted to overwrite a file
//TODO: delete? not sure I need this anymore
void check_file(char* save_path, char* save_path_name, const char* name, int tile_num, img_type type)
{
    FILE* File_ptr = NULL;
    char* alt_path;
    const char* lFilterPatterns[3] = {"*.FRM", "*.MSK", ""};

#ifdef QFO2_WINDOWS
    // Windows w/wide character support
    wchar_t* w_save_name = tinyfd_utf8to16(save_path_name);
    errno_t error = _wfopen_s(&File_ptr, w_save_name, L"rb");
    if (error == 0)
#elif defined(QFO2_LINUX)
    File_ptr = fopen(save_path_name, "rb");
    if (File_ptr != NULL)
#endif
    {
        fclose(File_ptr);
        // handles the case where the file exists
        char* ptr = strrchr(save_path_name, PLATFORM_SLASH);
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
void Create_File_Name(char* return_buffer, const char* name, img_type save_type, char* save_path, int tile_num)
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

//TODO: replace this with something that saves unique data type
//      *.q or something, allowing user to open up whole worldmap
//      FRM/MSK file to edit without exporting to tiles
void Save_Full_MSK_OpenGL(image_data* img_data, user_info* usr_info)
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
    uint8_t* texture_buffer = (uint8_t*)malloc(texture_size);
    // copy edited texture to buffer, combine with original image
    glBindTexture(GL_TEXTURE_2D, img_data->MSK_texture);
    // read pixels into buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer);

    // get filename
    FILE* File_ptr = NULL;
    char* Save_File_Name;
    const char* lFilterPatterns[2] = {"*.MSK", ""};
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
    wchar_t* w_save_name = tinyfd_utf8to16(Save_File_Name);
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

//TODO: rename, no longer uses OpenGL
void Save_MSK_Image_OpenGL(uint8_t* tile_buffer, FILE* File_ptr, int width, int height)
{
    // int buff_size = ceil(width / 8.0f) * height;
    int buff_size = (width + 7) / 8 * height;

    // final output buffer
    //TODO: replace with stack buffer[]
    uint8_t *out_buffer = (uint8_t *)malloc(buff_size);

    int shift = 0;
    uint8_t bitmask = 0;
    uint8_t* outp = out_buffer;
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

//TODO: implement
void Save_to_GIF(image_data* img_data, struct user_info* usr_nfo)
{
    if (img_data->FRM_data == nullptr) {
        return;
    }
    
}