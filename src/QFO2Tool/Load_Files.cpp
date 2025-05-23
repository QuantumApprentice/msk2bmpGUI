#include <stdio.h>
#include <string.h>
// #include <stringapiset.h>
// #include <SDL_image.h>
#include <stb_image.h>

#ifdef QFO2_WINDOWS
#include <Windows.h>
#elif defined(QFO2_LINUX)
#include <unistd.h>
#endif

#include <filesystem>
#include <cstdint>
#include <system_error>
#include <algorithm>
#include <string_view>

#include "platform_io.h"

#include "Load_Files.h"
#include "Load_Animation.h"
#include "Load_Settings.h"
#include "Image2Texture.h"
#include "FRM_Convert.h"
#include "MSK_Convert.h"
#include "Edit_Image.h"

#include "display_FRM_OpenGL.h"

#include "timer_functions.h"
#include "ImGui_Warning.h"
#include "ImFileDialog.h"

char *Program_Directory()
{

#ifdef QFO2_WINDOWS
    wchar_t buff[MAX_PATH] = {};
    GetModuleFileNameW(NULL, buff, MAX_PATH);
    char *utf8_buff = strdup(io_wchar_utf8(buff));
#elif defined(QFO2_LINUX)
    char *utf8_buff = (char *)malloc(MAX_PATH * sizeof(char));

    ssize_t read_size = readlink("/proc/self/exe", utf8_buff, MAX_PATH - 1);
    if (read_size >= MAX_PATH || read_size == -1)
    {
        //TODO: log to file
        set_popup_warning(
            "[ERROR] Program Directory()\n\n"
            "Error reading program .exe location."
        );
        printf("Error reading program .exe location, read_size: %d", read_size);
        return NULL;
    }
    utf8_buff[read_size + 1] = '\0'; // append null to entire string

#endif

    char *ptr = strrchr(utf8_buff, PLATFORM_SLASH) + 1; // replace filename w/null leaving only directory
    *ptr = '\0';

    // MessageBoxW(NULL,
    //     buff,
    //     L"string",
    //     MB_ABORTRETRYIGNORE);

    return utf8_buff;
}

bool drag_drop_POPUP(variables* My_Variables, LF* F_Prop, image_paths* images_arr, int* counter)
{
    bool open = true;
    bool process_animation;
    if (ImGui::BeginPopupModal("Drag_Drop_Folder", &open)) {
        const char* name_ptr;
        for (int i = 0; i < 6; i++)
        {
            if (!images_arr[i].animation_images.empty()) {
                // name_ptr = (char*)images_arr[i].animation_images[0].c_str();
                name_ptr = images_arr[i].animation_images[0].u8string().c_str();
                break;
            }
        }

        ImGui::Text(
            "%s\n\n"
            "Is this a group of sequential animation frames?",
            name_ptr
        );

        if (ImGui::Button("Yep, everything in this folder \nis part of an animation.")) {
            open = false;
            process_animation = true;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();

            for (int i = 0; i < 6; i++) {
                if (images_arr[i].animation_images.empty()) {
                    continue;
                }
                F_Prop->file_open_window = Drag_Drop_Load_Animation(images_arr[i].animation_images, F_Prop);
            }
            if (F_Prop->file_open_window) {
                (*counter)++;
            }

            return true;
        }

        //TODO: remove individual image opening?
        //      seems like it might be a bit silly and not useful
        if (ImGui::Button("Nope, open all images up \nindividually.")) {
            open = false;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();

            for (int i = 0; i < 6; i++) {
                for (int j = 0; j < images_arr[i].animation_images.size(); j++) {
                    LF* F_Prop = &My_Variables->F_Prop[*counter];
                    const char* path = (char*)images_arr[i].animation_images[j].c_str();
                    F_Prop->file_open_window = File_Type_Check(F_Prop, &My_Variables->shaders, &F_Prop->img_data, path);
                    if (F_Prop->file_open_window) {
                        (*counter)++;
                    }
                }
            }

            return true;
        }


        if (ImGui::Button("Cancel")) {
            open = false;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }


        ImGui::EndPopup();
    }

    return false;
}

