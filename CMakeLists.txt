cmake_minimum_required(VERSION 3.16)
project(jluna)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")

### JULIA ###

# set julia automatically if JULIA_PATH wasn't set
if (NOT DEFINED ENV{JULIA_PATH})
    execute_process(
            COMMAND julia -e "println(joinpath(Sys.BINDIR, \"..\"))"
            OUTPUT_VARIABLE JULIA_PATH_LOCAL)

    if ("${JULIA_PATH}" STREQUAL "")
    message(FATAL_ERROR "Cannot determine location of julia image. Before running cmake, please manually set the environment variable JULIA_PATH using\n    export JULIA_PATH=/path/to/your/.../julia\nIf you are unsure of the location of your julia image, you can access the path from within the julia REPL using\n    println(joinpath(Sys.BINDIR, \"..\"))
For more information, visit https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting")
    endif()

    set($ENV{JULIA_PATH} ${JULIA_PATH_LOCAL})
endif()

set(JULIA_LIB "$ENV{JULIA_PATH}/lib/libjulia.so")
set(JULIA_INCLUDE "$ENV{JULIA_PATH}/include/julia")
set(JULIA_FOUND TRUE)

# verify shared library
if (NOT EXISTS ${JULIA_LIB})
    message(WARNING "Cannot find shared library libjulia.so in $ENV{JULIA_PATH}/lib/")

    set(JULIA_FOUND FALSE)
endif()

# verify header
if (NOT EXISTS ${JULIA_INCLUDE}/julia.h)
    message(WARNING "Cannot find library header julia.h in $ENV{JULIA_PATH}/include/julia/")

    set(JULIA_FOUND FALSE)
endif()

if (${JULIA_FOUND})

    # verify version


    message("[LOG] Succesfully detected julia image at $ENV{JULIA_PATH}")
else()
    message(FATAL_ERROR "Failed to detect julia image. Make sure JULIA_PATH is set correctly and that the julia image is uncompressed and not corrupted.\nFor more information, visit https://github.com/Clemapfel/jluna/blob/master/README.md#troubleshooting")
endif()

include_directories(${JULIA_INCLUDE})

### JLUNA ###

include_directories(${CMAKE_SOURCE_DIR})

set(RESOURCE_PATH ${CMAKE_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/.src/include_julia.inl.in ${CMAKE_SOURCE_DIR}/.src/include_julia.inl @ONLY)

add_library(jluna SHARED
    jluna.hpp

    include/exceptions.hpp
    .src/exceptions.cpp

    include/typedefs.hpp
    include/concepts.hpp
    include/julia_extension.hpp

    include/box.hpp
    .src/box.inl

    include/unbox.hpp
    .src/unbox.inl

    include/state.hpp
    .src/state.cpp

    include/proxy.hpp
    .src/proxy.cpp
    .src/proxy.inl

    include/utilities.hpp

    include/array.hpp
    .src/array.inl
    .src/array_iterator.inl

    include/cppcall.hpp
    .src/cppcall.inl

    include/expression.hpp
    .src/expression.cpp

        .src/state.inl .src/typedefs.inl)

set_target_properties(jluna PROPERTIES
    LINKER_LANGUAGE C
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_library(jluna_c_adapter SHARED
    .src/c_adapter.hpp
    .src/c_adapter.cpp
)

set_target_properties(jluna_c_adapter PROPERTIES
    LINKER_LANGUAGE C
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

target_link_libraries(jluna_c_adapter ${JULIA_LIB})
target_link_libraries(jluna jluna_c_adapter ${JULIA_LIB})

### TEST EXECUTABLE ###

add_executable(JLUNA_TEST .test/main.cpp .test/test.hpp)
target_link_libraries(JLUNA_TEST jluna)
