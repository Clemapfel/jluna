include("${CMAKE_CURRENT_LIST_DIR}/jluna-targets.cmake")

# set JLUNA_INCLUDE_DIRECTORIES
get_target_property(JLUNA_INCLUDE_DIRECTORIES jluna::jluna INTERFACE_INCLUDE_DIRECTORIES)

# set JLUNA_LIBRARIES
get_target_property(JLUNA_LIBRARIES jluna::jluna INTERFACE_LINK_LIBRARIES)
set(JLUNA_LIBRARIES "jluna::jluna;${JLUNA_LIBRARIES}")

# set JULIA_LIBRARY_PATH, needed for meson
get_target_property(JULIA_LIBRARY_PATH jluna::jluna INTERFACE_LINK_DIRECTORIES)

# for some reason, this is required for `meson.get_variable` to work
set(JLUNA_INCLUDE_DIRECTORIES "${JLUNA_INCLUDE_DIRECTORIES}")
set(JULIA_LIBRARY_PATH "${JULIA_LIBRARY_PATH}")