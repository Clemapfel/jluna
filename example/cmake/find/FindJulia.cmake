#
# This FindJulia.cmake is intended to detect Julia as a package.
#
# Use
# `list(APPEND CMAKE_MODULE_PATH "path/to/this/file")`
# `find_package(Julia 1.7.0 REQUIRED)`
#
# to make the Julia target available through:
# `"$<BUILD_INTERFACE:Julia::Julia>"`
#
# Furthermore the following variables will be set on
# successful detection:
#
# JULIA_LIBRARY         : Julia shared library
# JULIA_EXECUTABLE      : Julia REPL executable
# JULIA_BINDIR          : directory to Julia binary
# JULIA_INCLUDE_DIR     : directory that contains julia.h
#
#[=======================================================================[.rst:

FindJulia
-----------

This module is intended to detect Julia as a package.

Imported Targets
^^^^^^^^^^^^^^^^

``Julia::Julia``
    Julia Package, if found

Result Variables
^^^^^^^^^^^^^^^^

``JULIA_LIBRARY``
    Julia shared library
``JULIA_EXECUTABLE``
    Julia REPL executable
``JULIA_BINDIR``
    directory to Julia binary
``JULIA_INCLUDE_DIR``
    directory that contains Julia.h

Usage
^^^^^
Use:
    ``list(APPEND CMAKE_MODULE_PATH "path/to/this/file")``
    ``find_package(Julia 1.7.0 REQUIRED)``

to make the Julia target available to your target through
    ``"$<BUILD_INTERFACE:Julia::Julia>"``

#]=======================================================================]

macro(julia_bail_if_false message var)
    if(NOT ${var})
        set(Julia_FOUND 0)
        set(Julia_NOT_FOUND_MESSAGE "${message}")
        return()
    endif()
endmacro()

# detect Julia executable
find_program(JULIA_EXECUTABLE julia PATHS ENV JULIA_BINDIR)
julia_bail_if_false("Unable to detect the Julia executable. Make sure JULIA_BINDIR is set correctly." JULIA_EXECUTABLE)

# detect Julia binary dir
if(NOT DEFINED JULIA_BINDIR)
    # The executable could be a chocolatey shim, so run some Julia code to report
    # the path of the BINDIR
    execute_process(
        COMMAND "${JULIA_EXECUTABLE}" -e "print(Sys.BINDIR)"
        OUTPUT_VARIABLE JULIA_BINDIR_LOCAL
    )
    file(TO_CMAKE_PATH "${JULIA_BINDIR_LOCAL}" JULIA_BINDIR_LOCAL)
    set(JULIA_BINDIR "${JULIA_BINDIR_LOCAL}" CACHE PATH "")
endif()
get_filename_component(JULIA_PATH_PREFIX "${JULIA_BINDIR}" DIRECTORY)

if(WIN32)
    set(julia_old_CMAKE_FIND_LIBRARY_SUFFIXES "")
    set(julia_old_CMAKE_FIND_LIBRARY_PREFIXES "")
    if(CMAKE_FIND_LIBRARY_SUFFIXES)
        set(julia_old_CMAKE_FIND_LIBRARY_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
        list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .dll.a)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.dll.a")
    endif()
    if(CMAKE_FIND_LIBRARY_PREFIXES)
        set(julia_old_CMAKE_FIND_LIBRARY_PREFIXES "${CMAKE_FIND_LIBRARY_PREFIXES}")
        list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
    else()
        set(CMAKE_FIND_LIBRARY_PREFIXES ";lib")
    endif()
endif()

# detect Julia library
find_library(JULIA_LIBRARY julia HINTS "${JULIA_PATH_PREFIX}/lib")

if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${julia_old_CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_PREFIXES "${julia_old_CMAKE_FIND_LIBRARY_PREFIXES}")
endif()

julia_bail_if_false("Unable to find the julia shared library. Make sure JULIA_BINDIR is set correctly and that the julia image is uncompressed" JULIA_LIBRARY)

# detect Julia include dir
find_path(
    JULIA_INCLUDE_DIR julia.h
    HINTS "${JULIA_PATH_PREFIX}/include" "${JULIA_PATH_PREFIX}/include/julia"
)
julia_bail_if_false("Unable to find julia.h. Make sure JULIA_BINDIR is set correctly and that your image is uncompressed." JULIA_INCLUDE_DIR)

# detect Julia version
if(NOT DEFINED JULIA_VERSION)
    file(STRINGS "${JULIA_INCLUDE_DIR}/julia_version.h" JULIA_VERSION_LOCAL LIMIT_COUNT 1 REGEX JULIA_VERSION_STRING)
    string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1" JULIA_VERSION_LOCAL "${JULIA_VERSION_LOCAL}")
    set(JULIA_VERSION "${JULIA_VERSION_LOCAL}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Julia
    REQUIRED_VARS JULIA_LIBRARY JULIA_EXECUTABLE JULIA_BINDIR JULIA_INCLUDE_DIR
    VERSION_VAR JULIA_VERSION
)

# detect target properties
if(NOT TARGET Julia::Julia)
    set(julia_has_implib NO)
    set(julia_library_type STATIC)
    if(JULIA_LIBRARY MATCHES "\\.(so|dylib)$")
        set(julia_library_type SHARED)
    elseif(JULIA_LIBRARY MATCHES "\\.(lib|dll\\.a)$")
        set(julia_library_type UNKNOWN)
        find_file(
            JULIA_LIBRARY_DLL
            NAMES libjulia.dll julia.dll
            HINTS "${JULIA_BINDIR}"
        )
        if(JULIA_LIBRARY_DLL)
          set(julia_has_implib YES)
          set(julia_library_type SHARED)
        endif()
    endif()

    add_library(Julia::Julia "${julia_library_type}" IMPORTED)
    set_target_properties(
        Julia::Julia PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${JULIA_INCLUDE_DIR}"
        IMPORTED_LINK_INTERFACE_LANGUAGES C
    )
    if(julia_has_implib)
        if(JULIA_LIBRARY AND EXISTS "${JULIA_LIBRARY}")
            set_property(TARGET Julia::Julia PROPERTY IMPORTED_IMPLIB "${JULIA_LIBRARY}")
        endif()
        if(JULIA_LIBRARY_DLL AND EXISTS "${JULIA_LIBRARY_DLL}")
            set_property(TARGET Julia::Julia PROPERTY IMPORTED_LOCATION "${JULIA_LIBRARY_DLL}")
        endif()
    elseif(JULIA_LIBRARY AND EXISTS "${JULIA_LIBRARY}")
        set_property(TARGET Julia::Julia PROPERTY IMPORTED_LOCATION "${JULIA_LIBRARY}")
    endif()
endif()

# finish
mark_as_advanced(JULIA_EXECUTABLE JULIA_BINDIR JULIA_LIBRARY JULIA_INCLUDE_DIR JULIA_VERSION JULIA_LIBRARY_DLL)
