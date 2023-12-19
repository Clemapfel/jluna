cmake_minimum_required(VERSION 3.12)

# set project name
set(PROJECT_NAME jluna_example)

# declare project
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX)

# find jluna, this also sets `JLUNA_LIBRARIES` and `JLUNA_INCLUDE_DIRECTORIES`
find_package(jluna REQUIRED VERSION 1.1.0)

# add executable
add_executable(${PROJECT_NAME}
    jluna_example.hpp
    jluna_example.cpp
    main.cpp
)

# set cpp standard to c++20
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

# add jluna and Julia include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${JLUNA_INCLUDE_DIRECTORIES}
)

# link against jluna and Julia
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${JLUNA_LIBRARIES}
)

# Julia C-Library requires `-fpic` for Clang or GCC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PUBLIC "-fpic")
endif()