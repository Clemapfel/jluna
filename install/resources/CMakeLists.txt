# this file was generate by jluna/install/init.sh
cmake_minimum_required(VERSION 3.12)
project(@PROJECT_NAME@ VERSION 0.0.0 LANGUAGES CXX)

# find julia
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/find")
find_package(Julia 1.7.0 REQUIRED)

# find jluna
set(CMAKE_FIND_LIBRARY_SUFFIXES_WIN32 ".lib;.dll;.dll.a;")
set(CMAKE_FIND_LIBRARY_SUFFIXES_UNIX ".so")

find_library(jluna REQUIRED
	NAMES jluna
	PATHS ${CMAKE_SOURCE_DIR}/jluna/build)

# executable
add_executable(my_executable ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(my_executable PRIVATE
    ${jluna}
    "$<BUILD_INTERFACE:Julia::Julia>"
)

# set C++ standard
target_compile_features(my_executable PRIVATE cxx_std_20)

# include directories
target_include_directories(my_executable PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/jluna
    ${JULIA_INCLUDE_DIR}
)