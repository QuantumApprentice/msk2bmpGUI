#!/usr/bin/env bash

# Set up some safeguards if things go wrong
set -o pipefail     # fail if a command being piped into another command fails
shopt -s failglob   # fail if using a * and it doesn't match any files
set -u              # fail if we use a variable that hasn't been set
set -e              # fail if any command run fails

cd "${0%/*}" # change into the directory containing this script in case it's run from elsewhere

INCLUDE_ARGS=(
  -I "src/QFO2Tool"
  -I "src/dependencies/GLAD/include"
  -I "src/dependencies/glfw-3.4/include"
  -I "src/dependencies/imgui-docking/include"
  -I "src/dependencies/tinyfiledialogs"
  -I "src/dependencies/stb"
)

CC_ARGS=(-g "${INCLUDE_ARGS[@]}")
CPP_ARGS=(-g "${INCLUDE_ARGS[@]}")

if [[ "${1:-}" == "clean" ]]; then
  shift
  rm -fr build
fi

if [[ "${1:-}" == "release" ]]; then
  shift
  CC_ARGS+=(-O3)
  CPP_ARGS+=(-O3)
fi

mkdir -p build

echo "Building C libraries (GLFW, GLAD, tinyfiledialogs)"
cc "${CC_ARGS[@]}" -c -o build/c_libs.o src/build_linux_c_libs.c

echo "Building C++ libs (Dear ImGUI)"
c++ "${CPP_ARGS[@]}" -c -o build/cpp_libs.o src/build_linux_cpp_libs.cpp

echo "Building main project"
c++ "${CPP_ARGS[@]}" -o build/QFO2Tool src/build_linux.cpp build/c_libs.o build/cpp_libs.o

echo "Copying resources"
cp -a src/resources build

cov=0
if [[ "${1:-}" == "coverage" ]]; then
  shift
  CPP_ARGS+=(--coverage)
  cov=1
fi

if [[ "${1:-}" == "test" ]]; then
  shift
  cp -a test/test_resources build
  c++ "${CPP_ARGS[@]}" -o build/test test/test_tile_export.cpp build/c_libs.o build/cpp_libs.o
  echo "Running tests"
  (cd build && ./test) || echo 'Tests failed'
  [[ "$cov" == 1 ]] && (mkdir -p build/coverage && gcov build/QFO2Tool_test-test_tile_export && mv -- *.gcov build/coverage) >/dev/null && echo "Coverage files generated from tests in build/coverage"
fi

echo "Done"
