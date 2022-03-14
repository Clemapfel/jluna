cmake_minimum_required(VERSION 3.16)

project(jluna VERSION 0.0.0 LANGUAGES CXX)

include(cmake/project-is-top-level.cmake)

# ---- Find Julia ----

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find")
find_package(Julia REQUIRED)

# ---- Declare C adapter ----

add_library(
    jluna_c_adapter SHARED
    .src/c_adapter.hpp
    .src/c_adapter.cpp
)

target_compile_features(jluna_c_adapter PUBLIC cxx_std_20)
target_include_directories(jluna_c_adapter PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>")

# NOTE: Julia::Julia is not present in the install interface. Consumers will
#       have to link to julia manually. Julia doesn't have a CMake package.
target_link_libraries(jluna_c_adapter PUBLIC "$<BUILD_INTERFACE:Julia::Julia>")

# ---- Declare library ----

add_library(
    jluna SHARED

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

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

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
