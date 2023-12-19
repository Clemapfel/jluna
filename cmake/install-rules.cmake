#
# Install rules for jluna/CMakeLists.txt
#

set(package jluna)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    FILES jluna.hpp
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    DIRECTORY include/jluna
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    TARGETS jluna
    EXPORT jluna-targets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

write_basic_package_version_file(
    "${package}-config-version.cmake"
    COMPATIBILITY AnyNewerVersion
)

# Allow package maintainers to freely override the path for the configs
set(jluna_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(jluna_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${jluna_INSTALL_CMAKEDIR}"
    RENAME "${package}-config.cmake"
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}-config-version.cmake"
    DESTINATION "${jluna_INSTALL_CMAKEDIR}"
)

install(
    EXPORT jluna-targets
    NAMESPACE jluna::
    DESTINATION "${jluna_INSTALL_CMAKEDIR}"
)

include(CPack)
