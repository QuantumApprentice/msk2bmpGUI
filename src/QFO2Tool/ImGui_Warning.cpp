#include "imgui.h"
#include "ImGui_Warning.h"

// NOTE:
// ImGui's popup system seems to be based on the current
//  open window. If a popup is opened it needs to be
//  called between ImGui::Begin() and ImGui::End() for
//  each individual window, separately.
// This is, of course, pretty annoying.
//  What's even more annoying is that even though
//  one window is opened inside another window,
//  the popup only opens inside the immediate window,
//  and won't open a global one (afaik) located in the
//  root most window.
// Whatever...this works well enough for now,
//  and allows me to easily give the player feedback
//  without crashing.
//  (at least it does if I keep track of errors correctly)

static char warn_str[2048];
// static char warn_name[2048];

void set_popup_warning(const char* str)//, const char* name)
{
    // strncpy(warn_name, name, 2048);
    strncpy(warn_str,  str,  2048);
    ImGui::OpenPopup("Warning");
    // Always center this window when appearing
    //  hopefully the next window opened is the popup one
    // ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 center = ImGui::GetWindowViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
}

void show_popup_warnings()//const char* modal_name, const char* str)
{
    if (ImGui::BeginPopupModal("Warning", NULL, ImGuiWindowFlags_MenuBar))
    {
        ImGui::Text(warn_str);

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}