#pragma once
#include <filesystem>

struct user_info {
	char default_save_path[_MAX_PATH];
    char default_game_path[_MAX_PATH];
    char default_load_path[_MAX_PATH];
	size_t length;
};

void Load_Config(struct user_info *user_info);
void write_cfg_file(struct user_info* user_info);

void parse_data(char *file_data, size_t size, struct user_info *user_info);
void parse_key(char *file_data, size_t size, struct config_data *config_data);
void parse_comment(char *file_data, size_t size, struct config_data *config_data);
void parse_value(char *file_data, size_t size, struct config_data *config_data, struct user_info *user_info);
void store_config_info(struct config_data *config_data, struct user_info *user_info);