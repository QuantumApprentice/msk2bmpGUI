#!/usr/bin/env bash

#safeguards
#(these are hard-coded and don't need to be strings)
set -o pipefail
shopt -s failglob
set -u
set -e    #halts script if compile fails

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
  -I "$src/dependencies/GLFW/glfw-3.4/include"
  -I "$src/dependencies/imgui-1.90.8-docking"
  -I "$src/dependencies/imgui-1.90.8-docking/backends"
  -I "$src/dependencies/ImFileDialog"
  -I "$src/dependencies/tinyfiledialogs"  #this goes byebye soon
  -I "$src/dependencies/stb"
  # -I "/usr/include/SDL2"
)
#^^^^^^^^^^^^^^^these area all variables storing file info

#"-g" compiler-generate debug info
#applies to compile step
#debug info works with GDB
CC_ARGS=( -g -mavx "${INCLUDE_ARGS[@]}")
CPP_ARGS=(-g -mavx "${INCLUDE_ARGS[@]}")

if [[ "${1:-}" == "clean" ]];
  then
  shift
  rm -fr build
fi

if [[ "${1:-}" == "release" ]];
  then
  shift
  CC_ARGS+=(-O3)
  CPP_ARGS+=(-O3)
fi

mkdir -p build

#"-c" option = compile only to object file (.o), no executable
#"-c" = Compile and assemble, but do not link
#"-o" = place output file here, with name in "quotes"
echo "Building GLFW"
cc "${CC_ARGS[@]}" -c -o "$build/glfw_null.o" "build_linux_glfw_null.c"
cc "${CC_ARGS[@]}" -c -o "$build/glfw.o"      "build_linux_glfw.c"
echo "GLFW Built"

#options first (-Dmacro -g)
#include folders next (-I /dir/*.h files) (need -I per directory)
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
c++ -DQFO2_LINUX \
    "${CPP_ARGS[@]}" \
    -o "$build/QFO2Tool" "$project_dir/build_linux.cpp" \
    "$build"/*.o 
echo $'QFO2Tool Built\n'

echo "Copying resources"
# cp -a "$src/resources" "$build"
cp --update -R "$src/resources" "$build"
echo $'Copying finished'

echo $'Build Finished!\n\n'


###############################################
echo $'Test starting...\n'

coverage=0
#${0} == zero indexed, always _this_ script name
#        parameters (${1}...etc) are passed here
#${1} == index 1, first argument after script name
if [[ "${1:-}" == "coverage" ]];
      #:- == defaults to empty string if no arg provided
  then
  shift   #shifts ${2...etc} down one index
          # so ${2} becomes ${1} etc,
          # but ${0} is not touched
  CPP_ARGS+=(--coverage)
  cov=1
fi  #apparently this is how you close an if check

if [[ "${1:-}" == "profile" ]];
  then
  shift
  CPP_ARGS+=(-DREPETITIONS=1000000) #one million repititions
fi

if [[ "${1:-}" == "test" ]];
  then
  shift
  echo "Compiling tests..."
  # cp -a test/test_resources build
  c++ -DQFO2_LINUX "${CPP_ARGS[@]}"     \
      -o build/test                     \
      test/test_crop_single_tile.cpp    \
      test/test_assign_tile_id.cpp      \
      # "$build"/*.o -lSDL2 -lSDL2_image

  echo "Running tests..."
  #() == parenthises create a subshell to run commands in
  #   this changes dir to build and runs ./test
  #   the return from test is checked by || operator
  (cd build && ./test) || echo "Tests failed :_("

fi

echo "Donesers..."

