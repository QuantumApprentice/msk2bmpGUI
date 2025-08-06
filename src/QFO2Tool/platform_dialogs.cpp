#include <string.h>
#include <ImFileDialog.h>
#include <glad/glad.h>

#include "platform_dialogs.h"



//https://github.com/dfranx/ImFileDialog
//initialize Ifd::FileDialog system
//  by setting callbacks that create and delete thumbnail textures
void init_IFD()
{
    //TODO: move this to some initializing function
    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;
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

//returns NULL while window is open
//returns "\0" on cancel
//returns buffer ptr on select
char* ImDialog_save_folder(user_info* usr_nfo)
{

   //TODO: move this to initialize at program start?
    init_IFD();

    char* folder = usr_nfo->default_save_path;
    ifd::FileDialog::Instance().Open("FileSaveDialog", "Save Folder", "", false, folder);

    static char save_folder[MAX_PATH];
    folder = NULL;

    if (ifd::FileDialog::Instance().IsDone("FileSaveDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(save_folder, temp.c_str(), temp.length()+1);
            strncpy(usr_nfo->default_save_path, temp.c_str(), temp.length()+1);
            folder = save_folder;
        } else {
            folder = "\0";
        }
        ifd::FileDialog::Instance().Close();
    }

    return folder;
}
