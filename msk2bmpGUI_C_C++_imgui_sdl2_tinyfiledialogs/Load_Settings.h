#pragma once
#include <filesystem>

#ifdef QFO2_WINDOWS
    #define MAX_PATH 256
    //TODO: add error messages if user goes over MAX_PATH
#elif defined(QFO2_LINUX)
    #define MAX_PATH 4096
    #include <string.h>
#endif
#define MAX_KEY  32

enum export_auto {
    not_set  = 0,
    auto_all = 1,
    manual   = 2,
};

struct fo2_files {
    char* TILES_LST;
    char* WORLDMAP_TXT;
};

struct user_info {
    char default_save_path[MAX_PATH];
    char default_game_path[MAX_PATH];
    char default_load_path[MAX_PATH];
    char* exe_directory = NULL;

    fo2_files game_files;

    int  auto_export = not_set;           // 0=not set, 1=auto all the way, 2=manual
    bool save_full_MSK_warning;
    bool show_image_stats;
    size_t length;
};

enum img_type {
    UNK   = -1,
    MSK   =  0,
    FRM   =  1,
    FR0   =  2,
    FRx   =  3,
    TILE  =  3,
    OTHER =  4,
};

void Load_Config    (struct user_info *user_info, char* exe_path);
void write_cfg_file (struct user_info* user_info, char* exe_path);

void parse_data     (char *file_data, size_t size, struct user_info *user_info);
void parse_key      (char *file_data, size_t size, struct config_data *config_data);
void parse_comment  (char *file_data, size_t size, struct config_data *config_data);
void parse_value    (char *file_data, size_t size, struct config_data *config_data, struct user_info *user_info);
void store_config_info(struct config_data *config_data, struct user_info *user_info);