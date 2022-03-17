#!/bin/bash

#
# init.sh
#
# this bash script creates a new C++ project using jluna.
# It is intended to be used for novice users, advanced users are encouraged to
# `make install` jluna and link to it and julia in their own project
#
# for more information, visit https://github.com/Clemapfel/jluna/blob/master/docs/installation.md
#

if [ -z "$1" ] | [ -z "$2" ]; then
  echo "Usage: /bin/bash init.sh <project_name> <project_root_path>"
  exit 1
fi

project_name="$1"
project_root="$2"

echo "creating $project_root/$project_name..."

cd $project_root
mkdir -p $project_name
cd $project_name

git clone -b cmake_rework https://github.com/Clemapfel/jluna.git

cd jluna
mkdir -p build
cd build

cmake .. -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_INSTALL_PREFIX="$project_root/$project_name/jluna"
make install

cd ../..
cp ./jluna/install/resources/CMakeLists.txt ./CMakeLists.txt
cp ./jluna/install/resources/main.cpp ./main.cpp

mkdir -p $project_root/$project_name/cmake/find
cp ./jluna/install/resources/FindJulia.cmake ./cmake/find/FindJulia.cmake

cd $project_root/$project_name
mkdir - build
cd build
cmake ..
make

exit 0




