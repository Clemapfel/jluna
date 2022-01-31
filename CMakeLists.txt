cmake_minimum_required(VERSION 3.16)
project(jluna)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

### JULIA ###

set(JULIA_DIR /home/clem/Applications/julia/)
include_directories(${JULIA_DIR}/include/julia/)

### JLUNA ###

set(RESOURCE_PATH ${CMAKE_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/.src/include_julia.inl.in ${CMAKE_SOURCE_DIR}/.src/include_julia.inl @ONLY)

add_library(jluna SHARED
    jluna.hpp

        include/exceptions.hpp
        .src/exceptions.cpp
        include/typedefs.hpp include/julia_extension.hpp include/box.hpp include/concepts.hpp .src/box.inl include/unbox.hpp .src/unbox.inl include/state.hpp .src/state.cpp)

set_target_properties(jluna PROPERTIES
    LINKER_LANGUAGE C
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(jluna ${JULIA_DIR}/lib/libjulia.so)

### TEST EXECUTABLE ###

add_executable(JLUNA_TEST .test/main.cpp .test/test.hpp)
target_link_libraries(JLUNA_TEST jluna ${JULIA_DIR}/lib/libjulia.so)