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

} config;

void Load_Config(struct user_info *usr_info)
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
        //    mkdir();
        //    std::filesystem::create_directory("config\\");
        //    write_cfg_file(user_info);
        //}

        std::filesystem::path p("config\\");
        if (std::filesystem::is_directory(p)) {
            write_cfg_file(usr_info);
  
        }
        else {
            std::filesystem::create_directory("config\\");
            write_cfg_file(usr_info);
        }
    }
    else
    {
        size_t file_size = std::filesystem::file_size("config\\msk2bmpGUI.cfg");

        file_data = (char*)malloc(file_size + 1);
        fread(file_data, file_size, 1, config_file_ptr);
        file_data[file_size] = '\0';
        fclose(config_file_ptr);

        parse_data(file_data, file_size, usr_info);
    }
}

//normal char version
void parse_data(char * file_data, size_t size, struct user_info *usr_info)
{
    while (config.char_ptr < size)
    {
        switch (file_data[config.char_ptr])
        {
        case '\r': case '\n':
        {
            config.char_ptr++;
            parse_key(file_data, size, &config);
            break;
        }
        case ';':
        {
            parse_comment(file_data, size, &config);
            break;
        }
        case '=':
        {
            parse_value(file_data, size, &config, usr_info);
            break;
        }
        default:
        {
            parse_key(file_data, size, &config);
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
void write_cfg_file(struct user_info* usr_info)
{
    FILE * config_file_ptr = NULL;
    //fopen_s(&config_file_ptr, "config\\msk2bmpGUI.cfg", "wt");
    _wfopen_s(&config_file_ptr, L"config\\msk2bmpGUI.cfg", L"wb");

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