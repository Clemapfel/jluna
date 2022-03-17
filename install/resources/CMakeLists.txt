# this file was generate by jluna/install/init.sh

cmake_minimum_required(VERSION 3.12)
project(MyProject VERSION 0.0.0 LANGUAGES CXX)

# find julia
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/find")
find_package(Julia 1.7.0 REQUIRED)

# find jluna
find_library(jluna REQUIRED
    NAMES libjluna.so libjluna.dll.a libjluna.dll libjluna.lib
    PATHS ${CMAKE_SOURCE_DIR}/jluna ${CMAKE_SOURCE_DIR}/jluna/build)

# executable
add_executable(my_executable ${CMAKE_SOURCE_DIR}/main.cpp)
target_link_libraries(my_executable PRIVATE
    ${jluna}
    "$<BUILD_INTERFACE:Julia::Julia>"
)
target_compile_features(my_executable PUBLIC cxx_std_20)
target_include_directories(my_executable PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>"
    ${CMAKE_SOURCE_DIR}/jluna
    ${JULIA_INCLUDE_DIR}
)