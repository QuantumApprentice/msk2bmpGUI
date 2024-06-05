#!/usr/bin/env bash

# Set up some safeguards if things go wrong
set -o pipefail     # fail if a command being piped into another command fails
shopt -s failglob   # fail if using a * and it doesn't match any files
set -u              # fail if we use a variable that hasn't been set
set -e              # fail if any command run fails

cd "${0%/*}" # change into the directory containing this script in case it's run from elsewhere

project_dir=`pwd`
src="$project_dir/src"
build="$project_dir/build"

CC_ARGS=(-g)
CPP_ARGS=(-g)

INCLUDE_DIRS=(
  "$src/QFO2Tool"
  "$src/dependencies/GLAD/include"
  "$src/dependencies/glfw-3.4/include"
  "$src/dependencies/imgui-docking/include"
  "$src/dependencies/tinyfiledialogs"
  "$src/dependencies/stb"
)

# To clean: rm -fr "$build"
mkdir -p "$build"

# 
# Initialize git submodules and update to the correct versions
#echo "Updating Submodules"
#git submodule init    >  "$build/git-submodule-update.log" 2>&1
#git submodule sync    >> "$build/git-submodule-update.log" 2>&1
#git submodule update  >> "$build/git-submodule-update.log" 2>&1

INCLUDE_ARGS=()
for dir in "${INCLUDE_DIRS[@]}"; do INCLUDE_ARGS+=(-I "$dir"); done

CC_ARGS+=("${INCLUDE_ARGS[@]}")
CPP_ARGS+=("${INCLUDE_ARGS[@]}")


echo "Building GLFW"
# unfortunately GLFW doesn't support a unity build so we need to build a couple different libs for it to link
cc "${CC_ARGS[@]}" -c -o "$build/glfw1.o" "$src/build_linux_glfw1.c"
cc "${CC_ARGS[@]}" -c -o "$build/glfw2.o" "$src/build_linux_glfw2.c"

echo "Building other C libraries (GLAD, tinyfiledialogs)"
cc "${CC_ARGS[@]}" -c -o "$build/c_libs.o" "$src/build_linux_c_libs.c"

echo "Building C++ libs (Dear ImGUI)"
c++ "${CPP_ARGS[@]}" -c -o "$build/cpp_libs.o" "$src/build_linux_cpp_libs.cpp"

echo "Building main project"
c++ "${CPP_ARGS[@]}" -o "$build/QFO2Tool" "$src/build_linux.cpp" "$build/glfw1.o" "$build/glfw2.o" "$build/c_libs.o" "$build/cpp_libs.o"

echo "Copying resources"
cp -a "$src/resources" "$build"

echo "Removing build artifacts"
rm "$build"/*.o

echo "Done"
