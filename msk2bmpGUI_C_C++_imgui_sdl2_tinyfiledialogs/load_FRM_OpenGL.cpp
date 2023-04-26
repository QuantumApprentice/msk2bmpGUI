#include "load_FRM_OpenGL.h"
#include "B_Endian.h"

//returns true/false for success/failure
bool init_framebuffer(struct image_data* img_data)
{
    glDeleteFramebuffers(1, &img_data->framebuffer);
    //glDeleteTextures(1, &img_data->render_texture);
    glGenTextures(1, &img_data->render_texture);
    glBindTexture(GL_TEXTURE_2D, img_data->render_texture);
    //set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //allocate video memory for texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
        img_data->width, img_data->height,
        0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    //init framebuffer
    glGenFramebuffers(1, &img_data->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, img_data->framebuffer);

    //attach texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, img_data->render_texture, 0);
    glViewport(0, 0, img_data->width, img_data->height);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
        //reset texture bind to default
        glBindTexture(GL_TEXTURE_2D, 0);
        return false;
    }
    else {
        //reset texture and framebuffer bind to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        return true;
    }
}

void Orientation(uint16_t Frame_0_Orient[6])
{
    for (int i = 0; i < 6; i++)
    {
        B_Endian::swap_16(Frame_0_Orient[i]);
    }
}

void Offset(uint32_t Frame_0_Offset[6])
{
    for (int i = 0; i < 6; i++)
    {
        B_Endian::swap_32(Frame_0_Offset[i]);
    }
}

void flip_header_endian(FRM_Header* header)
{
    B_Endian::swap_32(header->version);
    B_Endian::swap_16(header->FPS);
    B_Endian::swap_16(header->Action_Frame);
    B_Endian::swap_16(header->Frames_Per_Orient);
    Orientation(header->Shift_Orient_x);
    Orientation(header->Shift_Orient_y);
    Offset(header->Frame_0_Offset);
    B_Endian::swap_32(header->Frame_Area);

}

void flip_frame_endian(FRM_Frame_Info* frame_info)
{
    B_Endian::swap_16(frame_info->Frame_Width);
    B_Endian::swap_16(frame_info->Frame_Height);
    B_Endian::swap_32(frame_info->Frame_Size);
    B_Endian::swap_16(frame_info->Shift_Offset_x);
    B_Endian::swap_16(frame_info->Shift_Offset_y);
}

uint8_t* load_entire_file(const char* file_name, int* file_size)
{
    FILE* File_ptr;
    int file_length = 0;
    fopen_s(&File_ptr, file_name, "rb");
    if (!File_ptr) {
        printf("error, can't open file");
        return NULL;
    }
    int error = fseek(File_ptr, 0, SEEK_END);
    if (!error) {
        file_length = ftell(File_ptr);
        fseek(File_ptr, 0, SEEK_SET);
    }
    else {
        return NULL;
    }
    uint8_t* buffer = (uint8_t*)malloc(file_length);
    fread(buffer, file_length, 1, File_ptr);
    fclose(File_ptr);

    if (file_size != NULL) {
        *file_size = file_length;
    }

    return buffer;
}

