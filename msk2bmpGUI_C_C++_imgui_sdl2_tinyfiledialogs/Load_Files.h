#pragma once
#include <SDL.h>
//#include <SDL_opengl.h>
#include <glad/glad.h>

struct LF {
	char Opened_File[256];
	char * c_name;
	char * extension;
	SDL_Surface* image = nullptr;
	SDL_Surface* Final_Render = nullptr;
	//SDL_Texture* Optimized_Texture = nullptr;
	GLuint Optimized_Texture = 0;
	GLuint Optimized_Render_Texture = 0;

	int texture_width = 0, texture_height = 0;

	bool file_open_window;
	bool preview_tiles_window;
	char * FilterPattern1[3] = { "*.bmp" , "*.png", "*.frm" };
};
void Load_Files(LF[], int);
void FRM_Load(LF*);
