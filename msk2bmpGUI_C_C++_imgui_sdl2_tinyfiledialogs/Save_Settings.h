#pragma once
#include "imgui-docking/imgui.h"

struct ImGuiSettingsHandler;

struct Settings
{
	static void Init();

	static void ClearAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler*);
	static void* ReadOpenHandler(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
	static void ReadLineHandler(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
	static void ApplyAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler*);
	static void WriteAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);
};