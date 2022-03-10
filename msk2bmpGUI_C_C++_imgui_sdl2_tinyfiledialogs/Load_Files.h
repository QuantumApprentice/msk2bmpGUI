#pragma once
#include <SDL.h>
struct LF {
	char * Opened_File;
	char * c_name;
	SDL_Surface* image;

	bool file_open_window;
	char * FilterPattern1[2] = { "*.bmp" , "*.png" };
};
void Load_Files(LF[], int);