// Checks the file extension against known working extensions
// Returns true if any extension matches, else return false
bool Supported_Format(const std::filesystem::path &file)
{
    // array of compatible filetype extensions
    constexpr static NATIVE_STRING_TYPE supported[13][6]{
#ifdef QFO2_WINDOWS
        L".FRM", L".MSK", L".PNG",
        L".JPG", L".JPEG", L".BMP",
        L".GIF",
        L".FR0", L".FR1", L".FR2", L".FR3", L".FR4", L".FR5"
#elif defined(QFO2_LINUX)
        ".FRM", ".MSK",
        ".PNG", ".BMP", ".JPG", ".JPEG",
        ".GIF",
        ".FR0",".FR1", ".FR2", ".FR3", ".FR4", ".FR5"
#endif
    };
    int k = sizeof(supported) / (6 * sizeof(NATIVE_STRING_TYPE));


    // actual extension check
    int i = 0;
    while (i < k)
    {
        //compare extension to determine if file is viewable
        if (io_strncasecmp(file.extension().c_str(), supported[i], 6) == 0)
        {
            return true;
        }
        i++;
    }

    return false;
}

std::vector<std::filesystem::path> handle_subdirectory_vec(const std::filesystem::path &directory)
{
    std::vector<std::filesystem::path> animation_images;
    std::error_code error;
    for (const std::filesystem::directory_entry &file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] handle_subdirectory_vec()\n\n"
                "When checking if file_name is directory when loading file."
            );
            printf("Error: %s\nWhen checking if file_name is directory when loading file: %d", error.message(), __LINE__);
            return animation_images;
        }
        if (is_subdirectory) {
            // TODO: handle different directions in subdirectories?
            // animation_images = handle_subdirectory_vec(file.path());
            continue;
        }
        else if (Supported_Format(file)) {
            animation_images.push_back(file);
        }
    }

    // uint64_t start_time = start_timer();

    // std::sort(animation_images.begin(), animation_images.end());                                        // ~50ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end());                   // ~50ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //          [](std::filesystem::path& a, std::filesystem::path& b)
    //           { return (a.filename() < b.filename()); });                                               // ~35ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //          [](std::filesystem::path& a, std::filesystem::path& b)
    //           { return (strcmp(a.filename().string().c_str(), b.filename().string().c_str())); });      // ~75ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //          [](std::filesystem::path& a, std::filesystem::path& b)
    //           { return (wcscmp(a.c_str(), b.c_str()) < 0); });                                          // ~10ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //         [](std::filesystem::path& a, std::filesystem::path& b)
    //          { return (a.native() < b.native()); });                                                    // ~17ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //     [](std::filesystem::path& a, std::filesystem::path& b)
    //{ return (a < b); });                                                                               // ~47ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //          [](std::filesystem::path& a, std::filesystem::path& b)
    //           { const wchar_t* a_file = wcspbrk(a.c_str(), L"/\\");
    //             const wchar_t* b_file = wcspbrk(b.c_str(), L"/\\");
    //             return (wcscmp(a_file, b_file) < 0); });                                                // ~13ms
    //  size_t parent_path_size = directory.native().size();
    //  std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //             [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
    //              {
    //                const wchar_t* a_file = a.c_str() + parent_path_size;
    //                const wchar_t* b_file = b.c_str() + parent_path_size;
    //                return (wcscmp(a_file, b_file) < 0);
    //              });                                                                                     // ~1ms




    size_t parent_path_size = directory.native().size();
    std::sort(animation_images.begin(), animation_images.end(),
            [&parent_path_size](std::filesystem::path &a, std::filesystem::path &b)
            {
                int a_size = a.native().size();
                int b_size = b.native().size();
                int larger_size = (a_size > b_size) ? a_size : b_size;
                return (io_strncasecmp((a.c_str() + parent_path_size), (b.c_str() + parent_path_size), larger_size) < 0);
            });





    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //            [](std::filesystem::path& a, std::filesystem::path& b)
    //             {
    //                 std::wstring_view a_v = a.native();
    //                 std::wstring_view b_v = b.native();
    //                 a_v = a_v.substr(a_v.find_last_of(L"\\/"));
    //                 b_v = b_v.substr(b_v.find_last_of(L"\\/"));
    //                 return (a_v < b_v);
    //             });                                                                                   // ~9ms
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //            [](std::filesystem::path& a, std::filesystem::path& b)
    //             {
    //                 std::wstring_view a_v = a.native();
    //                 std::wstring_view b_v = b.native();
    //                 a_v = a_v.substr(a_v.find_last_of(L"\\/"));
    //                 b_v = b_v.substr(b_v.find_last_of(L"\\/"));
    //                 return (wcscmp(a_v.data(), b_v.data()) < 0);
    //             });                                                                                     // ~9ms
    // size_t parent_path_size = directory.native().size();
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //             [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
    //              {
    //                  std::wstring_view a_v = a.native();
    //                  std::wstring_view b_v = b.native();
    //                  a_v = a_v.substr(parent_path_size);
    //                  b_v = b_v.substr(parent_path_size);
    //                  return (a_v < b_v);
    //              });                                                                                    // ~2ms
    // size_t parent_path_size = directory.native().size();
    // std::sort(std::execution::seq, animation_images.begin(), animation_images.end(),
    //             [&parent_path_size](std::filesystem::path& a, std::filesystem::path& b)
    //              {
    //                  const std::wstring_view a_v = a.native().substr(parent_path_size);
    //                  const std::wstring_view b_v = b.native().substr(parent_path_size);
    //                  return (a_v < b_v);
    //              });                                                                                    // ~3ms

    // print_timer(start_time);

    return animation_images;
}

