cmake_minimum_required(VERSION 3.12)

#[=======================================================================[.rst:

Build jluna
-----------

This cmake attempts to automatically detect the Julia image.
If this is not possible, manually specify the path to the Julia binary using:

    ``-DJULIA_BINDIR=/path/to/.../julia/bin``

If you are unsure of the location of this folder, you can access
it from within the REPL using:

    ``println(Sys.BINDIR)``

Options
^^^^^^^
``jluna_DEVELOPER_MODE``
    enable building test and benchmark executables. Off by default
``BUILD_TESTING``
    build jluna_test, as CTest. On by default
``BUILD_BENCHMARK``
    build jluna_benchmark. Off by default

#]=======================================================================]

project(jluna VERSION 1.0.0 LANGUAGES CXX)

include(cmake/project-is-top-level.cmake)
if(PROJECT_IS_TOP_LEVEL)
    option(jluna_DEVELOPER_MODE "Enable developer mode" OFF)
endif()

### Find Julia ###

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find")
find_package(Julia 1.7.0 REQUIRED)

### Find Threads ###

find_package(Threads)

### Configure Files ###

include(GNUInstallDirs)
set(SHARED_LIBRARY_NAME "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}jluna${CMAKE_SHARED_LIBRARY_SUFFIX}")

# c.f. .src/include_julia.inl.in
file(READ .src/julia/jluna_01.jl JLUNA_01)
file(READ .src/julia/jluna_02.jl JLUNA_02)
file(READ .src/julia/jluna_03.jl JLUNA_03)
file(READ .src/julia/jluna_04.jl JLUNA_04)
file(READ .src/julia/jluna_05.jl JLUNA_05)
file(READ .src/julia/jluna_06.jl JLUNA_06)

configure_file("${CMAKE_SOURCE_DIR}/include/jluna/inline/include_julia.inl.in" "${CMAKE_SOURCE_DIR}/include/jluna/inline/include_julia.inl" @ONLY)

### Declare Library ###

set(JLUNA_INCLUDE
    jluna.hpp
    include/jluna/exceptions.hpp
    include/jluna/typedefs.hpp
    include/jluna/inline/typedefs.inl
    include/jluna/unsafe_utilities.hpp
    include/jluna/inline/unsafe_utilities.inl
    include/jluna/safe_utilities.hpp
    include/jluna/inline/safe_utilities.inl
    include/jluna/concepts.hpp
    include/jluna/box.hpp
    include/jluna/inline/box.inl
    include/jluna/unbox.hpp
    include/jluna/inline/unbox.inl
    include/jluna/proxy.hpp
    include/jluna/inline/proxy.inl
    include/jluna/array.hpp
    include/jluna/inline/array.inl
    include/jluna/inline/array_iterator.inl
    include/jluna/cppcall.hpp
    include/jluna/inline/cppcall.inl
    include/jluna/module.hpp
    include/jluna/inline/module.inl
    include/jluna/symbol.hpp
    include/jluna/inline/symbol.inl
    include/jluna/type.hpp
    include/jluna/inline/type.inl
    include/jluna/generator_expression.hpp
    include/jluna/usertype.hpp
    include/jluna/inline/usertype.inl
    include/jluna/multi_threading.hpp
    include/jluna/inline/multi_threading.inl
    include/jluna/mutex.hpp
    include/jluna/c_adapter.hpp
)

set(JLUNA_SRC
    .src/exceptions.cpp
    .src/unsafe_utilities.cpp
    .src/safe_utilities.cpp
    .src/proxy.cpp
    .src/module.cpp
    .src/symbol.cpp
    .src/type.cpp
    .src/generator_expression.cpp
    .src/mutex.cpp
    .src/c_adapter.cpp
)

add_library(jluna SHARED
    ${JLUNA_INCLUDE}
    ${JLUNA_SRC}
)

target_compile_features(jluna PUBLIC cxx_std_20)

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(jluna PUBLIC "-fpic")
endif()

target_include_directories(jluna PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/include"
    "${JULIA_INCLUDE_DIR}"
)

target_link_libraries(jluna PUBLIC
    "$<BUILD_INTERFACE:Julia::Julia>"
    "$<BUILD_INTERFACE:Threads::Threads>"
    ${JULIA_LIBRARY}
)

set_target_properties(jluna PROPERTIES
    LINKER_LANGUAGE CXX
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS "YES" # TODO: Why is this necessary?
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION}"
    INTERFACE_INCLUDE_DIRECTORIES "${JULIA_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES ${JULIA_LIBRARY}
)

### Install rules ###

if(NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif()

### Declare Test ###

option(JLUNA_BUILD_TESTING "Enable jluna_test" ON)
if (JLUNA_BUILD_TESTING)
    include(CTest)
    add_executable(
        jluna_test
        "${CMAKE_SOURCE_DIR}/.test/main.cpp"
        "${CMAKE_SOURCE_DIR}/.test/test.hpp"
    )
    target_link_libraries(jluna_test PRIVATE jluna)
    target_include_directories(jluna_test PRIVATE ${JLUNA_INCLUDE_DIRECTORIES})
    add_test(NAME jluna_test COMMAND jluna_test)
endif()

### Declare Benchmark ###

option(JLUNA_BUILD_BENCHMARK "Enable jluna_benchmark" OFF)
if (JLUNA_BUILD_BENCHMARK)
    add_executable(
        jluna_benchmark
        .benchmark/main.cpp
        .benchmark/benchmark.hpp
        .benchmark/benchmark_aux.hpp
    )
    target_link_libraries(jluna_benchmark PRIVATE jluna)
endif()
