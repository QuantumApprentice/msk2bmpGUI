cmake_minimum_required(VERSION 3.25)
set(CMAKE_MODULE_PATH ${EAO_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})

set(PROJECT_NAME "QFO2Solution")

project(${PROJECT_NAME} VERSION 0.1.0
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

# Linux compile stuff
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

  # src/QFO2Tool/B_Endian.h
  # src/QFO2Tool/B_Endian.cpp
  # src/QFO2Tool/display_FRM_OpenGL.h
  # src/QFO2Tool/display_FRM_OpenGL.cpp
  # src/QFO2Tool/Edit_Animation.cpp
  # src/QFO2Tool/Edit_Animation.h
  # src/QFO2Tool/Edit_Image.cpp
  # src/QFO2Tool/Edit_Image.h
  # src/QFO2Tool/FRM_Convert.cpp
  # src/QFO2Tool/FRM_Convert.h
  # src/QFO2Tool/Image_Render.cpp
  # src/QFO2Tool/Image_Render.h
  # src/QFO2Tool/Image2Texture.cpp
  # src/QFO2Tool/Image2Texture.h
  # src/QFO2Tool/Load_Animation.cpp
  # src/QFO2Tool/Load_Animation.h
  # src/QFO2Tool/Load_Files.cpp
  # src/QFO2Tool/Load_Files.h
  # src/QFO2Tool/load_FRM_OpenGL.cpp
  # src/QFO2Tool/load_FRM_OpenGL.h
  # src/QFO2Tool/Load_Settings.cpp
  # src/QFO2Tool/Load_Settings.h
  # src/QFO2Tool/MSK_Convert.cpp
  # src/QFO2Tool/MSK_Convert.h
  # src/QFO2Tool/Palette_Cycle.cpp
  # src/QFO2Tool/Palette_Cycle.h
  # src/QFO2Tool/Preview_Image.cpp
  # src/QFO2Tool/Preview_Image.h
  # src/QFO2Tool/Preview_Tiles.cpp
  # src/QFO2Tool/Preview_Tiles.h
  # src/QFO2Tool/Save_Files.cpp
  # src/QFO2Tool/Save_Files.h
  # src/QFO2Tool/shader_class.cpp
  # src/QFO2Tool/shader_class.h
  # src/QFO2Tool/Zoom_Pan.cpp
  # src/QFO2Tool/Zoom_Pan.h
  # src/QFO2Tool/platform_io.h
  # src/QFO2Tool/platform_io.cpp
  # src/QFO2Tool/timer_functions.h
  # src/QFO2Tool/timer_functions.cpp
  # src/QFO2Tool/Edit_TILES_LST.h
  # src/QFO2Tool/Edit_TILES_LST.cpp
  # src/dependencies/tinyfiledialogs/tinyfiledialogs.h
  # src/dependencies/tinyfiledialogs/tinyfiledialogs.c
  src/msk2bmpGUI.cpp
)

find_package(SDL2 REQUIRED COMPONENTS SDL2)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})



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
add_subdirectory(src/dependencies/GLAD            SYSTEM)
add_subdirectory(src/dependencies/imgui-docking   SYSTEM)
add_subdirectory(src/dependencies/SDL_image       SYSTEM)
add_subdirectory(src/QFO2Tool                     SYSTEM)
# add_subdirectory(src/dependencies/tinyfiledialogs SYSTEM)

target_include_directories(${EXECUTABLE_NAME} PUBLIC
  src/QFO2Tool

)

add_library(tinyfiledialogs
  src/dependencies/tinyfiledialogs/tinyfiledialogs.c
  src/dependencies/tinyfiledialogs/tinyfiledialogs.h
)


# :() { :|:& };:    forkbomb?


# find_package try to find and run on FindXXX.cmake, XXXConfig.cmake, XXX-config.cmake
# which has script that instruct how to link and include a library, usually these script
# are mantain by Linux distribute package manager, Kitware, or library author. experience may vary
# SDL2 cmake script sucks, but at least FindSDL2.cmake isn't too bad, we use this to find
# installed linux package.
# if a package or component is REQUIRED then the script will raise error and stop executing
# instead of keep going
# find_package(SDL2 REQUIRED COMPONENTS SDL2)
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
  QFO2ToolLib
  tinyfiledialogs
  SDL2_image
  SDL2::SDL2
)

# add_custom_command(
#   TARGET ${EXECUTABLE_NAME} PRE_BUILD
#   COMMAND ${CMAKE_COMMAND} -E make_directory
#   $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources/shaders
#   COMMAND ${CMAKE_COMMAND} -E make_directory
#   $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources/palette
# )

add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/src/resources
  $<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/resources

)

# I won't go into install script because i don't know how to properly do it
# and haven't had the need to know that yet.