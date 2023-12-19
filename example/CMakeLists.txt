cmake_minimum_required(VERSION 3.12)
project(jluna_example VERSION 0.1.0 LANGUAGES CXX)

# find jluna, this also sets `JLUNA_LIBRARIES` and `JLUNA_INCLUDE_DIRECTORIES`
find_package(jluna REQUIRED)

include(GNUInstallDirs)
message(WARNING ${CMAKE_INSTALL_LIBDIR})

# add executable
add_executable(jluna_example
    jluna_example.hpp
    jluna_example.cpp
    main.cpp
)

# set to C++20
target_compile_features(jluna_example PUBLIC cxx_std_20)

# add jluna and Julia include directories
target_include_directories(jluna_example PUBLIC
    ${PROJECT_SOURCE_DIR}
    ${JLUNA_INCLUDE_DIRECTORIES}
)

# link against jluna and Julia
target_link_libraries(jluna_example PUBLIC
    ${JLUNA_LIBRARIES}
)

# Julia C-Library requires `-fpic` for Clang or GCC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(jluna_example PUBLIC "-fpic")
endif()