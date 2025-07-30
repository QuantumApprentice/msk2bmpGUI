#include <string.h>
#include <ImFileDialog.h>
#include <glad/glad.h>

#include "platform_dialogs.h"







//initialize Ifd::FileDialog system
//by setting callbacks that create and delete thumbnail textures
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

//returns NULL while window is open
//returns "\0" on cancel
//returns buffer ptr on select
char* ImDialog_save_folder(user_info* usr_nfo)
{

   //TODO: move this to initialize at program start?
    init_IFD();
    char* folder = usr_nfo->default_save_path;
    ifd::FileDialog::Instance().Open("FileSaveDialog", "Save Folder", "", false, folder);

    // static bool success   = false;
    // static bool overwrite = false;
    static char save_folder[MAX_PATH];
    folder = NULL;
    // static char save_path[MAX_PATH];
    // if (ImGui::BeginPopupModal("Match found"))
    // {
    //     char* dup_name = strrchr(save_path, PLATFORM_SLASH)+1;
    //     ImGui::Text(
    //         "%s already exists,\n\n", dup_name
    //     );
    //     if (ImGui::Button("Overwrite?")) {
    //         ImGui::CloseCurrentPopup();
    //         overwrite = true;
    //         success   = true;
    //     }
    //     if (ImGui::Button("Select a different folder?")) {
    //         ImGui::CloseCurrentPopup();
    //         save_folder[0] = '\0';
    //         save_path[0]   = '\0';
    //         overwrite      = false;
    //         success        = false;
    //         ifd::FileDialog::Instance().Open("FileSaveDialog", "Save File", "", false, folder);
    //     }
    //     if (ImGui::Button("Cancel")) {
    //         ImGui::CloseCurrentPopup();
    //         free(selected);
    //         selected       = NULL;
    //         save_folder[0] = '\0';
    //         save_path[0]   = '\0';
    //         overwrite      = false;
    //         success        = false;
    //         ImGui::EndPopup();
    //         return false;
    //     }
    //     ImGui::EndPopup();
    //     return true;
    // }

    if (ifd::FileDialog::Instance().IsDone("FileSaveDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(save_folder, temp.c_str(), temp.length()+1);
            strncpy(usr_nfo->default_save_path, temp.c_str(), temp.length()+1);
            folder = save_folder;
            // success = true;
        } else {
            folder = "\0";
            // printf("closing\n");
        }
        ifd::FileDialog::Instance().Close();
    }


    // if (strlen(save_folder) > 0 && success) {
    //     success = save_tiles_SURFACE(save_folder, save_name, save_path,
    //                 selected, src, type, usr_nfo, sv_info, overwrite);
    // }
    // if (success) {
    //     free(selected);
    //     selected       = NULL;
    //     save_folder[0] = '\0';
    //     save_path[0]   = '\0';
    //     overwrite      = false;
    //     success        = false;
    //     return false;
    // }


    // return success;
    return folder;
}
