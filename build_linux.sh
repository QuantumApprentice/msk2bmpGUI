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

echo $src




# git submodule init
# git submodule sync
# git submodule update