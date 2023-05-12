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
    Anim_Header* header = (Anim_Header*)malloc(sizeof(Anim_Header));
    header->Frames_Per_Orient = num_frames;
    Anim_Frame* frame = (Anim_Frame*)malloc(sizeof(Anim_Frame));

    Anim_Frame_Info* frame_info = (Anim_Frame_Info*)malloc(sizeof(Anim_Frame_Info) * num_frames);

    std::sort(path_vector.begin(), path_vector.end());

    for (int i = 0; i < path_vector.size(); i++)
    {
        frame_info[i].frame_start  = IMG_Load(tinyfd_utf16to8(path_vector[i].c_str()));
    }

    F_Prop->img_data.ANIM_hdr = header;
    F_Prop->img_data.ANIM_frame = frame;
    F_Prop->img_data.ANIM_frame->frame_info = frame_info;

    F_Prop->type = OTHER;
    F_Prop->img_data.width  = frame_info->frame_start->w;
    F_Prop->img_data.height = frame_info->frame_start->h;


    //assign_direction(direction, &frame_1);



    prep_extension(F_Prop, NULL);
    return true;
}

void assign_direction(char* direction, Anim_Frame* frame)
{
    if      (!strncmp(direction, "NE\0", sizeof("NE\0"))) {
        frame->orientation = NE;
    }
    else if (!strncmp(direction, "E\0",  sizeof("E\0")))  {
        frame->orientation = E;
    }
    else if (!strncmp(direction, "SE\0", sizeof("SE\0"))) {
        frame->orientation = SE;
    }
    else if (!strncmp(direction, "SW\0", sizeof("SW\0"))) {
        frame->orientation = SW;
    }
    else if (!strncmp(direction, "W\0",  sizeof("W\0")))  {
        frame->orientation = W;
    }
    else if (!strncmp(direction, "NW\0", sizeof("NW\0"))) {
        frame->orientation = NW;
    }

}