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

uint16_t* Orientation(FILE* File_ptr)
{
    int size = 6 * sizeof(uint16_t);
    uint16_t buffer;
    uint16_t* data = (uint16_t*)malloc(size);

    for (int i = 0; i < 6; i++)
    {
        fread(&buffer, sizeof(uint16_t), 1, File_ptr);
        data[i] = B_Endian::write_u16(buffer);
    }
    return data;
}

uint32_t* Offset(FILE* File_ptr)
{
    int size = 6 * sizeof(uint32_t);
    uint32_t buffer;
    uint32_t* data = (uint32_t*)malloc(size);

    for (int i = 0; i < 6; i++)
    {
        fread(&buffer, sizeof(uint32_t), 1, File_ptr);
        data[i] = B_Endian::write_u32(buffer);
    }
    return data;
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

    FILE* File_ptr;
    fopen_s(&File_ptr, file_name, "rb");
    if (!File_ptr) {
        printf("error, can't open file");
        return NULL;
    }

    uint16_t buffer_16;
    uint32_t buffer_32;

    fread(&buffer_32, 4, 1, File_ptr);
    img_data->FRM_Info.version                = B_Endian::write_u32(buffer_32);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.FPS                    = B_Endian::write_u16(buffer_16);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.Action_Frame           = B_Endian::write_u16(buffer_16);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.Frames_Per_Orientation = B_Endian::write_u16(buffer_16);
    img_data->FRM_Info.Shift_Orient_x         = Orientation(File_ptr);
    img_data->FRM_Info.Shift_Orient_y         = Orientation(File_ptr);
    img_data->FRM_Info.Frame_0_Offset         = Offset(File_ptr);
    fread(&buffer_32, 4, 1, File_ptr);
    img_data->FRM_Info.Frame_Area             = B_Endian::write_u32(buffer_32);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.Frame_0_Width          = B_Endian::write_u16(buffer_16);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.Frame_0_Height         = B_Endian::write_u16(buffer_16);
    fread(&buffer_32, 4, 1, File_ptr);
    img_data->FRM_Info.Frame_0_Size           = B_Endian::write_u32(buffer_32);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.Shift_Offset_x         = B_Endian::write_u16(buffer_16);
    fread(&buffer_16, 2, 1, File_ptr);
    img_data->FRM_Info.Shift_Offset_y         = B_Endian::write_u16(buffer_16);


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

    int frm_size    = img_data->FRM_Info.Frame_0_Size;
    int frm_number  = img_data->FRM_Info.Frames_Per_Orientation;
    int frm_width   = img_data->FRM_Info.Frame_0_Width;
    int frm_height  = img_data->FRM_Info.Frame_0_Height;

    img_data->width  = frm_width;
    img_data->height = frm_height;

    fseek(File_ptr, 0x4A, SEEK_SET);

    uint8_t* data = (uint8_t*)malloc(frm_size * frm_number);
    //read in FRM data including animation frames
    fread(data, 1, frm_size * frm_number, File_ptr);
    fclose(File_ptr);

    if (data) {
        //Change alignment with glPixelStorei() (this change is global/permanent until changed back)
        //FRM's are aligned to 1-byte
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        //bind data to FRM_texture for display
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frm_width, frm_height, 0, GL_RED, GL_UNSIGNED_BYTE, data);

        bool success = false;
        success = init_framebuffer(img_data);
        if (!success) {
            printf("image framebuffer failed to attach correctly?\n");
            return false;
        }
        img_data->FRM_data = data;
        return true;
    }
    else {
        printf("FRM image didn't load...\n");
        return false;
    }
}