#include <stdio.h>
#include <SDL_image.h>

#include <cstdint>

#include "Load_Files.h"
#include "Load_Animation.h"
#include "Edit_Animation.h"
//#include "tinyfiledialogs.h"


bool Drag_Drop_Load_Animation(std::vector <std::filesystem::path>& path_vector, LF* F_Prop)
{
    char buffer[MAX_PATH];
    char direction[MAX_PATH];
    image_data* img_data = &F_Prop->img_data;
    std::sort(path_vector.begin(), path_vector.end());

    snprintf(direction, MAX_PATH, "%s", path_vector[0].parent_path().filename().u8string().c_str());
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", path_vector[0].u8string().c_str());

    F_Prop->c_name    = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.'  ) + 1;

    //char* dir_ptr = strrchr(path_vector, '/\\');
    //snprintf(buffer, (strlen(file_names[0]) - (strlen(dir_ptr)-1)), "%s", file_names[0]);
    //dir_ptr = strrchr(buffer, '/\\');
    //snprintf(direction, strlen(dir_ptr), "%s", dir_ptr+1);

    //TODO: probably don't want to store folder name here?
    //snprintf(F_Prop->Opened_File, MAX_PATH, "%s", direction);

    Direction temp_orient = assign_direction(direction);
    int num_frames = path_vector.size();
    if (img_data->ANM_dir == NULL) {
        img_data->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir) * 6);
        new(img_data->ANM_dir) ANM_Dir[6];
        if (!img_data->ANM_dir) {
            //TODO: change to tinyfd_dialog() warning
            printf("Unable to allocate enough memory");
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
        //TODO: change to tinyfd_dialog() warning
        printf("Unable to allocate enough memory");
        return false;
    }
    else {
        img_data->ANM_dir[temp_orient].frame_data = frame_data;
    }


    for (int i = 0; i < path_vector.size(); i++)
    {
        frame_data[i].frame_start  = IMG_Load(path_vector[i].u8string().c_str());
        frame_data[i].Frame_Width  = frame_data[i].frame_start->w;
        frame_data[i].Frame_Height = frame_data[i].frame_start->h;
        frame_data[i].Shift_Offset_x = 0;
        frame_data[i].Shift_Offset_y = 0;
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
        SDL_Surface* data = img_data->ANM_dir[temp_orient].frame_data[0].frame_start;
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //FRM's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        //bind data to FRM_texture for display
        //uint8_t * blank = (uint8_t*)calloc(1, data->w*data->h);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, data->w, data->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data->pixels);
        //glTexSubImage2D(GL_TEXTURE_2D, 0, x_offset, y_offset, frm_width, frm_height, GL_RED, GL_UNSIGNED_BYTE, data);
        //free(blank);

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        return true;
    }
    else {
        printf("FRM image didn't load...\n");
        return false;
    }

    prep_extension(F_Prop, NULL);
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

void set_names(char** names_array, image_data* img_data)
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

void Gui_Video_Controls(image_data* img_data, img_type type)
{
    //gui video controls
    ImGui::SetCursorPosY(ImGui::GetContentRegionMax().y - 80);
    const char* speeds[] = { "Pause", "1/4x", "1/2x", "Play", "2x" };
    ImGui::Combo("Playback Speed", &img_data->playback_speed, speeds, IM_ARRAYSIZE(speeds));

    //TODO: definitely have to adjust for .FR1, FR2, etc...
    //populate names[] only with existing directions
    char* names[6];
    set_names(names, img_data);
    ImGui::Combo("Direction", &img_data->display_orient_num, names, IM_ARRAYSIZE(names));
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
    }

    if (type == OTHER) {
        if (img_data->ANM_dir[img_data->display_orient_num].num_frames > 0) {
            max_frame = img_data->ANM_dir[img_data->display_orient_num].num_frames - 1;
        }
        else {
            img_data->display_frame_num = 0;
            max_frame = 0;
        }
        ImGui::SliderInt("Frame Number", &img_data->display_frame_num, 0,
            max_frame, NULL);
    }
    else if (type == FRM) {
        if (img_data->FRM_dir[img_data->display_orient_num].num_frames > 0) {
            max_frame = img_data->FRM_dir[img_data->display_orient_num].num_frames - 1;
        }
        else {
            img_data->display_frame_num = 0;
            max_frame = 0;
        }
        ImGui::SliderInt("Frame Number", &img_data->display_frame_num, 0,
            max_frame, NULL);
    }
}