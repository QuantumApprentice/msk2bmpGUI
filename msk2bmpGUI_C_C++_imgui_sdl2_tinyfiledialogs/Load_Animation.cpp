#include <stdio.h>
#include <SDL_image.h>

#include <cstdint>

#include "Load_Files.h"
#include "Load_Animation.h"
#include "tinyfiledialogs.h"





bool Drag_Drop_Load_Animation(std::vector <std::filesystem::path>& path_vector, LF* F_Prop)
{
    char buffer[MAX_PATH];
    char direction[MAX_PATH];
    image_data* img_data = &F_Prop->img_data;

    snprintf(direction, MAX_PATH, "%s", tinyfd_utf16to8(path_vector[0].parent_path().filename().c_str()));
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", tinyfd_utf16to8(path_vector[0].c_str()));

    F_Prop->c_name    = strrchr(F_Prop->Opened_File, '/\\') + 1;
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.')   + 1;

    //char* dir_ptr = strrchr(path_vector, '/\\');
    //snprintf(buffer, (strlen(file_names[0]) - (strlen(dir_ptr)-1)), "%s", file_names[0]);
    //dir_ptr = strrchr(buffer, '/\\');
    //snprintf(direction, strlen(dir_ptr), "%s", dir_ptr+1);

    //TODO: probably don't want to store folder name here?
    //snprintf(F_Prop->Opened_File, MAX_PATH, "%s", direction);

    int num_frames = path_vector.size();
    if (img_data->ANIM_hdr == NULL) {
        img_data->ANIM_hdr = (Anim_Header*)malloc(sizeof(Anim_Header));
        img_data->ANIM_hdr->Frames_Per_Orient = num_frames;
    }

    if (img_data->ANIM_frame == NULL) {
        img_data->ANIM_frame = (Anim_Frame*)malloc(sizeof(Anim_Frame) * 6);
    }

    Anim_Frame_Info* frame_info = (Anim_Frame_Info*)malloc(sizeof(Anim_Frame_Info) * num_frames);
    Orientation temp_orient = assign_direction(direction);

    std::sort(path_vector.begin(), path_vector.end());

    img_data->ANIM_frame[temp_orient].orientation = temp_orient;


    for (int i = 0; i < path_vector.size(); i++)
    {
        char* converted_path = tinyfd_utf16to8(path_vector[i].c_str());
        frame_info[i].frame_start  = IMG_Load(converted_path);
        frame_info[i].Frame_Width  = frame_info[i].frame_start->w;
        frame_info[i].Frame_Height = frame_info[i].frame_start->h;
    }

    img_data->ANIM_frame[temp_orient].frame_info = frame_info;


    F_Prop->type = OTHER;
    img_data->width  = frame_info[0].frame_start->w;
    img_data->height = frame_info[0].frame_start->h;
    img_data->display_orient_num = temp_orient;





    //load & gen texture
    glGenTextures(1, &img_data->FRM_texture);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (img_data->ANIM_frame[temp_orient].frame_info) {
        SDL_Surface* data = img_data->ANIM_frame[temp_orient].frame_info[0].frame_start;
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

Orientation assign_direction(char* direction)
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
    for (int i = 0; i < 6; i++)
    {
        switch (img_data->ANIM_frame[i].orientation)
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