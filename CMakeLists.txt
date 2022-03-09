cmake_minimum_required(VERSION 3.16)
project(jluna)

#correct was to set a default build type
# https://blog.kitware.com/cmake-and-the-default-build-type/
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type was set. Setting build type to ${default_build_type}.")
  set(CMAKE_BUILD_TYPE ${default_build_type} CACHE 
    STRING "Choose the type to build" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

option(BUILD_SHARED_LIBS "prefer to build shared libraries" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiler support
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION STRGREATER_EQUAL "12.0.0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-volatile")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION STRGREATER_EQUAL "10.0.0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts -Wno-volatile")
else()
    message(FATAL_ERROR "Currently, the only supported compilers are G++10, G++11 (recommended) and Clang-12")
endif()

include(CTest)
include(GNUInstallDirs)

### JULIA ###

# access julia through JULIA_PATH environment variable
if (NOT DEFINED ENV{JULIA_PATH})
    execute_process(
            COMMAND julia -e "println(joinpath(Sys.BINDIR, \"..\"))"
            OUTPUT_VARIABLE JULIA_PATH_LOCAL)

    if ("${JULIA_PATH_LOCAL}" STREQUAL "")
        message(FATAL_ERROR "Cannot determine location of julia image. Before running cmake, please manually set the environment variable JULIA_PATH using\n    export JULIA_PATH=/path/to/your/.../julia\nIf you are unsure of the location of your julia image, you can access the path from within the julia REPL using\n    println(joinpath(Sys.BINDIR, \"..\"))
For more information, visit https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting")
    endif()

    set(ENV{JULIA_PATH} ${JULIA_PATH_LOCAL})
endif()

find_library(JULIA_LIB julia HINTS ENV JULIA_PATH REQUIRED)
find_path(JULIA_INCLUDE julia.h HINTS ENV JULIA_PATH PATH_SUFFIXES julia REQUIRED)


### JLUNA ###


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/.src/include_julia.inl.in ${CMAKE_CURRENT_SOURCE_DIR}/.src/include_julia.inl @ONLY)

add_library(jluna
    include/jluna.hpp

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

target_include_directories(jluna PUBLIC
  ${JULIA_INCLUDE}
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.src>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

add_library(jluna_c_adapter SHARED
    .src/c_adapter.hpp
    .src/c_adapter.cpp
)

target_include_directories(jluna_c_adapter
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    # users should include julia on their own
    PUBLIC $<BUILD_INTERFACE:${JULIA_INCLUDE}>
)

target_compile_features(jluna_c_adapter PUBLIC cxx_std_20)
target_link_libraries(jluna_c_adapter PUBLIC ${JULIA_LIB})
target_link_libraries(jluna PUBLIC jluna_c_adapter)

### EXECUTABLES ###

if(BUILD_TESTING)
  add_executable(JLUNA_TEST
      .test/main.cpp
      .test/test.hpp
  )
  target_link_libraries(JLUNA_TEST PRIVATE jluna)
  add_test(jluna_test JLUNA_TEST)
endif()

#add_executable(JLUNA_BENCHMARK
#   .benchmark/main.cpp
#   .benchmark/benchmark.hpp
#)
#target_link_libraries(JLUNA_BENCHMARK jluna ${JULIA_LIB})

### INSTALL ###

# TODO: ConfigVersion.cmake
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

install(FILES 
  ./.src/array.inl
  ./.src/array_iterator.inl
  ./.src/box.inl
  ./.src/cppcall.inl
  ./.src/exceptions.inl
  ./.src/include_julia.inl
  ./.src/module.inl
  ./.src/proxy.inl
  ./.src/state.inl
  ./.src/symbol.inl
  ./.src/typedefs.inl
  ./.src/type.inl
  ./.src/unbox.inl
  ./.src/usertype.inl
  ./.src/c_adapter.hpp
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jluna)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/jluna)

target_include_directories(jluna INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/jluna>)
