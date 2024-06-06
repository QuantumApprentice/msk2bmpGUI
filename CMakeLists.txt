cmake_minimum_required(VERSION 3.25)
#CMAKE_MODULE_PATH is used for adding cmake plugins
#not using plugins right now, so commented out
# set(CMAKE_MODULE_PATH ${EAO_CMAKE_MODULE_PATH} ${CMAKE_MODULE_PATH})
# set(PROJECT_NAME "QFO2Solution")

#sets PROJECT_NAME, then other settings
#https://cmake.org/cmake/help/latest/command/project.html#project
project(
  QFO2Solution VERSION 0.1.0
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
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

#target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_17) # requires minimum C++17
#set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_EXTENSIONS FALSE)


target_sources(
  ${EXECUTABLE_NAME} PRIVATE
  src/msk2bmpGUI.cpp
)

#error if SDL2 packaged is not found
#not sure what SDL2_INCLUDE_DIRS are (#define? where defined?)
find_package(SDL2 REQUIRED COMPONENTS SDL2)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})


# There are a few way to get a dependecy, we will go through a few of them.

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

#is this what allows <> around local #includes?
target_include_directories(
  ${EXECUTABLE_NAME} PUBLIC
  src/QFO2Tool
)

#this creates a new "library" for the linker (.dll/.so)
#(you don't have to add a CMakeLists.txt file tinyfiledialogs folder)
#library still needs to be added to target_link_libraries()
add_library(tinyfiledialogs
  src/dependencies/tinyfiledialogs/tinyfiledialogs.c
  src/dependencies/tinyfiledialogs/tinyfiledialogs.h
)

# if a package or component is REQUIRED then the script will raise error and stop executing
# instead of keep going
# find_package(SDL2 REQUIRED COMPONENTS SDL2)
# find_package(SDL2_image REQUIRED)


# finally we link these libraries to our executable
# PRIVATE because nothing (target) will depend on our executable
target_link_libraries(
  ${EXECUTABLE_NAME} PRIVATE
  glad
  imgui_docking
  QFO2ToolLib
  tinyfiledialogs
  SDL2_image
  SDL2::SDL2
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