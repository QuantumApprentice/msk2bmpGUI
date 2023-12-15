#include "Load_Settings.h"
#include "dependencies/tinyfiledialogs/tinyfiledialogs.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <filesystem>
//#include <sys/stat.h>

char * folder_name;

struct config_data {
    int char_ptr = 0;
    int column;
    //regular char buffers
    char key_buffer[MAX_KEY];
    char val_buffer[MAX_PATH];
} config_data_struct;

//TODO: config file needs to be stored in a writeable directory
//      depending on O/S the exe directory may not be writeable
// hybrid approach - have it support a portable version and an installed version.
// In the case of the portable version, you ship a configuration file with it
// that enables portable mode, then when you start the app you read the config
// file in the same directory as the exe

// In the case of an installed version you don't have that configuration file,
// so it uses system paths for storing the configuration files
// Not necessarily same directory, just same file hierarchy somewhere,
// it can be in a subdirectory

// Or you can make it support loading configuration from multiple places
// depending on which one it finds first

// So when you load the program it tries to find the config file in a few places,
// and marks the one where it found it as the configuration path,
// and when you save it tries to write there

// Then it uses the "user-level" directory if it finds none,
// since in the "portable" version you'd have a configuration
// file somewhere near the exe that it should find
// environment variables on Linux as well. Like XDG_DESKTOP_SOMETHING

void Load_Config(struct user_info *usr_info, char* exe_path)
{
    //regular char
    //char buffer[MAX_PATH];
    char* file_data;

    char path_buffer[MAX_PATH];
    snprintf(path_buffer, sizeof(path_buffer), "%s%s", exe_path, "config/msk2bmpGUI.cfg");

    //TODO: Need to be able to check if directory exists,
    //      and create it if it doesn't
    FILE * config_file_ptr = NULL;

#ifdef QFO2_WINDOWS
    errno_t error = _wfopen_s(&config_file_ptr, tinyfd_utf8to16(path_buffer), L"rb");
#elif defined(QFO2_LINUX)
    config_file_ptr = fopen(path_buffer, "rb");
#endif

    if (!config_file_ptr) {

#ifdef QFO2_WINDOWS
            printf("error, can't open config file to read, error: %d", err);
        if (err == 13) {
            printf("file opened by another program?");
        }
#elif defined(QFO2_LINUX)
        //TODO: change to warning, possibly remove since file is created anyway
        printf("error, can't open config file to read, error: %d\n%s\n", errno, strerror(errno));
#endif

    //// C version of the code using <sys/stats.h>
        ////check if directory exists
        //struct stat stats;
        //stat("config\\", &stats);
        //if (stats.st_mode & S_IFDIR) {
        //    write_cfg_file(user_info);
        //}
        //else {
        //    mkdir();
        //    std::filesystem::create_directory("config\\");
        //    write_cfg_file(user_info);
        //}

        std::filesystem::path p(path_buffer);
        if (std::filesystem::is_directory(p.parent_path())) {
            write_cfg_file(usr_info, exe_path);
  
        }
        else {
            std::filesystem::create_directory(std::filesystem::path(exe_path) / "config");
            write_cfg_file(usr_info, exe_path);
        }
    }
    else
    {
        size_t file_size = std::filesystem::file_size(path_buffer);

        file_data = (char*)malloc(file_size + 1);
        fread(file_data, file_size, 1, config_file_ptr);
        file_data[file_size] = '\0';
        fclose(config_file_ptr);

        parse_data(file_data, file_size, usr_info);
    }
}

//normal char version
//Parse the msk2bmpGUI.cfg file
//stores config settings into global user_info struct
void parse_data(char * file_data, size_t size, struct user_info *usr_info)
{
    //config_data_struct is global here (maybe instance here instead?)
    while (config_data_struct.char_ptr < size)
    {
        switch (file_data[config_data_struct.char_ptr])
        {
        case '\r': case '\n':
        {
            config_data_struct.char_ptr++;
            parse_key(file_data, size, &config_data_struct);
            break;
        }
        case ';':
        {
            parse_comment(file_data, size, &config_data_struct);
            break;
        }
        case '=':
        {
            parse_value(file_data, size, &config_data_struct, usr_info);
            break;
        }
        default:
        {
            parse_key(file_data, size, &config_data_struct);
            break;
        }
        }
    }
}
void parse_key(char *file_data, size_t size, struct config_data *config)
{
    int i = 0;
    while (config->char_ptr < size)
    {
        switch (file_data[config->char_ptr])
        {
        case '\r': case '\n': case '=':
        {
            config->key_buffer[i++] = '\0';
            return;
        }
        }
        config->key_buffer[i++] = file_data[config->char_ptr++];
    }
}
void parse_comment(char *file_data, size_t size, struct config_data *config)
{
    while (config->char_ptr < size)
    {
        switch (file_data[config->char_ptr])
        {
        case '\r': case '\n':
        {
            return;
        }
        }
        config->char_ptr++;
    }
}
void parse_value(char *file_data, size_t size, struct config_data *config, struct user_info *usr_info)
{
    int i = 0;
    config->char_ptr++;
    while (config->char_ptr < size)
    {
        switch (file_data[config->char_ptr])
        {
        case '\r': case '\n':
        {
            config->val_buffer[i++] = '\0';
            store_config_info(config, usr_info);
            return;
        }
        case ';':
        { return; }
        }
        config->val_buffer[i++] = file_data[config->char_ptr++];
    }
    config->val_buffer[i++] = '\0';
    store_config_info(config, usr_info);
}