//Store directory files in memory for quick Next/Prev buttons
//Filepaths for files are assigned to pointers passed in
void Next_Prev_File(char *next, char *prev, char *frst, char *last, char *current)
{
    // LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
    // LARGE_INTEGER Frequency;
    // QueryPerformanceFrequency(&Frequency);
    // QueryPerformanceCounter(&StartingTime);

    std::filesystem::path w_next;
    std::filesystem::path w_prev;
    std::filesystem::path w_frst;
    std::filesystem::path w_last;
    std::filesystem::path w_current(current);
    const std::filesystem::path &directory = w_current.parent_path();
    size_t parent_path_size = directory.native().size();

    std::error_code error;
    for (const std::filesystem::directory_entry &file : std::filesystem::directory_iterator(directory))
    {
        bool is_subdirectory = file.is_directory(error);
        if (error)
        {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] Next_Prev_File()\n\n"
                "Error occurred when checking if file is directory.\n"
            );
            printf("Error: Checking if file is directory: %s : %d", file, __LINE__);
        }
        if (is_subdirectory)
        {
            // TODO: handle different directions in subdirectories?
            // handle_subdirectory(file.path());
            continue;
        }
        else
        {
            if (Supported_Format(file))
            {
                NATIVE_STRING_TYPE* iter_file = (file.path().c_str() + parent_path_size);
                //TODO:
                //move all filesystem things (includeing std::filesystem) into platform layer
                //and use windows conversion funcs to convert from wchar_t to utf8

                if (w_frst.empty() ||
                    //io_strncasecmp(iter_file, w_frst.filename().u8string().c_str(), MAX_PATH) < 0)
                    (io_strncasecmp(iter_file, (w_frst.c_str() + parent_path_size), MAX_PATH) < 0))
                {
                    w_frst = file;
                }
                if (w_last.empty() || 
                    //io_strncasecmp(iter_file, w_last.filename().u8string().c_str(), MAX_PATH) > 0)
                    (io_strncasecmp(iter_file, (w_last.c_str() + parent_path_size), MAX_PATH) > 0))
                {
                    w_last = file;
                }

                //int cmp = io_strncasecmp(iter_file, w_current.filename().u8string().c_str(), MAX_PATH);
                int cmp = io_strncasecmp(iter_file, (w_current.c_str() + parent_path_size), MAX_PATH);

                if (cmp < 0)
                {
                    if (w_prev.empty() || 
                        //io_strncasecmp(iter_file, w_prev.filename().u8string().c_str(), MAX_PATH) > 0)
                        (io_strncasecmp(iter_file, (w_prev.c_str() + parent_path_size), MAX_PATH) > 0))
                    {
                        w_prev = file;
                    }
                }
                else if (cmp > 0)
                {
                    if (w_next.empty() || 
                        //io_strncasecmp(iter_file, w_next.filename().u8string().c_str(), MAX_PATH) < 0)
                        (io_strncasecmp(iter_file, (w_next.c_str() + parent_path_size), MAX_PATH) < 0))
                    {
                        w_next = file;
                    }
                }
            }
        }
    }

    if (w_prev.empty()) {
        w_prev = w_last;
    }
    if (w_next.empty()) {
        w_next = w_frst;
    }

    strncpy(prev, w_prev.u8string().c_str(), MAX_PATH);
    strncpy(next, w_next.u8string().c_str(), MAX_PATH);
    strncpy(frst, w_frst.u8string().c_str(), MAX_PATH);
    strncpy(last, w_last.u8string().c_str(), MAX_PATH);

    // QueryPerformanceCounter(&EndingTime);
    // ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
    // ElapsedMicroseconds.QuadPart *= 1000000;
    // ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

    // printf("Next_Prev_File time: %d\n", ElapsedMicroseconds.QuadPart);
}

