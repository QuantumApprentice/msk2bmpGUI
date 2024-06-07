#!/usr/bin/env bash

#safeguards
#(these are hard-coded and don't need to be strings)
set -o pipefail
shopt -s failglob
set -u
set -e

#run this script from it's containing directory
#(in case it's called from outside)
cd "${0%/*}"
#cd - change directory
#"" - must be string to support paths w/spaces
#     always use strings in shell scripts
#     for reliability (at least for variables)
#"$0" == "${0}" - script filename
#     (including full directory path)
#     path is relative to where script is called from
#"%" - removes suffix pattern
#    starts pattern matching from the next character
#"/" - literal slash character
#"*" - wildcard character
#    --basically matches a "/" then any character after
#       and removes them

#pwd - print working directory
#      can use either $() or ``
#      around command to return and assign results
project_dir=$(pwd)
src="$project_dir/src"
build="$project_dir/build"

# To clean: rm -fr "$build"
mkdir -p "$build"

# Initialize git submodules and update to the correct versions
#echo "Updating Submodules"
# git submodule init
# git submodule sync
# git submodule update

#creates an array of commands
#that will be passed to compiler
INCLUDE_ARGS=(
  -I "$src/QFO2Tool"
  -I "$src/dependencies/GLAD/include"
  -I "$src/dependencies/glfw-3.4/include"
  -I "$src/dependencies/imgui-1.90.8-docking"
  -I "$src/dependencies/tinyfiledialogs"
  -I "$src/dependencies/stb"
  -I "/usr/include/SDL2"
)
#^^^^^^^^^^^^^^^these area all variables storing file info

#"-g" compiler-generate debug info
#applies to compile step
#debug info works with GDB
CC_ARGS=(-g)
CPP_ARGS=(-g)

#append array of args to (-g)
CC_ARGS+=("${INCLUDE_ARGS[@]}")
CPP_ARGS+=("${INCLUDE_ARGS[@]}")

#"-c" option = compile only to object file (.o), no executable
#"-c" = Compile and assemble, but do not link
#"-o" = place output file here, with name in "quotes"
echo "Building GLFW"
cc "${CC_ARGS[@]}" -c -o "$build/glfw_null.o" "build_linux_glfw_null.c"
cc "${CC_ARGS[@]}" -c -o "$build/glfw.o"      "build_linux_glfw.c"
echo "GLFW Built"

#options first (-Dmacro -g -I)
#include folders next (.h files)
#compile method next (-c -o) (cc/c++ --help)
#object/executable destination/filename next
#source file.c at the end
#included libraries after
echo "Building other C libraries (GLAD, tinyfiledialogs)"
echo "Glad first..."
cc -g -I "$src/dependencies/GLAD/include" -c -o "$build/glad.o" "$src/dependencies/GLAD/src/glad.c"
echo $'GLAD Built\n'

echo "TinyFileDialogs next..."
#do not need include folder because TFD header
#is in the same folder as the .c file
cc -g -c -o "$build/tinyfiledialogs.o" "$src/dependencies/tinyfiledialogs/tinyfiledialogs.c"
echo "TinyFileDialogs built"
echo
echo "Building Dear ImGui..."
c++ "${CPP_ARGS[@]}" -c -o "$build/imgui_docking.o" "build_linux_dearimgui.c"
echo $'Dear ImGui Built\n'

echo "Building Qs FO2Tool..."
c++ -DQFO2_LINUX -I "$src/dependencies/imgui-1.90.8-docking/backends" "${CPP_ARGS[@]}" \
    -o "$build/QFO2Tool" "$project_dir/build_linux.cpp" \
    "$build"/*.o -lSDL2 -lSDL2_image
echo $'QFO2Tool Built\n'

echo "Copying resources"
cp -a "$src/resources" "$build"
echo $'Copying finished'

echo "Effing finally! DONE!"