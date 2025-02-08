#pragma once
// #include <SDL.h>
#include "Image2Texture.h"


void Edit_Image(variables* My_Variables, image_data* edit_data, Edit_Surface* edit_srfc, Surface* edit_MSK_srfc, bool edit_MSK, bool Palette_Update, uint8_t* Color_Pick);
//void Edit_MSK_SDL(LF* F_Prop, bool* Palette_Update, ImVec2 Origin);

bool Create_MSK_OpenGL(image_data* img_data);

//SDL_Surface* Create_MSK_SDL(SDL_Surface* image, GLuint* texture, bool* window);
//void CPU_Blend_SDL(SDL_Surface* surface1, SDL_Surface* surface2);
//void Update_Palette_SDL(struct LF* files, bool blend);
//void Update_Palette2(SDL_Surface* surface, GLuint* texture, SDL_PixelFormat* pxlFMT);

//void texture_paint(int x, int y, int brush_w, int brush_h, int value, unsigned int texture);
void texture_paint(variables* My_Variables, image_data* edit_data, Surface* edit_srfc, bool edit_MSK);
void brush_size_handler(variables* My_Variables);
void display_img_ImGUI(variables* My_Variables, image_data* edit_data);