bool load_FRM_header(const char* file_name, image_data* img_data)
{
    int file_size = 0;
    int buff_offset = 0;
    int info_size = sizeof(FRM_Frame_Info);
    int hdr_size  = sizeof(FRM_Header);
    uint8_t* buffer = load_entire_file(file_name, &file_size);
    FRM_Header* header = (FRM_Header*)buffer;
    flip_header_endian(header);

    FRM_Frame_Info* frame_info;

    int num_orients = (header->Frame_0_Offset[1]) ? 6 : 1;
    int num_frames  = header->Frames_Per_Orient;

    img_data->Frame = (FRM_Frame*)malloc(header->Frames_Per_Orient * sizeof(FRM_Frame*) * num_orients);

    buff_offset = hdr_size;
    for (int i = 0; i < num_orients; i++)
    {
        for (int j = 0; j < num_frames; j++)
        {
            frame_info = (FRM_Frame_Info*)(buffer + buff_offset);
            flip_frame_endian(frame_info);
            //img_data->Frame->frame_info->Frame_Height   = frame_info->Frame_Height;
            //img_data->Frame->frame_info->Frame_Width    = frame_info->Frame_Width;
            //img_data->Frame->frame_info->Frame_Size     = frame_info->Frame_Size;
            //img_data->Frame->frame_info->Shift_Offset_x = frame_info->Shift_Offset_x;
            //img_data->Frame->frame_info->Shift_Offset_y = frame_info->Shift_Offset_y;
            img_data->Frame[j + (i * num_frames)].frame_number  = j;
            img_data->Frame[j + (i * num_frames)].orientation   = i;
            img_data->Frame[j + (i * num_frames)].frame_info    = frame_info;

            buff_offset += frame_info->Frame_Size + info_size;
        }
    }





    uint16_t buffer_16;
    uint32_t buffer_32;

    //img_data->FRM_Info.version                  = B_Endian::write_u32(*((uint32_t*)(buff_ptr)));
    //img_data->FRM_Info.FPS                      = B_Endian::write_u16(*((uint16_t*)(buff_ptr + 4)));
    //img_data->FRM_Info.Action_Frame             = B_Endian::write_u16(*((uint16_t*)(buff_ptr + 6)));
    //img_data->FRM_Info.Frames_Per_Orientation   = B_Endian::write_u16(*((uint16_t*)(buff_ptr + 8)));

    //memcpy(img_data->FRM_Info.Shift_Orient_x, Orientation(File_ptr), sizeof(img_data->FRM_Info.Shift_Orient_x));
    //img_data->FRM_Info.Shift_Orient_y = Orientation(File_ptr);
    //img_data->FRM_Info.Frame_0_Offset = Offset(File_ptr);
    //fread(&buffer_32, 4, 1, File_ptr);
    //img_data->FRM_Info.Frame_Area = B_Endian::write_u32(buffer_32);
    //fread(&buffer_16, 2, 1, File_ptr);


    //img_data->FRM_Info.Frame_0_Width = B_Endian::write_u16(buffer_16);
    //fread(&buffer_16, 2, 1, File_ptr);
    //img_data->FRM_Info.Frame_0_Height = B_Endian::write_u16(buffer_16);
    //fread(&buffer_32, 4, 1, File_ptr);
    //img_data->FRM_Info.Frame_0_Size = B_Endian::write_u32(buffer_32);
    //fread(&buffer_16, 2, 1, File_ptr);
    //img_data->FRM_Info.Shift_Offset_x = B_Endian::write_u16(buffer_16);
    //fread(&buffer_16, 2, 1, File_ptr);
    //img_data->FRM_Info.Shift_Offset_y = B_Endian::write_u16(buffer_16);
    return true;
}







//load FRM image from char* file_name
//stores GLuint and size info to img_data
//returns true on success, else false
bool load_FRM_OpenGL(const char* file_name, image_data* img_data)
{
    //load & gen texture
    glGenTextures(1, &img_data->FRM_texture);
    glBindTexture(GL_TEXTURE_2D, img_data->FRM_texture);
    //texture settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int frm_number;
    int frm_size;
    int frm_width;
    int frm_height;

    bool success = load_FRM_header(file_name, img_data);
    if (success) {
        frm_number  = img_data->FRM_Info.Frames_Per_Orient;

        frm_size    = img_data->Frame->frame_info->Frame_Size;
        frm_width   = img_data->Frame->frame_info->Frame_Width;
        frm_height  = img_data->Frame->frame_info->Frame_Height;

        img_data->width  = frm_width;
        img_data->height = frm_height;
    }



    //uint16_t frm_number = 0;
    //fseek(File_ptr, 0x0008, SEEK_SET);
    //fread(&frm_number, 2, 1, File_ptr);
    //frm_number = B_Endian::write_u16(frm_number);

    //uint16_t frm_width  = 0;
    //uint16_t frm_height = 0;
    //uint32_t frm_size   = 0;
    //fseek(File_ptr, 0x3E, SEEK_SET);
    //fread(&frm_width, 2, 1, File_ptr);
    //frm_width = B_Endian::write_u16(frm_width);
    //fread(&frm_height, 2, 1, File_ptr);
    //frm_height = B_Endian::write_u16(frm_height);
    //fread(&frm_size, 4, 1, File_ptr);
    //frm_size = B_Endian::write_u32(frm_size);




    //read in FRM data including animation frames

    if (img_data->FRM_data) {
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //FRM's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //bind data to FRM_texture for display
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frm_width, frm_height, 0, GL_RED, GL_UNSIGNED_BYTE, img_data->FRM_data);

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        img_data->FRM_data = img_data->FRM_data;
        return true;
    }
    else {
        printf("FRM image didn't load...\n");
        return false;
    }
}