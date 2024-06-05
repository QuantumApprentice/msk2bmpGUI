// The built-in Dear ImGUI OpenGL loader was getting compile errors and we're already using GLAD
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include <glad/glad.h>

// Dear ImGUI docking branch from revision 1d8e48c161370c37628c4f37f3f87cb19fbcb723 (v1.89.9-docking tag)
#include "dependencies/imgui-docking/src/imgui.cpp"
#include "dependencies/imgui-docking/src/imgui_demo.cpp"
#include "dependencies/imgui-docking/src/imgui_draw.cpp"
#include "dependencies/imgui-docking/src/imgui_impl_glfw.cpp"
#include "dependencies/imgui-docking/src/imgui_impl_opengl3.cpp"
#include "dependencies/imgui-docking/src/imgui_tables.cpp"
#include "dependencies/imgui-docking/src/imgui_widgets.cpp"
