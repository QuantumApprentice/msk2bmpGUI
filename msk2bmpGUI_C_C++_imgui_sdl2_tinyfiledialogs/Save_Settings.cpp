#include "Save_Settings.h"
#include "imgui-docking/imgui_internal.h"
#include <map>

void Settings::Init()
{
	ImGuiSettingsHandler iniHandler;
	iniHandler.TypeName = "UserSettings";
	iniHandler.TypeHash = ImHashStr(iniHandler.TypeName);
	iniHandler.ClearAllFn = ClearAllHandler;
	iniHandler.ReadOpenFn = ReadOpenHandler;
	iniHandler.ReadLineFn = ReadLineHandler;
	iniHandler.ApplyAllFn = ApplyAllHandler;
	iniHandler.WriteAllFn = WriteAllHandler;
	ImGui::GetCurrentContext()->SettingsHandlers.push_back(iniHandler);
}

void Settings::ClearAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler* settings)
{
}

void* Settings::ReadOpenHandler(ImGuiContext* ctx, ImGuiSettingsHandler* hndlr, const char* name)
{
	return NULL;
}
void Settings::ReadLineHandler(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
	"this is a test string";
}
void Settings::ApplyAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler*)
{
}
void Settings::WriteAllHandler(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{
}
