#pragma once
#include <filesystem>

#define MAX_PATH 1024
#define MAX_KEY  32

struct user_info {
    char default_save_path[MAX_PATH];
    char default_game_path[MAX_PATH];
    char default_load_path[MAX_PATH];
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
    OTHER =  4
};

void Load_Config    (struct user_info *user_info, char* exe_path);
void write_cfg_file (struct user_info* user_info, char* exe_path);

void parse_data     (char *file_data, size_t size, struct user_info *user_info);
void parse_key      (char *file_data, size_t size, struct config_data *config_data);
void parse_comment  (char *file_data, size_t size, struct config_data *config_data);
void parse_value    (char *file_data, size_t size, struct config_data *config_data, struct user_info *user_info);
void store_config_info(struct config_data *config_data, struct user_info *user_info);