bool handle_directory_drop_POPUP(char* dir_name, image_paths* image_arr)
{
    bool is_dir = io_isdir(dir_name);
    if (!is_dir) {
        return false;
    }

    void* directory = io_open_dir(dir_name);
    if (directory == NULL) {
        //TODO: log to file
        set_popup_warning(
            "[ERROR] handle_directory_drop_POPUP()\n\n"
            "Error occurred when opening directory\n"
        );
        printf("Error: Unable to open directory? %s\n L%d\n", dir_name, __LINE__);
        return true;
    }

    //handle the case with appropriately named subfolders
    //  storing each direction's animations
    //  (NE/E/SE/SW/W/NW)
    Direction dir;
    char* name;
    while ((name = io_scan_dir(directory)) != NULL) {
        if (name[0] == '.') {
            //this also skips hidden directories in Linux (possibly also windows?)
            //need to change if I want to scan those
            continue;
        }

        //get the file?/folder? name?
        char buffer[MAX_PATH];
        snprintf(buffer, MAX_PATH, "%s%c%s", dir_name, PLATFORM_SLASH, name);
        // printf("d_name:   %s\n", iterator->d_name);

        if (io_isdir(buffer)) {
            char* sub_dir = strrchr(buffer, PLATFORM_SLASH)+1;
            dir = assign_direction(sub_dir);
            std::filesystem::path path(buffer);
            image_arr[dir].animation_images = handle_subdirectory_vec(path);
        }
    }

    bool err = io_close_dir(directory);
    if (err) {
        //TODO: log to file
        set_popup_warning(
            "[ERROR] handle_directory_drop_POPUP()\n\n"
            "Error occurred when closing directory\n"
        );
        printf("Error: Unable to close directory? %s\n L%d\n", dir_name, __LINE__);
    }

    for (int i = 0; i < 6; i++) {
        if (!image_arr[i].animation_images.empty()) {
            // printf("popping\n");
            ImGui::OpenPopup("Drag_Drop_Folder");
            return true;
        }
    }

    //handle the case where the dropped directory has images directly in it
    //  but no subdirectories
    char* sub_dir = strrchr(dir_name, PLATFORM_SLASH)+1;
    dir = assign_direction(sub_dir);
    std::filesystem::path path(dir_name);
    image_arr[dir].animation_images = handle_subdirectory_vec(path);
    if (!image_arr[dir].animation_images.empty()) {
        ImGui::OpenPopup("Drag_Drop_Folder");
        return true;
    }

    return false;
}

bool prep_extension(LF *F_Prop, user_info *usr_info, const char *file_name)
{
    snprintf(F_Prop->Opened_File, MAX_PATH, "%s", file_name);
    F_Prop->c_name = strrchr(F_Prop->Opened_File, PLATFORM_SLASH) + 1;
    if (!strrchr(F_Prop->Opened_File, '.')) {
        return false;
    }
    F_Prop->extension = strrchr(F_Prop->Opened_File, '.') + 1;

    // store filepaths in this directory for navigating through
    Next_Prev_File(F_Prop->Next_File,
                   F_Prop->Prev_File,
                   F_Prop->Frst_File,
                   F_Prop->Last_File,
                   F_Prop->Opened_File);

    if (usr_info != NULL)
    {
        std::filesystem::path file_path(F_Prop->Opened_File);
        snprintf(usr_info->default_load_path, MAX_PATH, "%s", file_path.parent_path().string().c_str());
    }
    // TODO: remove this printf 8==D
    printf("\nextension: %s\n", F_Prop->extension);
    return true;
}

