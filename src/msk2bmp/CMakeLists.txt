add_library(msk2bmp
  B_Endian.cpp
  B_Endian.h
  CMakeLists.txt
  display_FRM_OpenGL.cpp
  display_FRM_OpenGL.h
  Edit_Animation.cpp
  Edit_Animation.h
  Edit_Image.cpp
  Edit_Image.h
  Edit_TILES_LST.cpp
  Edit_TILES_LST.h
  FRM_Animate.cpp
  FRM_Animate.h
  FRM_Convert.cpp
  FRM_Convert.h
  Image2Texture.cpp
  Image2Texture.h
  Image_Render.cpp
  Image_Render.h
  Load_Animation.cpp
  Load_Animation.h
  Load_Files.cpp
  Load_Files.h
  load_FRM_OpenGL.cpp
  load_FRM_OpenGL.h
  Load_Settings.cpp
  Load_Settings.h
  MSK_Convert.cpp
  MSK_Convert.h
  Palette_Cycle.cpp
  Palette_Cycle.h
  platform_io.cpp
  platform_io.h
  Preview_Image.cpp
  Preview_Image.h
  Preview_Tiles.cpp
  Preview_Tiles.h
  Save_Files.cpp
  Save_Files.h
  shader_class.cpp
  shader_class.h
  timer_functions.cpp
  timer_functions.h
  Zoom_Pan.cpp
  Zoom_Pan.h
)

target_include_directories(msk2bmp PUBLIC
  ../../dependencies/GLAD/include
  ../../dependencies/imgui-docking/include
  ../../dependencies/tinyfiledialogs
)

# Windows compile stuff?
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")

target_compile_definitions(
  msk2bmp
  PUBLIC QFO2_WINDOWS
)

# Linux compile stuff?
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")

target_compile_definitions(
  msk2bmp
  PUBLIC QFO2_LINUX
)

endif()


