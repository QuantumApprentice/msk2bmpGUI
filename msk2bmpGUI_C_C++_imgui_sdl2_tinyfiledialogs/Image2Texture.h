#pragma once
#include "imgui-docking/imgui.h"
#include "Load_Files.h"

struct variables {
	ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
	ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
	ImVec2 uv_max = ImVec2(1.0f, 1.0f);
	bool Render_Tiles = false;
	bool Preview_Tiles = false;
	bool Render_Window = false;
	int Render_Width = 0, Render_Height = 0;

	SDL_Color *PaletteColors = nullptr;

	SDL_Surface* Temp_Surface = nullptr;

	struct LF F_Prop[99]{};
};

//void Image2Texture(variables* My_Variables, int counter);
void Image2Texture(SDL_Surface* surface, GLuint* texture, bool* window);
void SDL_to_OpenGl(SDL_Surface* Temp_Surface, GLuint *Optimized_Texture);