macro(julia_bail_if_false message var)
  if(NOT ${var})
    set(Julia_FOUND 0)
    set(Julia_NOT_FOUND_MESSAGE "${message}")
    return()
  endif()
endmacro()

# https://docs.julialang.org/en/v1/manual/environment-variables/#JULIA_BINDIR
find_program(JULIA_EXECUTABLE julia PATHS ENV JULIA_BINDIR)
julia_bail_if_false("The Julia executable could not be found" JULIA_EXECUTABLE)

# The executable could be a chocolatey shim, so run some Julia code to report
# the path of the BINDIR
execute_process(
    COMMAND "${JULIA_EXECUTABLE}" -e "print(Sys.BINDIR)"
    OUTPUT_VARIABLE julia_bindir
)
file(TO_CMAKE_PATH "${julia_bindir}" julia_bindir)
set(JULIA_BINDIR "${julia_bindir}" CACHE PATH "")
julia_bail_if_false("The Julia executable could not report its location" JULIA_BINDIR)

get_filename_component(julia_prefix "${JULIA_BINDIR}" DIRECTORY)

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

find_library(JULIA_LIBRARY julia HINTS "${julia_prefix}/lib")

if(WIN32)
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${julia_old_CMAKE_FIND_LIBRARY_SUFFIXES}")
  set(CMAKE_FIND_LIBRARY_PREFIXES "${julia_old_CMAKE_FIND_LIBRARY_PREFIXES}")
endif()

julia_bail_if_false("The Julia library could not be found" JULIA_LIBRARY)

find_path(
    JULIA_INCLUDE_DIR julia.h
    HINTS "${julia_prefix}/include" "${julia_prefix}/include/julia"
)
julia_bail_if_false("The Julia header could not be found" JULIA_INCLUDE_DIR)

file(STRINGS "${JULIA_INCLUDE_DIR}/julia_version.h" julia_version LIMIT_COUNT 1 REGEX JULIA_VERSION_STRING)
string(REGEX REPLACE ".*\"([^\"]+)\".*" "\\1" julia_version "${julia_version}")
set(JULIA_VERSION "${julia_version}" CACHE STRING "Version of Julia")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Julia
    REQUIRED_VARS JULIA_EXECUTABLE JULIA_BINDIR JULIA_LIBRARY JULIA_INCLUDE_DIR
    VERSION_VAR JULIA_VERSION
)

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

mark_as_advanced(JULIA_EXECUTABLE JULIA_BINDIR JULIA_LIBRARY JULIA_INCLUDE_DIR JULIA_VERSION JULIA_LIBRARY_DLL)