void game_path_NOT_set_POPUP()
{
    if (ImGui::BeginPopupModal("Fallout2.exe Not Found")) {
        ImGui::Text(
            "Fallout2.exe not found, game path not set.\n"
            // "Fallout2.exe or Fallout2HR.exe not found at:\n"
            // "%s\n",
            // usr_nfo.

        );
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}


void game_path_set_POPUP(user_info* usr_nfo)
{
    if (ImGui::BeginPopupModal("Fallout2.exe Found")) {
        ImGui::Text(
            "Fallout2.exe found, new game path set to:\n"
            "%s\n",
            usr_nfo->default_game_path
        );
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void set_game_path_POPUP(user_info* usr_nfo)
{
    if (ifd::FileDialog::Instance().IsDone("Fallout2exe_path")) {
        if (ifd::FileDialog::Instance().HasResult()) {

            NATIVE_STRING_TYPE* fallout2_exe;
            NATIVE_STRING_TYPE* fallout2HR_exe;
#ifdef QFO2_WINDOWS
            fallout2_exe = L"fallout2.exe";
            fallout2HR_exe = L"fallout2HR.exe";
#elif defined(QFO2_LINUX)
            fallout2_exe = "fallout2.exe";
            fallout2HR_exe = "fallout2HR.exe";
#endif

            std::filesystem::path game_path = ifd::FileDialog::Instance().GetResult();
            std::filesystem::path filename = game_path.filename();

            if (io_strncasecmp(filename.c_str(), fallout2_exe, 13)
            &&  io_strncasecmp(filename.c_str(), fallout2HR_exe, 15)) {

                ImGui::OpenPopup("Fallout2.exe Not Found");
                ifd::FileDialog::Instance().Close();
                return;
            }

            if (std::filesystem::exists(game_path)) {
                strncpy(usr_nfo->default_game_path, game_path.parent_path().u8string().c_str(), MAX_PATH);
                ImGui::OpenPopup("Fallout2.exe Found");
            } else {
                ImGui::OpenPopup("Fallout2.exe Not Found");
            }
        }
        ifd::FileDialog::Instance().Close();
    }
}

//Ask user where the default Fallout 2 path is,
//then store path in both default_game_path and default_save_path if default_save_path is '\0'
//then write user_info out to config file
void Set_Default_Game_Path(user_info* usr_nfo, char* exe_path)
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

    //Set the default Fallout 2 game path
    char* file_path = usr_nfo->default_game_path;
    if (file_path[0] == '\0') {
        file_path = exe_path;
    }
    char path_buff[MAX_PATH];
    snprintf(path_buff, MAX_PATH, "%s", file_path);

    char* ptr = strrchr(path_buff, PLATFORM_SLASH);
    if (ptr) {
        ptr[0] = '\0';
    }

    ifd::FileDialog::Instance().Open(
        "Fallout2exe_path",
        "Open Folder",
        "Fallout2(*.exe;){.exe,.EXE,}",
        false, path_buff);
}

bool ImDialog_load_MSK(LF* F_Prop, image_data* img_data, user_info* usr_info, shader_info* shaders)
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

    static bool load_MSK;
    static char load_name[MAX_PATH];
    if (ImGui::Button("Load MSK to this slot")) {
        const char* ext_filter;
            ext_filter = "MSK files"
            "(*.msk;)"
            "{.msk,.MSK,}";

        char* folder = usr_info->default_load_path;
        ifd::FileDialog::Instance().Open("MSKLoadDialog", "Load File", ext_filter, folder);
    }

    if (ifd::FileDialog::Instance().IsDone("MSKLoadDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(load_name, temp.c_str(), temp.length()+1);
            strncpy(usr_info->default_load_path, temp.c_str(), temp.length()+1);
            char* ptr = strrchr(usr_info->default_load_path, PLATFORM_SLASH);
            *ptr = '\0';
            load_MSK = true;
        }
        ifd::FileDialog::Instance().Close();
    }
    if (strlen(load_name) < 1) {
        return false;
    }

    if (load_MSK) {
        char* ext = strrchr(load_name, '.')+1;
        if (io_strncmp(ext, "MSK", 4)) {  // 0 == match
            load_MSK = false;
            return false;
        }

        F_Prop->file_open_window = Load_MSK_Tile_SURFACE(load_name, img_data);
    }
    if (load_MSK) {
        F_Prop->edit_MSK = true;
        load_MSK = false;
        load_name[0] = '\0';
    }

    return true;
}


bool ImDialog_load_files(LF* F_Prop, image_data *img_data, user_info *usr_info, shader_info *shaders)
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

    static bool load_file;
    static char load_name[MAX_PATH];
    if (ImGui::Button("Load File")) {
        const char* ext_filter;
            ext_filter = "FRM/MSK and image files"
            "(*.png;"
            // "*.apng;"
            "*.jpg;*.jpeg;*.frm;*.fr0-5;*.msk;)"
            "{.fr0,.FR0,.fr1,.FR1,.fr2,.FR2,.fr3,.FR3,.fr4,.FR4,.fr5,.FR5,"
                ".png,.jpg,.jpeg,"
                ".frm,.FRM,"
                ".msk,.MSK,"
            "}";

        char* folder = usr_info->default_load_path;
        ifd::FileDialog::Instance().Open("FileLoadDialog", "Load File", ext_filter, folder);
    }

    if (ifd::FileDialog::Instance().IsDone("FileLoadDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string temp = ifd::FileDialog::Instance().GetResult().u8string();
            strncpy(load_name, temp.c_str(), temp.length()+1);
            strncpy(usr_info->default_load_path, temp.c_str(), temp.length()+1);
            char* ptr = strrchr(usr_info->default_load_path, PLATFORM_SLASH);
            *ptr = '\0';
            load_file = true;
        }
        ifd::FileDialog::Instance().Close();
    }

    bool success = false;
    if (load_file && strlen(load_name) > 1) {
        success = File_Type_Check(F_Prop, shaders, img_data, load_name);
    }

    if (success) {
        load_name[0] = '\0';
        success      = false;
        load_file    = false;
        return true;
    }


    return false;
}

