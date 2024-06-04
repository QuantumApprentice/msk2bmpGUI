cmake_minimum_required(VERSION 3.27)

#set(PROJECT_NAME "glad_for_fallout")

project(
    "glad_for_fallout" VERSION 0.0.1
#    DESCRIPTION "glad_for_fallout 
    DESCRIPTION "${PROJECT_NAME} 
    Open and edit Fallout 2 image files.")

add_library(glad STATIC)

target_sources(
    glad PRIVATE
    include/glad/glad.h
    include/KHR/khrplatform.h
    src/glad.c
)

# ${${PROJECT_NAME}_SOURCE_DIR} will resoved to glad_SOURCE_DIR
# which is the source directory of the call to project with name glad
# which is this file 's directory: msk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/dependencies/GLAD
# PUBLIC because glad itself depends on these files (that in the include path), as well as
# the consumer of glad.
# PUBLIC allows the compilation unit of glad.c to have 
# -Imsk2bmpGUI_C_C++_imgui_sdl2_tinyfiledialogs/dependencies/GLAD/include
# and any other consumer of glad as well.

target_include_directories(
    glad
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

# $< : > thingy is generator expression, they are useful
# https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html
# read BUILD_INTERFACE link above ^, we use this here because the include dir
# when building (${${PROJECT_NAME}_SOURCE_DIR}/include) is different than
# the include dir when we install and we aren't going to install this glad library.