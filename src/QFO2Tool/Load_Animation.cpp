#include <stdio.h>
#include <cstdint>
#include <algorithm>

#include "Load_Files.h"
#include "Load_Animation.h"
#include "Edit_Animation.h"
#include "tinyfiledialogs.h"
#include "ImGui_Warning.h"


bool Drag_Drop_Load_Animation(std::vector <std::filesystem::path>& path_set, LF* F_Prop)
{
    char buffer[MAX_PATH];
    char direction[MAX_PATH];
    image_data* img_data = &F_Prop->img_data;
    std::sort(path_set.begin(), path_set.end());

    snprintf(direction, MAX_PATH, "%s", (*path_set.begin()).parent_path().filename().u8string().c_str());
    //TODO: test this!          8==D
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", (*path_set.begin()).parent_path().parent_path().u8string().c_str());

    // store filepaths in this directory for navigating through
    Next_Prev_File(F_Prop->Next_File,
                   F_Prop->Prev_File,
                   F_Prop->Frst_File,
                   F_Prop->Last_File,
                   F_Prop->Opened_File);



    F_Prop->c_name    = strrchr(F_Prop->Opened_File, PLATFORM_SLASH) + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    Direction temp_orient = assign_direction(direction);
    int num_frames = path_set.size();
    if (img_data->ANM_dir == NULL) {
        img_data->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir) * 6);
        if (!img_data->ANM_dir) {
            //TODO: log out to txt file
            set_popup_warning("Unable to allocate enough memory");
            printf("Unable to allocate enough memory\n");
            return false;
        } else {
            new(img_data->ANM_dir) ANM_Dir[6];
        }
    }



    img_data->ANM_dir[temp_orient].orientation = temp_orient;
    if (img_data->ANM_dir[temp_orient].num_frames != num_frames) {
        img_data->ANM_dir[temp_orient].num_frames  = num_frames;
    }

    ANM_Frame* frame_data = img_data->ANM_dir[temp_orient].frame_data;
    if (frame_data != NULL) {
        memset(frame_data, 0, sizeof(ANM_Frame));
        free(frame_data);
    }
    frame_data = (ANM_Frame*)malloc(sizeof(ANM_Frame) * num_frames);
    if (!frame_data) {
        //TODO: log out to txt file
        set_popup_warning("Unable to allocate enough memory");
        printf("Unable to allocate enough memory");
        return false;
    } else {
        img_data->ANM_dir[temp_orient].frame_data = frame_data;
    }

    //iterate over images in directory provided and assign to frame_data[]
    int i = 0;
    for (const std::filesystem::path& path : path_set) {

        frame_data[i].frame_start  = Load_File_to_RGBA(path.u8string().c_str());
        // frame_data[i].frame_start  = IMG_Load(path.u8string().c_str());
        //handle image bit depth less than 32bpp
        // frame_data[i].frame_start  = Surface_32_Check(frame_data[i].frame_start);

        frame_data[i].Frame_Width  = frame_data[i].frame_start->w;
        frame_data[i].Frame_Height = frame_data[i].frame_start->h;
        frame_data[i].Shift_Offset_x = 0;
        frame_data[i].Shift_Offset_y = 0;
        i++;
    }

    F_Prop->img_data.type = OTHER;
    //TODO: refactor img_data.width/height out in favor of FRM_boundary_box?
    img_data->width  = frame_data[0].frame_start->w;
    img_data->height = frame_data[0].frame_start->h;
    img_data->display_orient_num = temp_orient;

    //load & gen texture
    glGenTextures(1, &img_data->FRM_texture);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (img_data->ANM_dir[temp_orient].frame_data) {
        Surface* data = img_data->ANM_dir[temp_orient].frame_data[0].frame_start;
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //FRM's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data->w, data->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data->pxls);

        bool success = init_framebuffer(img_data);
        if (!success) {
            //TODO: log out to txt file
            set_popup_warning(
                "image framebuffer failed to attach correctly?\n"
            );
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        return true;
    }
    else {
        //TODO: log out to txt file
        set_popup_warning("FRM image didn't load...\n");
        printf("FRM image didn't load...\n");
        return false;
    }

    prep_extension(F_Prop, NULL, (char*)(*path_set.begin()).parent_path().u8string().c_str());
    return true;
}

