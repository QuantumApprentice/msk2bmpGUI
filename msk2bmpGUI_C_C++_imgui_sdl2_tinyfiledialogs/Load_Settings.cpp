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
	char key_buffer[256];
	char val_buffer[256];
} config_data;

void LoadFileData(struct user_info *user_info)
{
	char buffer[256];
	//uint8_t *fileData;
	char* file_data;

	FILE * config_file_ptr = NULL;
	fopen_s(&config_file_ptr, "config\\msk2bmpGUIa.cfg", "rt");

	if (!config_file_ptr) {
		fopen_s(&config_file_ptr, "config\\msk2bmpGUIa.cfg", "wt");
		//TODO add something to sprintf call to include the default save path somehow
		sprintf(buffer, "Default_Save_Path=");
		fwrite(buffer, strlen(buffer), 1, config_file_ptr);
		fclose(config_file_ptr);
	}
	else
	{
		size_t size;
		size = std::filesystem::file_size("config\\msk2bmpGUIa.cfg");

		file_data = (char*)malloc(size);
		fread(file_data, size, 1, config_file_ptr);
		file_data[size] = '\0';
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
	if (strncmp(config_data->key_buffer,"Default_Save_Path", sizeof(config_data->val_buffer)) == 0)
	{
		user_info->default_save_path = (char*)malloc(sizeof(config_data->val_buffer));
		strncpy(user_info->default_save_path, config_data->val_buffer, sizeof(config_data->val_buffer));
	}
}



//void parse_cell(char *file_data, size_t size, int *char_ptr)
//{
//	int length;
//	while (*char_ptr < size)
//	{
//		switch (file_data[*char_ptr])
//		{
//		case ',': case '\r': case '\n': 
//			{
//				return;
//			}
//		default:
//			{
//				length++;
//				*char_ptr++;
//				break;
//			}
//		}
//	}
//}
//void parse_key(char *file_data, size_t size, int *char_ptr)
//{
//	int length;
//	while (*char_ptr < size)
//	{
//		switch (file_data[*char_ptr])
//		{
//		case ',': case '\r': case '\n':
//			{
//				return;
//			}
//		case '=':
//			{
//				(*char_ptr)++;
//			}
//		default:
//			{
//				length++;
//				break;
//			}
//		}
//	}
//}
//#include "imgui-docking/imgui_internal.h"
//void Settings::Init()
//{
//	ImGuiSettingsHandler iniHandler;
//	iniHandler.TypeName = "UserSettings";
//	iniHandler.TypeHash = ImHashStr(iniHandler.TypeName);
//	iniHandler.ClearAllFn = ClearAllHandler;
//	iniHandler.ReadOpenFn = ReadOpenHandler;
//	iniHandler.ReadLineFn = ReadLineHandler;
//	iniHandler.ApplyAllFn = ApplyAllHandler;
//	iniHandler.WriteAllFn = WriteAllHandler;
//	ImGui::GetCurrentContext()->SettingsHandlers.push_back(iniHandler);
//}
//void Settings::ClearAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler* settings)
//{
//}
//void* Settings::ReadOpenHandler(ImGuiContext* ctx, ImGuiSettingsHandler* hndlr, const char* name)
//{
//	return NULL;
//}
//void Settings::ReadLineHandler(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
//{
//	"this is a test string";
//}
//void Settings::ApplyAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler*)
//{
//}
//void Settings::WriteAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
//{
//}
