#!/bin/bash
# init.sh
#
# this bash script creates a new C++ project using jluna.
# It is intended for novice users, advanced users are encouraged to
# `make install` jluna and link it and julia to their own project manually
#
# for more information, visit https://github.com/Clemapfel/jluna/blob/master/docs/installation.md
#
# @param $1: project name (i.e. "MyProject")
# @param $2: project root path (i.e "/home/user/Desktop")
# @param $3: (optional) compiler. One of: "g++-10", "g++-11", "clang++-12"
# @param $4: (optional) If `TRUE`, overwrites folder if it already exists.
#

# catch input
if [ -z "$1" ] | [ -z "$2" ]; then
  echo "Usage: /bin/bash init.sh <project_name> <project_root_path> [<compiler> = clang++-12]"
  exit 1
fi

# default compiler
if [ -n "$3" ]; then
  compiler="clang++-12"
else
  compiler="$3"
fi

# catch unset JULIA_BINDIR
if [ -z "$JULIA_BINDIR" ]; then
  printf "JULIA_BINDIR was not set. Please do so manually, using \`export JULIA_BINDIR=\$(julia -e \"println(Sys.BINDIR)\")\`\n"
  exit 0
fi

# project directory
project_name="$1"
project_root="$2"

# prevent accidental deletion
if [ -d "$project_root/$project_name" ]; then

  if [ "$4" == "TRUE" ]; then
    rm -r "$project_root/$project_name"
  else
    printf "Folder $project_root/$project_name already exists. Please first delete it manually or specify a different directory\n"
    exit 0
  fi
fi

# setup project folder
echo "creating $project_root/$project_name..."

cd $project_root
mkdir -p $project_name
cd $project_name

# clone jluna
git clone https://github.com/Clemapfel/jluna.git

# build jluna
cd jluna
mkdir -p build
cd build

cmake .. -DCMAKE_CXX_COMPILER=$compiler -DCMAKE_INSTALL_PREFIX="$project_root/$project_name/jluna/build"
make

# copy main.cpp, CMakeLists.txt and FindJulia into project folder
cd ../..
cp ./jluna/install/resources/CMakeLists.txt ./CMakeLists.txt
sed -i "s/@PROJECT_NAME@/$project_name/" ./CMakeLists.txt

cp ./jluna/install/resources/main.cpp ./main.cpp

mkdir -p $project_root/$project_name/cmake/find
cp ./jluna/install/resources/FindJulia.cmake ./cmake/find/FindJulia.cmake

# build project
cd $project_root/$project_name
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=$compiler
make

# execute hello world
printf "\n"
./hello_world

# exit
exit 0
