cmake_minimum_required(VERSION 3.25)
set(CMAKE_MODULE_PATH ${EAO_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

set(PROJECT_NAME "QFO2Solution")

project(${PROJECT_NAME} VERSION 0.0.1
  DESCRIPTION "${PROJECT_NAME}"
)
set(EXECUTABLE_NAME "QFO2Tool")
add_executable(${EXECUTABLE_NAME})

# Windows compile stuff?
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

target_compile_definitions(
  ${EXECUTABLE_NAME}
  PUBLIC QFO2_WINDOWS
)


# Linux compile stuff?
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

target_compile_definitions(
  ${EXECUTABLE_NAME}
  PUBLIC QFO2_LINUX
)

endif()


# I was wrong, this is fine as well as the target way, this will set default
# for all the target that doesn't specify for themself
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

#target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17) # requires minimum C++17
#set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_EXTENSIONS FALSE)


# PRIVATE here is different than the others, usually i don't know what is this
# so ... read the book?
target_sources(
  ${EXECUTABLE_NAME} PRIVATE

  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/B_Endian.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/B_Endian.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/display_FRM_OpenGL.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/display_FRM_OpenGL.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Edit_Animation.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Edit_Animation.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Edit_Image.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Edit_Image.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/FRM_Animate.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/FRM_Animate.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/FRM_Convert.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/FRM_Convert.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Image_Render.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Image_Render.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Image2Texture.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Image2Texture.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Load_Animation.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Load_Animation.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Load_Files.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Load_Files.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/load_FRM_OpenGL.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/load_FRM_OpenGL.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Load_Settings.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Load_Settings.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/MSK_Convert.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/MSK_Convert.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/msk2bmpGUI.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Palette_Cycle.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Palette_Cycle.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Preview_Image.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Preview_Image.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Preview_Tiles.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Preview_Tiles.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Save_Files.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Save_Files.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/shader_class.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/shader_class.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/tinyfiledialogs.c
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/tinyfiledialogs.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Zoom_Pan.cpp
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/Zoom_Pan.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/platform_io.h
  msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/platform_io.cpp
)

# There are a few way to get a dependecy, we will go though a few of them.

# the SYSTEM parameter indicate the library is a system library which
# tell the compiler to not run certain static analysis like warnings
# because as library users we tent not to modify other people library.
# g++ notice -isystem instead of -I

# We write CMakeLists.txt ourself, usually this is not recommended because
# it's is not the library users responsiblity to mantain build scripts.
# but since glad, imgui is a simple drop in library, it is trivial
# Writing CMakeLists.txt ourself usually the last resort when it comes to libraries
# that doesn't care to support cmake.
add_subdirectory(msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/dependencies/GLAD          SYSTEM)
add_subdirectory(msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/dependencies/imgui-docking SYSTEM)
add_subdirectory(msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/dependencies/SDL_image     SYSTEM)

# find_package try to find and run on FindXXX.cmake, XXXConfig.cmake, XXX-config.cmake
# which has script that instruct how to link and include a library, usually these script
# are mantain by Linux distribute package manager, Kitware, or library author. experience may vary
# SDL2 cmake script sucks, but at least FindSDL2.cmake isn't too bad, we use this to find
# installed linux package.
# if a package or component is REQUIRED then the script will raise error and stop executing
# instead of keep going
find_package(SDL2 REQUIRED COMPONENTS SDL2)
# find_package(SDL2_image REQUIRED)


# One other way, use external package manager like conan and vcpgk
# they are other can of worms, which sometime can be useful but setting them
# up could be a little complicated.

# One otherway that we could build the library ourself outside of this probject
# and set variable to their path so find_package could do the linking work for us.
# or we could link .a files, and include directories ourself, a little bit
# hacky IMO.


# finally we link these libraries to our executable
# PRIVATE because nothing (target) going to depends on our executable
target_link_libraries(
  ${EXECUTABLE_NAME}
  PRIVATE
  glad
  imgui_docking
  SDL2_image
  SDL2::SDL2
)

add_custom_command(
  TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
  $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources/shaders

  COMMAND ${CMAKE_COMMAND} -E make_directory
  $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources/palette
)

add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/resources/shaders
  $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources/shaders

  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/resources/palette
  $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources/palette
)

# I won't go into install script because i don't know how to properly do it
# and haven't had the need to know that yet.