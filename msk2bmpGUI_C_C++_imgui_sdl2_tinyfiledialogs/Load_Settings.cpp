#include "Load_Settings.h"

#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <filesystem>

char * folder_name;
struct config_data {
	int char_ptr = 0;
	int column;
	char key_buffer[_MAX_PATH];
	char val_buffer[_MAX_PATH];
} config_data;

void Load_Config(struct user_info *user_info)
{
	char buffer[_MAX_PATH];
	//uint8_t *fileData;
	char* file_data;

	FILE * config_file_ptr = NULL;
	fopen_s(&config_file_ptr, "config\\msk2bmpGUI.cfg", "rt");

	if (!config_file_ptr) {
        write_cfg_file(user_info);
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
			store_config_info(config_data, user_info);
			return;
		}
		case ';':
		{ return; }
		}
		config_data->val_buffer[i++] = file_data[config_data->char_ptr++];
	}
	store_config_info(config_data, user_info);
}

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
}

void write_cfg_file(struct user_info* user_info)
{
    FILE * config_file_ptr = NULL;
    fopen_s(&config_file_ptr, "config\\msk2bmpGUI.cfg", "wt");
    fwrite("Default_Save_Path=", strlen("Default_Save_Path="), 1, config_file_ptr);
    fwrite(user_info->default_save_path, strlen(user_info->default_save_path), 1, config_file_ptr);
    fwrite("\nDefault_Game_Path=", strlen("\nDefault_Game_Path="), 1, config_file_ptr);
    fwrite(user_info->default_game_path, strlen(user_info->default_game_path), 1, config_file_ptr);
    fclose(config_file_ptr);
}