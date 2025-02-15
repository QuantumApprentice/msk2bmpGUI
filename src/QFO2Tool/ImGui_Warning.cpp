#include "imgui.h"
#include "ImGui_Warning.h"
#include "stdio.h"

static char warn_str[2048];
// static char warn_name[2048];

void set_popup_warning(const char* str)//, const char* name)
{
    // strncpy(warn_name, name, 2048);
    strncpy(warn_str,  str,  2048);
    ImGui::OpenPopup("Warning");
}

void popup_warnings()//const char* modal_name, const char* str)
{
    if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_MenuBar))
    {
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        ImGui::Text(warn_str);

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}