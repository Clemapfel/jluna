#
# Install rules for jluna/CMakeLists.txt
#

# find_package(<package>) call for consumers to find this project
set(package jluna)

if(PROJECT_IS_TOP_LEVEL)
    set(CMAKE_INSTALL_INCLUDEDIR "include/${package}" CACHE PATH "")
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    FILES jluna.hpp
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT jluna_Development
)

install(
    DIRECTORY include .src
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT jluna_Development
)

install(
    TARGETS jluna
    EXPORT jluna-targets
    RUNTIME #
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    COMPONENT jluna_Runtime
    LIBRARY #
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT jluna_Runtime
    NAMELINK_COMPONENT jluna_Development
    ARCHIVE #
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT jluna_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}-config-version.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    jluna_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(jluna_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${jluna_INSTALL_CMAKEDIR}"
    RENAME "${package}-config.cmake"
    COMPONENT jluna_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}-config-version.cmake"
    DESTINATION "${jluna_INSTALL_CMAKEDIR}"
    COMPONENT jluna_Development
)

install(
    EXPORT jluna-targets
    NAMESPACE jluna::
    DESTINATION "${jluna_INSTALL_CMAKEDIR}"
    COMPONENT jluna_Development
)

if(PROJECT_IS_TOP_LEVEL)
    include(CPack)
endif()
