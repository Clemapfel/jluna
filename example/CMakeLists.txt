cmake_minimum_required(VERSION 3.12)
project(jluna_example VERSION 0.1.0 LANGUAGES CXX)

# detect Julia
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find")
find_package(Julia 1.7.0 REQUIRED)

# jluna
find_library(jluna REQUIRED NAMES jluna)
set(jluna_INCLUDE_DIRS "/usr/local/include/jluna;/usr/local/include/jluna/include")

# test
add_executable(jluna_example
    jluna_example.hpp
    jluna_example.cpp
    main.cpp
)
target_compile_features(jluna_example PUBLIC cxx_std_20)
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(jluna_example PUBLIC "-fpic")
endif()

# link against detected Julia library
target_include_directories(jluna_example PUBLIC
    "${PROJECT_SOURCE_DIR}"
    "${JULIA_INCLUDE_DIR}"
    "${jluna_INCLUDE_DIRS}"
)

target_link_libraries(jluna_example PUBLIC
    "${jluna}"
    "${JULIA_LIBRARY}"
)