#pragma once
#include <SDL.h>
struct LF {
	char * Opened_File;
	char * c_name;
	SDL_Surface* image1;

	bool file_open_window[1][1];
	char * FilterPattern1[2] = { "*.bmp" , "*.png" };
};
void Load_Files(LF&);