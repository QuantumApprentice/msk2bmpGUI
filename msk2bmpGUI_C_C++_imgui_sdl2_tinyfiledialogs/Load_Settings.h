#pragma once
//#include "imgui-docking/imgui.h"

//struct ImGuiSettingsHandler;
//
//struct Settings
//{
//	static void Init();
//
//	static void ClearAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler*);
//	static void* ReadOpenHandler(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
//	static void ReadLineHandler(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
//	static void ApplyAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler*);
//	static void WriteAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);
//};
struct user_info {
	char * default_save_path;
	size_t length;
};

void LoadFileData(struct user_info *user_info);

void parse_data(char *file_data, size_t size, struct user_info *user_info);
void parse_key(char *file_data, size_t size, struct config_data *config_data);
void parse_comment(char *file_data, size_t size, struct config_data *config_data);
void parse_value(char *file_data, size_t size, struct config_data *config_data, struct user_info *user_info);
void store_config_info(struct config_data *config_data, struct user_info *user_info);