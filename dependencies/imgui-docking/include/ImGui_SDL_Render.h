#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

struct SDL_Renderer;

IMGUI_IMPL_API bool     ImGui_ImplSDLRenderer_Init(SDL_Renderer* renderer);
IMGUI_IMPL_API void     ImGui_ImplSDLRenderer_RenderDrawData(ImDrawData* draw_data);