//Check file extension to make sure it's one of the varieties of FRM
//TODO: maybe combine with Supported_Format()?
bool FRx_check(char *ext)
{
    if (
        (io_strncmp(ext, "FRM", 4) == 0)
     || (io_strncmp(ext, "FR0", 4) == 0)
     || (io_strncmp(ext, "FR1", 4) == 0)
     || (io_strncmp(ext, "FR2", 4) == 0)
     || (io_strncmp(ext, "FR3", 4) == 0)
     || (io_strncmp(ext, "FR4", 4) == 0)
     || (io_strncmp(ext, "FR5", 4) == 0))
    {
        return true;
    }
    return false;
}

//TODO: maybe combine with Supported_Format()?
bool File_Type_Check(LF *F_Prop, shader_info *shaders, image_data *img_data, const char *file_name)
{
    //TODO: make a function that checks if image has a different palette
    //      besides the default Fallout 1/2 palette
    //      also need to convert all float* palettes to Palette*
    bool success = prep_extension(F_Prop, NULL, file_name);
    if (!success) {
        return false;
    }
    img_data->display_frame_num = 0;
    // FRx_check checks extension to make sure it's one of the FRM variants (FRM, FR0, FR1...FR5)
    if (FRx_check(F_Prop->extension)) {
        // The new way to load FRM images using openGL
        F_Prop->file_open_window = load_FRM_OpenGL(F_Prop->Opened_File, img_data, shaders);
        if (F_Prop->file_open_window == false) {
            return false;
        }
        img_data->type = FRM;
    }
    else if (io_strncmp(F_Prop->extension, "MSK", 4) == 0) {  // 0 == match
        F_Prop->file_open_window = Load_MSK_Tile_SURFACE(F_Prop->Opened_File, img_data);
        if (F_Prop->file_open_window == false) {
            return false;
        }
        bool success   = false;
        img_data->type = MSK;
        success = framebuffer_init(&img_data->render_texture, &F_Prop->img_data.framebuffer, 350, 300);
        if (!success) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] Load_MSK_File_SURFACE\n\n"
                "Image framebuffer failed to attach correctly?"
            );
            printf("Image framebuffer failed to attach correctly?\n");
            return false;
        }
        SURFACE_to_texture(
            img_data->MSK_srfc,
            img_data->MSK_texture,
            350, 300, 1);
        draw_texture_to_framebuffer(
            shaders->FO_pal,
            shaders->render_FRM_shader,
            &shaders->giant_triangle,
            img_data->framebuffer,
            img_data->MSK_texture, 350, 300);
    }
    // TODO: add another type for known generic image types?
    else {  //all other more common (generic) image types
        Surface* temp_surface = nullptr;
        temp_surface = Load_File_to_RGBA(F_Prop->Opened_File);
        if (!temp_surface) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] File_Type_Check()\n\n"
                "Unable to load image."
            );
            printf("Unable to load image: %s\n", F_Prop->Opened_File);
            return false;
        }

        img_data->ANM_dir = (ANM_Dir*)malloc(sizeof(ANM_Dir) * 6);
        if (!img_data->ANM_dir) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] File_Type_Check()\n\n"
                "Unable to allocate memory for ANM_dir."
            );
            printf("Unable to allocate memory for ANM_dir: %d\n", __LINE__);
            return false;
        }
        //initialize allocated memory
        new (img_data->ANM_dir) ANM_Dir[6];

        img_data->ANM_dir[0].frame_data = (Surface**)malloc(sizeof(Surface*));
        if (!img_data->ANM_dir[0].frame_data) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] File_Type_Check()\n\n"
                "Unable to allocate memory for ANM_Frame."
            );
            printf("Unable to allocate memory for ANM_Frame: %d\n", __LINE__);
            free(img_data->ANM_dir);
            img_data->ANM_dir = NULL;
            return false;
        }
        new (img_data->ANM_dir->frame_data) ANM_Frame;

        Surface* srfc = img_data->ANM_dir[0].frame_data[0] = temp_surface;
        if (img_data->ANM_dir->frame_data) {
            img_data->width  = srfc->w;
            img_data->height = srfc->h;
            img_data->ANM_dir[0].num_frames = 1;

            img_data->type = OTHER;

            img_data->FRM_texture = init_texture(
                srfc,
                srfc->w,
                srfc->h,
                img_data->type);

            framebuffer_init(
                &img_data->render_texture,
                &img_data->framebuffer,
                srfc->w,
                srfc->h);

            //assign display direction to same as image slot
            //so we can see the image on load
            img_data->display_orient_num = NE;
            img_data->display_frame_num  = 0;
        }

        if (!img_data->ANM_dir[0].frame_box) {
            img_data->ANM_dir[0].frame_box = (rectangle*)calloc(1,sizeof(rectangle));
        }
        if (!img_data->ANM_dir[0].frame_box) {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] File_Type_Check()\n\n"
                "Unable to allocate memory for ANM_dir[0].frame_box."
            );
            printf("Unable to allocate memory for ANM_dir[0].frame_box: %d\n", __LINE__);
            free(img_data->ANM_dir);
            img_data->ANM_dir = NULL;
            return false;
        }
    }

    if (img_data->ANM_dir != NULL)
    {
        if ((img_data->ANM_dir[img_data->display_orient_num].frame_data == NULL)
            && img_data->type != FRM
            && img_data->type != MSK)
        {
            //TODO: log to file
            set_popup_warning(
                "[ERROR] File_Type_Check()\n\n"
                "Unable to open image file."
            );
            printf("Unable to open image file %s!\n", F_Prop->Opened_File);
            return false;
        }
    }
    // Set display window to open
    return true;
}

//TODO: delete? am I using load_tile_texture() anymore?
void load_tile_texture(GLuint *texture, char *file_name)
{
    Surface *surface = Load_File_to_RGBA(file_name);

    if (!glIsTexture(*texture))
    {
        glDeleteTextures(1, texture);
    }
    glGenTextures(1, texture);

    glBindTexture(GL_TEXTURE_2D, *texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Same

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pxls);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FreeSurface(surface);

    printf("glError: %d\n", glGetError());
}