Direction assign_direction(char* direction)
{
    if (!strncmp(direction, "NE\0", sizeof("NE\0"))) {
        return NE;
    }
    if (!strncmp(direction, "E\0",  sizeof("E\0")))  {
        return E;
    }
    if (!strncmp(direction, "SE\0", sizeof("SE\0"))) {
        return SE;
    }
    if (!strncmp(direction, "SW\0", sizeof("SW\0"))) {
        return SW;
    }
    if (!strncmp(direction, "W\0",  sizeof("W\0")))  {
        return W;
    }
    if (!strncmp(direction, "NW\0", sizeof("NW\0"))) {
        return NW;
    }
    //default
    return NE;
}

void set_directions(const char** names_array, image_data* img_data)
{
    Direction* dir_ptr = NULL;

    for (int i = 0; i < 6; i++)
    {
        if (img_data->type == OTHER) {
            dir_ptr = &img_data->ANM_dir[i].orientation;
        }
        else if (img_data->type == FRM ) {
            dir_ptr = &img_data->FRM_dir[i].orientation;
        }
        assert(dir_ptr != NULL && "Not FRM or OTHER?");
        switch (*dir_ptr)
        {
        case(NE):
            names_array[i] = "NE";
            break;
        case(E):
            names_array[i] = "E";
            break;
        case(SE):
            names_array[i] = "SE";
            break;
        case(SW):
            names_array[i] = "SW";
            break;
        case(W):
            names_array[i] = "W";
            break;
        case(NW):
            names_array[i] = "NW";
            break;
        default:
            names_array[i] = "no image";
            break;
        }
    }
}

void Clear_img_data(image_data* img_data)
{
    if (img_data->MSK_data) {
        free(img_data->MSK_data);
        free(img_data->MSK_srfc);
        img_data->MSK_data = NULL;
        img_data->MSK_srfc = NULL;
    }
    if (img_data->FRM_data) {
        free(img_data->FRM_data);
        img_data->FRM_data = NULL;
        img_data->FRM_hdr  = NULL;
        img_data->FRM_dir  = NULL;
    }
    if (img_data->ANM_dir) {
        for (int i = 0; i < 6; i++) {
            if (img_data->ANM_dir[i].frame_data) {
                //TODO: check if number of frames are set for individual images
                for (int j = 0; j < img_data->ANM_dir[i].num_frames; j++)
                {
                    FreeSurface(img_data->ANM_dir[i].frame_data[j].frame_start);
                }
                free(img_data->ANM_dir[i].frame_data);
                img_data->ANM_dir[i].frame_data = NULL;
            }
        }
        free(img_data->ANM_dir);
        img_data->ANM_dir = NULL;
    }
}

