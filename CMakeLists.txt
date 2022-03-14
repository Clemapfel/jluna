cmake_minimum_required(VERSION 3.16)

project(jluna LANGUAGES CXX)

# ---- Find Julia ----

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find")
find_package(Julia REQUIRED)

### JLUNA ###

include_directories(${CMAKE_SOURCE_DIR})

set(RESOURCE_PATH ${CMAKE_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/.src/include_julia.inl.in ${CMAKE_SOURCE_DIR}/.src/include_julia.inl @ONLY)

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

set_target_properties(jluna PROPERTIES
    LINKER_LANGUAGE C
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)
target_compile_features(jluna PUBLIC cxx_std_20)

### C ADAPTER ##

add_library(jluna_c_adapter SHARED
    .src/c_adapter.hpp
    .src/c_adapter.cpp
)

target_include_directories(jluna_c_adapter
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    PUBLIC $<BUILD_INTERFACE:${JULIA_INCLUDE}>
)

target_link_libraries(jluna_c_adapter PUBLIC $<BUILD_INTERFACE:${JULIA_LIB}>)

set_target_properties(jluna_c_adapter PROPERTIES
    LINKER_LANGUAGE C
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

target_link_libraries(jluna PUBLIC jluna_c_adapter)

### EXECUTABLES ###

add_executable(JLUNA_TEST
    .test/main.cpp
    .test/test.hpp
)
target_link_libraries(JLUNA_TEST jluna ${JULIA_LIB})

add_executable(JLUNA_BENCHMARK
    .benchmark/main.cpp
    .benchmark/benchmark.hpp
    .benchmark/benchmark_aux.hpp
)
target_link_libraries(JLUNA_BENCHMARK jluna ${JULIA_LIB})

### INSTALL ###

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/jluna-config.cmake [[
if (NOT ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION STRGREATER_EQUAL "12.0.0") OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION STRGREATER_EQUAL "10.0.0")))
    message(FATAL_ERROR "Currently, the only supported compilers are G++10, G++11 (recommended) and Clang-12")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/jluna-targets.cmake")
]])

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/jluna-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jluna
)

install(TARGETS jluna jluna_c_adapter EXPORT jluna_targets)

install(EXPORT jluna_targets
    FILE jluna-targets.cmake
    NAMESPACE jluna::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/jluna
)

install(FILES jluna.hpp ${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jluna)
install(DIRECTORY include .src DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jluna)

target_include_directories(jluna INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/jluna>)