cmake_minimum_required(VERSION 3.12)

project(jluna VERSION 0.8.4 LANGUAGES CXX)

include(cmake/project-is-top-level.cmake)
if(PROJECT_IS_TOP_LEVEL)
    option(DEVELOPER_MODE "Enable developer mode" OFF)
endif()

# ---- Find Julia ----

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find")
find_package(Julia 1.7.0 REQUIRED)

# ---- Resource path include ----

# FIXME: This is pointing at the project directory, this obviously will not
#        work after install. Figure out how to make this work as a relocatable
#        CMake package with paths being relative to the prefix.
file(READ include/jluna.jl JULIA_SOURCE)
set(C_ADAPTER_PATH ${CMAKE_INSTALL_PREFIX})
configure_file("${CMAKE_SOURCE_DIR}/.src/include_julia.inl.in" "${CMAKE_SOURCE_DIR}/.src/include_julia.inl" @ONLY)

# ---- Declare C adapter ----

add_library(
    jluna_c_adapter SHARED
    .src/c_adapter.hpp
    .src/c_adapter.cpp
)

target_compile_features(jluna_c_adapter PUBLIC cxx_std_20)
target_include_directories(jluna_c_adapter PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>")
target_link_libraries(jluna_c_adapter PUBLIC "$<BUILD_INTERFACE:Julia::Julia>")

set_target_properties(jluna_c_adapter PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${C_ADAPTER_PATH}
)

message("[LOG] writing libjluna_c_adapter to globally available directory \"${C_ADAPTER_PATH}\"")

# ---- Declare library ----

add_library(jluna SHARED

    jluna.hpp

    include/exceptions.hpp
    .src/exceptions.cpp
    .src/exceptions.inl

    include/typedefs.hpp
    .src/typedefs.inl

    include/concepts.hpp
    include/julia_extension.hpp

    include/box.hpp
    .src/box.inl

    include/unbox.hpp
    .src/unbox.inl

    include/state.hpp
    .src/state.cpp
    .src/state.inl

    include/proxy.hpp
    .src/proxy.cpp
    .src/proxy.inl

    include/array.hpp
    .src/array.inl
    .src/array_iterator.inl

    include/cppcall.hpp
    .src/cppcall.inl

    include/expression.hpp
    .src/expression.cpp

    include/module.hpp
    .src/module.cpp
    .src/module.inl

    include/symbol.hpp
    .src/symbol.cpp
    .src/symbol.inl

    include/type.hpp
    .src/type.cpp
    .src/type.inl

    include/function.hpp
    .src/function.cpp

    include/generator_expression.hpp
    .src/generator_expression.cpp

    include/usertype.hpp
    .src/usertype.inl

    include/gc_sentinel.hpp
)

target_link_libraries(jluna PUBLIC jluna_c_adapter)
target_include_directories(jluna PRIVATE "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>")

# ---- HACK: export all symbols on Windows ----

# FIXME: Exporting all symbols is bad practice. Fix this using the
#        GenerateExportHeader CMake module and visibility proprties.
set_target_properties(jluna PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS YES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT DEVELOPER_MODE)
    return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
    message(
        AUTHOR_WARNING
        "Developer mode is intended for developers of jluna"
    )
endif()

include(CTest)
if(DEVELOPER_MODE)
    add_executable(
        jluna_test
        .test/main.cpp
        .test/test.hpp
    )
    target_link_libraries(jluna_test PRIVATE jluna)
    add_test(NAME jluna_test COMMAND jluna_test)

    add_executable(
        jluna_benchmark
        .benchmark/main.cpp
        .benchmark/benchmark.hpp
        .benchmark/benchmark_aux.hpp
    )
    target_link_libraries(jluna_benchmark PRIVATE jluna)
endif()
