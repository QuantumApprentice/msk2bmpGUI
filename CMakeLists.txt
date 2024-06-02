cmake_minimum_required (VERSION 3.14)
 
# --- Google Test -------------------------------------------------------------
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# ------------------------------------------------------------------------------

project(msk2bmpGUI
  LANGUAGES CXX C
  VERSION 1.0.0
)

add_subdirectory(./specs)
add_subdirectory(./src/msk2bmp)
add_subdirectory(./dependencies/GLAD)
#add_subdirectory(./dependencies/GLFW)
add_subdirectory(./dependencies/imgui-docking)
add_subdirectory(./dependencies/tinyfiledialogs)

add_executable(msk2bmpGUI ./src/msk2bmpGUI.cpp)

target_include_directories(msk2bmpGUI PUBLIC
  src/msk2bmp
  ./dependencies/GLAD
  #  ./dependencies/GLFW
  ./dependencies/imgui-docking
  ./dependencies/tinyfiledialogs
)

target_link_libraries(msk2bmpGUI
  PUBLIC msk2bmp
)