////wide character version
//void parse_data(wchar_t* file_data, size_t size, struct user_info *user_info)
//{
//    while (config_data.char_ptr < size)
//    {
//        switch (file_data[config_data.char_ptr])
//        {
//        case '\r': case '\n':
//        {
//            config_data.char_ptr++;
//            parse_key(file_data, size, &config_data);
//            break;
//        }
//        case ';':
//        {
//            parse_comment(file_data, size, &config_data);
//            break;
//        }
//        case '=':
//        {
//            parse_value(file_data, size, &config_data, user_info);
//            break;
//        }
//        default:
//        {
//            parse_key(file_data, size, &config_data);
//            break;
//        }
//        }
//    }
//}
//
//void parse_key(wchar_t *file_data, size_t size, struct config_data *config_data)
//{
//    int i = 0;
//    while (config_data->char_ptr < size)
//    {
//        switch (file_data[config_data->char_ptr])
//        {
//        case '\r': case '\n': case '=':
//        {
//            return;
//        }
//        }
//        config_data->key_buffer[i++] = file_data[config_data->char_ptr++];
//    }
//}
//
//void parse_comment(wchar_t *file_data, size_t size, struct config_data *config_data)
//{
//    while (config_data->char_ptr < size)
//    {
//        switch (file_data[config_data->char_ptr])
//        {
//        case '\r': case '\n':
//        {
//        	return;
//        }
//        }
//        config_data->char_ptr++;
//    }
//}
//void parse_value(wchar_t *file_data, size_t size, struct config_data *config_data, struct user_info *user_info)
//{
//    int i = 0;
//    config_data->char_ptr++;
//    while (config_data->char_ptr < size)
//    {
//        switch (file_data[config_data->char_ptr])
//        {
//        case '\r': case '\n':
//        {
//        	store_config_info(config_data, user_info);
//        	return;
//        }
//        case ';':
//        { return; }
//        }
//        config_data->val_buffer[i++] = file_data[config_data->char_ptr++];
//    }
//    store_config_info(config_data, user_info);
//}

void store_config_info(struct config_data *config, struct user_info *usr_info)
{
    if (strncmp(config->key_buffer, "Default_Save_Path", sizeof(config->val_buffer)) == 0)
    {
        snprintf(usr_info->default_save_path, sizeof(usr_info->default_save_path), "%s", config->val_buffer);
    }
    if (strncmp(config->key_buffer, "Default_Game_Path", sizeof(config->val_buffer)) == 0)
    {
        snprintf(usr_info->default_game_path, sizeof(usr_info->default_save_path), "%s", config->val_buffer);
    }
    if (strncmp(config->key_buffer, "Default_Load_Path", sizeof(config->val_buffer)) == 0)
    {
        snprintf(usr_info->default_load_path, sizeof(usr_info->default_save_path), "%s", config->val_buffer);
    }
    if (strncmp(config->key_buffer, "Save_Full_MSK_Warn", sizeof(config->val_buffer)) == 0)
    {   //handle boolean
        usr_info->save_full_MSK_warning = (config->val_buffer[0] == '1');
    }
    if (strncmp(config->key_buffer, "Show_Image_Stats", sizeof(config->val_buffer)) == 0)
    {   //handle boolean
        usr_info->show_image_stats = (config->val_buffer[0] == '1');
    }
}

//TODO: change to add all lines to a buffer then fwrite entire buffer
//TODO: add new entries that are already in user_cfg
void write_cfg_file(struct user_info* usr_info, char* exe_path)
{
    char path_buffer[MAX_PATH];
    snprintf(path_buffer, sizeof(path_buffer), "%s%s", exe_path, "config/msk2bmpGUI.cfg");

    FILE * config_file_ptr = NULL;
    //fopen_s(&config_file_ptr, "config\\msk2bmpGUI.cfg", "wt");

#ifdef QFO2_WINDOWS
    errno_t err = _wfopen_s(&config_file_ptr, tinyfd_utf8to16(path_buffer), L"wb");
#elif defined(QFO2_LINUX)
    config_file_ptr = fopen(path_buffer, "wb");
#endif

    if (config_file_ptr == NULL) {

#ifdef QFO2_WINDOWS
        printf("error, can't open config file to write, error: %d", err);
        if (err == 13) {
            printf("file opened by another program?");
        }
#elif defined(QFO2_LINUX)
        printf("error, can't open config file to write, error: %d\n%s", errno, strerror(errno));
#endif

        return;
    }

    fwrite("Default_Save_Path=",     strlen("Default_Save_Path="  ), 1, config_file_ptr);
    fwrite(usr_info->default_save_path, strlen(usr_info->default_save_path), 1, config_file_ptr);
    fwrite("\r\nDefault_Game_Path=", strlen("\r\nDefault_Game_Path="), 1, config_file_ptr);
    fwrite(usr_info->default_game_path, strlen(usr_info->default_game_path), 1, config_file_ptr);
    fwrite("\r\nDefault_Load_Path=", strlen("\r\nDefault_Load_Path="), 1, config_file_ptr);
    fwrite(usr_info->default_load_path, strlen(usr_info->default_load_path), 1, config_file_ptr);

    //TODO: might need to write a boolean handler?
    fwrite("\r\nSave_Full_MSK_Warn=", strlen("\r\nSave_Full_MSK_Warn="), 1, config_file_ptr);
    char buffer[2];
    snprintf(buffer, 2, "%d", usr_info->save_full_MSK_warning);
    fwrite(buffer, strlen(buffer), 1, config_file_ptr);

    fwrite("\r\nShow_Image_Stats=", strlen("\r\nShow_Image_Stats="), 1, config_file_ptr);
    snprintf(buffer, 2, "%d", usr_info->show_image_stats);
    fwrite(buffer, strlen(buffer), 1, config_file_ptr);



    fclose(config_file_ptr);
}