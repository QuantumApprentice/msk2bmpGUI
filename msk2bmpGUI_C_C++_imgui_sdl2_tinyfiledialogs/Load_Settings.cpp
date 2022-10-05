#include "Load_Settings.h"

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

} config_data;

void Load_Config(struct user_info *user_info)
{
    //regular char
    char buffer[MAX_PATH];
    char* file_data;

    //TODO: Need to be able to check if directory exists,
    //      and create it if it doesn't
    FILE * config_file_ptr = NULL;
    errno_t error =
    _wfopen_s(&config_file_ptr, L"config\\msk2bmpGUI.cfg", L"rb");

    if (!config_file_ptr) {
        //check if directory exists
        
    //// C version of the code using <sys/stats.h>
        //struct stat stats;
        //stat("config\\", &stats);
        //if (stats.st_mode & S_IFDIR) {
        //    write_cfg_file(user_info);
        //}
        //else {
        //    std::filesystem::create_directory("config\\");
        //    write_cfg_file(user_info);
        //}

        std::filesystem::path p("config\\");
        if (std::filesystem::is_directory(p)) {
            write_cfg_file(user_info);
  
        }
        else {
            std::filesystem::create_directory("config\\");
            write_cfg_file(user_info);
        }
    }
    else
    {
        size_t size;
        size = std::filesystem::file_size("config\\msk2bmpGUI.cfg");

        file_data = (char*)malloc(size);
        fread(file_data, size, 1, config_file_ptr);
        file_data[size-1] = '\0';
        fclose(config_file_ptr);

        parse_data(file_data, size, user_info);
    }
}

//normal char version
void parse_data(char * file_data, size_t size, struct user_info *user_info)
{
    while (config_data.char_ptr < size)
    {
        switch (file_data[config_data.char_ptr])
        {
        case '\r': case '\n':
        {
            config_data.char_ptr++;
            parse_key(file_data, size, &config_data);
            break;
        }
        case ';':
        {
            parse_comment(file_data, size, &config_data);
            break;
        }
        case '=':
        {
            parse_value(file_data, size, &config_data, user_info);
            break;
        }
        default:
        {
            parse_key(file_data, size, &config_data);
            break;
        }
        }
    }
}
void parse_key(char *file_data, size_t size, struct config_data *config_data)
{
    int i = 0;
    while (config_data->char_ptr < size)
    {
        switch (file_data[config_data->char_ptr])
        {
        case '\r': case '\n': case '=':
        {
            config_data->key_buffer[i++] = '\0';
            return;
        }
        }
        config_data->key_buffer[i++] = file_data[config_data->char_ptr++];
    }
}
void parse_comment(char *file_data, size_t size, struct config_data *config_data)
{
    while (config_data->char_ptr < size)
    {
        switch (file_data[config_data->char_ptr])
        {
        case '\r': case '\n':
        {
            return;
        }
        }
        config_data->char_ptr++;
    }
}
void parse_value(char *file_data, size_t size, struct config_data *config_data, struct user_info *user_info)
{
    int i = 0;
    config_data->char_ptr++;
    while (config_data->char_ptr < size)
    {
        switch (file_data[config_data->char_ptr])
        {
        case '\r': case '\n':
        {
            config_data->val_buffer[i++] = '\0';
            store_config_info(config_data, user_info);
            return;
        }
        case ';':
        { return; }
        }
        config_data->val_buffer[i++] = file_data[config_data->char_ptr++];
    }
    config_data->val_buffer[i++] = '\0';
    store_config_info(config_data, user_info);
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

void store_config_info(struct config_data *config_data, struct user_info *user_info)
{
    if (strncmp(config_data->key_buffer, "Default_Save_Path", sizeof(config_data->val_buffer)) == 0)
    {
        strncpy(user_info->default_save_path, config_data->val_buffer, sizeof(config_data->val_buffer));
    }
    if (strncmp(config_data->key_buffer, "Default_Game_Path", sizeof(config_data->val_buffer)) == 0)
    {
        strncpy(user_info->default_game_path, config_data->val_buffer, sizeof(config_data->val_buffer));
    }
    if (strncmp(config_data->key_buffer, "Default_Load_Path", sizeof(config_data->val_buffer)) == 0)
    {
        strncpy(user_info->default_load_path, config_data->val_buffer, sizeof(config_data->val_buffer));
    }
}

void write_cfg_file(struct user_info* user_info)
{
    FILE * config_file_ptr = NULL;
    //fopen_s(&config_file_ptr, "config\\msk2bmpGUI.cfg", "wt");
    _wfopen_s(&config_file_ptr, L"config\\msk2bmpGUI.cfg", L"wb");

    fwrite("Default_Save_Path=",   strlen("Default_Save_Path="  ), 1, config_file_ptr);
    fwrite(user_info->default_save_path, strlen(user_info->default_save_path), 1, config_file_ptr);
    fwrite("\r\nDefault_Game_Path=", strlen("\r\nDefault_Game_Path="), 1, config_file_ptr);
    fwrite(user_info->default_game_path, strlen(user_info->default_game_path), 1, config_file_ptr);
    fwrite("\r\nDefault_Load_Path=", strlen("\r\nDefault_Load_Path="), 1, config_file_ptr);
    fwrite(user_info->default_load_path, strlen(user_info->default_load_path), 1, config_file_ptr);

    fclose(config_file_ptr);
}