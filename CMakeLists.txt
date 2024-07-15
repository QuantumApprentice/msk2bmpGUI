cmake_minimum_required(VERSION 3.25)

# # Enable ASAN globally
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")


#compile option (-mavx) for is_tile_blank() in tiles_patterns.cpp
#applies to current directory & below?
add_compile_options(
  -mavx             #support for AVX instructions
  # -fsanitize=address
)


#sets PROJECT_NAME
#PROJECT_NAME is auto set when name is passed as 1st arg
#https://cmake.org/cmake/help/latest/command/project.html#project
project(
  QFO2Solution VERSION 1.2.1
  # ${PROJECT_NAME} VERSION 0.1.0
  # DESCRIPTION "${PROJECT_NAME}"
)
set(EXECUTABLE_NAME "QFO2Tool")
add_executable(${EXECUTABLE_NAME})


# Windows compile stuff
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_compile_definitions(
  ${EXECUTABLE_NAME}
  PUBLIC QFO2_WINDOWS
)

# Linux compile stuff
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
target_compile_definitions(
  ${EXECUTABLE_NAME}
  PUBLIC QFO2_LINUX
)
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS FALSE)

target_sources(
  ${EXECUTABLE_NAME} PRIVATE
  src/msk2bmpGUI.cpp
)

#these are directories that have CMakeLists.txt files
#they automatically compile into libraries
#just have to figure out how to hook them in
add_subdirectory(src/dependencies/GLAD            SYSTEM)
add_subdirectory(src/dependencies/GLFW/glfw-3.4   SYSTEM)
add_subdirectory(src/QFO2Tool                     SYSTEM)

#is this what allows <> around local #includes?
target_include_directories(
  ${EXECUTABLE_NAME}
  PUBLIC src/QFO2Tool
)

#this creates a new "library" for the linker (.dll/.so)
#(don't have to add a CMakeLists.txt file tinyfiledialogs folder)
#library still needs to be added to target_link_libraries()
add_library(tinyfiledialogs
  src/dependencies/tinyfiledialogs/tinyfiledialogs.c
  src/dependencies/tinyfiledialogs/tinyfiledialogs.h
)

add_library(STB
  src/dependencies/stb/stb_c_lexer.h
  src/dependencies/stb/stb_connected_components.h
  src/dependencies/stb/stb_divide.h
  src/dependencies/stb/stb_ds.h
  src/dependencies/stb/stb_dxt.h
  src/dependencies/stb/stb_easy_font.h
  src/dependencies/stb/stb_herringbone_wang_tile.h
  src/dependencies/stb/stb_hexwave.h
  src/dependencies/stb/stb_image_resize2.h
  src/dependencies/stb/stb_image_write.h
  src/dependencies/stb/stb_image.h
  src/dependencies/stb/stb_include.h
  src/dependencies/stb/stb_leakcheck.h
  src/dependencies/stb/stb_perlin.h
  src/dependencies/stb/stb_rect_pack.h
  src/dependencies/stb/stb_sprintf.h
  src/dependencies/stb/stb_textedit.h
  src/dependencies/stb/stb_tilemap_editor.h
  src/dependencies/stb/stb_truetype.h
  src/dependencies/stb/stb_voxel_render.h

  src/dependencies/stb/stb_vorbis.c
)

add_library(imgui_docking
  src/dependencies/imgui-1.90.8-docking/backends/imgui_impl_opengl3.h
  src/dependencies/imgui-1.90.8-docking/backends/imgui_impl_opengl3.cpp

  src/dependencies/imgui-1.90.8-docking/backends/imgui_impl_glfw.h
  src/dependencies/imgui-1.90.8-docking/backends/imgui_impl_glfw.cpp

  src/dependencies/imgui-1.90.8-docking/imgui.h
  src/dependencies/imgui-1.90.8-docking/imgui.cpp
  src/dependencies/imgui-1.90.8-docking/imgui_demo.cpp
  src/dependencies/imgui-1.90.8-docking/imgui_draw.cpp
  src/dependencies/imgui-1.90.8-docking/imgui_tables.cpp
  src/dependencies/imgui-1.90.8-docking/imgui_widgets.cpp
)

#must include target directories
#_AFTER_ the target is declared w/add_library()
target_include_directories(
  imgui_docking PUBLIC
  src/dependencies/imgui-1.90.8-docking
  src/dependencies/imgui-1.90.8-docking/backends
  src/dependencies/GLFW/glfw-3.4/include
)

target_include_directories(
  STB PUBLIC
  src/dependencies/stb/
)

# finally we link these libraries to our executable
# PRIVATE because nothing (target) will depend on our executable
target_link_libraries(
  ${EXECUTABLE_NAME} PRIVATE
  glad
  imgui_docking
  QFO2ToolLib
  tinyfiledialogs
  STB
  glfw
)

#this custom command copies the contents of src/resources
#to the compilation directory
#so the program will start without manually copying stuff
add_custom_command(
  TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/src/resources
  $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources
)