void Next_Prev_Buttons(LF* F_Prop, image_data* img_data, shader_info* shaders)
{
    ImVec2 origin = ImGui::GetCursorPos();
    // position buttons based on bottom right edge of window
    ImVec2 wind_pos = ImGui::GetWindowSize();
    ImVec2 button_size = { 50, 70 };
    ImVec2 button_pos;
    ImVec2 scrl_pos;
    scrl_pos.x = ImGui::GetScrollX();
    scrl_pos.y = ImGui::GetScrollY();

    button_pos.x = wind_pos.x + scrl_pos.x - button_size.x - 20;
    button_pos.y = wind_pos.y + scrl_pos.y - button_size.y - 20;

    ImGui::SetCursorPosX(button_pos.x);
    ImGui::SetCursorPosY(button_pos.y);

    char* current_file = NULL;

    bool check_file_type = false;
    if (ImGui::Button("next", button_size) || ImGui::IsKeyPressed(ImGuiKey_Period)) {
        if (strlen(F_Prop->Next_File)) {
            Clear_img_data(img_data);
            current_file    = F_Prop->Next_File;
            check_file_type = true;
        } else {
            //TODO: log to file
            set_popup_warning(
                "Found a file type that stb_image can load,\n"
                "but is not in Supported_Format().\n"
                "Please report this bug so I can fix it. :)"
            );
        }
    }

    ImGui::SetCursorPosX(button_pos.x - button_size.x - 2);
    ImGui::SetCursorPosY(button_pos.y);

    if (ImGui::Button("prev", button_size) || ImGui::IsKeyPressed(ImGuiKey_Comma)) {
        if (strlen(F_Prop->Next_File)) {
            Clear_img_data(img_data);
            current_file    = F_Prop->Prev_File;
            check_file_type = true;
        } else {
            set_popup_warning(
                "Found a file type that stb_image can load,\n"
                "but is not in Supported_Format().\n"
                "Please report this bug so I can fix it. :)"
            );
        }
    }



    ImGui::SetCursorPos(origin);

    if (check_file_type) {
        File_Type_Check(F_Prop, shaders, img_data, current_file);
    }
}

void Gui_Video_Controls(image_data* img_data, img_type type)
{
    ImVec2 origin = ImGui::GetCursorPos();
    // position buttons based on bottom right edge of window
    ImVec2 wind_pos = ImGui::GetWindowSize();
    ImVec2 scrl_pos;
    //TODO: might need scrl_pos.x in the future?
    //scrl_pos.x = ImGui::GetScrollX();
    scrl_pos.y = ImGui::GetScrollY();

    //gui video controls
    ImGui::SetCursorPosY(wind_pos.y + scrl_pos.y - 80);
    const char* speeds[] = { "Pause", "1/4x", "1/2x", "Play", "2x" };
    ImGui::Combo("Playback Speed", &img_data->playback_speed, speeds, IM_ARRAYSIZE(speeds));

    //populate directions[] only with existing directions
    const char* directions[6];
    set_directions(directions, img_data);
    ImGui::Combo("Direction", &img_data->display_orient_num, directions, IM_ARRAYSIZE(directions));
    int max_frame = 0;

    if (ImGui::IsWindowFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            static int last_selected_speed = 3;         //3 is index value for 1.0x speed in playback_speeds[]
            if (img_data->playback_speed == 0) {
                img_data->playback_speed = last_selected_speed;
            }
            else {
                last_selected_speed = img_data->playback_speed;
                img_data->playback_speed = 0;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
            img_data->display_frame_num++;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
            img_data->display_frame_num--;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            img_data->display_orient_num++;
            if (img_data->display_orient_num > 5) {
                img_data->display_orient_num = 0;
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            img_data->display_orient_num--;
            if (img_data->display_orient_num < 0) {
                img_data->display_orient_num = 5;
            }
        }
    }

    if (type == OTHER) {
        if (img_data->ANM_dir[img_data->display_orient_num].num_frames > 0) {
            max_frame = img_data->ANM_dir[img_data->display_orient_num].num_frames - 1;
        }
        else {
            img_data->display_frame_num = 0;
            max_frame = 0;
        }
        ImGui::SliderInt("Frame Number", &img_data->display_frame_num, 0, max_frame, NULL);
    }
    else if (type == FRM) {
        if (img_data->FRM_dir[img_data->display_orient_num].num_frames > 0) {
            max_frame = img_data->FRM_dir[img_data->display_orient_num].num_frames - 1;
        }
        else {
            img_data->display_frame_num = 0;
            max_frame = 0;
        }
        ImGui::SliderInt("Frame Number", &img_data->display_frame_num, 0, max_frame, NULL);
    }
    if (img_data->display_frame_num > max_frame) {
        img_data->display_frame_num = max_frame;
    }
    else if (img_data->display_frame_num < 0) {
             img_data->display_frame_num = 0;
    }

    ImGui::SetCursorPos(origin);
}