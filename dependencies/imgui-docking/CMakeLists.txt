cmake_minimum_required(VERSION 3.27)

project(
    "imgui_docking_for_fallout" VERSION 0.0.1
    DESCRIPTION "imgui_docking_for_fallout"
)

add_library(imgui_docking STATIC)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

target_link_libraries( imgui_docking PUBLIC
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}
)

target_sources(
    imgui_docking PRIVATE

    include/imconfig.h
    include/imgui.h
    include/imgui_impl_opengl3.h
    include/imgui_impl_opengl3_loader.h
    include/imgui_impl_sdl.h
    include/imgui_impl_sdlrenderer.h
    include/imgui_internal.h
    include/imstb_rectpack.h
    include/imstb_textedit.h
    include/imstb_truetype.h

    # include/ImGui_SDL_Render.h

    src/imgui.cpp
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_impl_sdl.cpp
    src/imgui_impl_sdlrenderer.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp

    # src/ImGui_SDL_Render.cpp
)

target_include_directories(
    imgui_docking
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# message("________-------"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
