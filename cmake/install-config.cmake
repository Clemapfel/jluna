include("${CMAKE_CURRENT_LIST_DIR}/jluna-targets.cmake")

# set JLUNA_INCLUDE_DIRECTORIES
get_target_property(JLUNA_INCLUDE_DIRECTORIES jluna::jluna INTERFACE_INCLUDE_DIRECTORIES)

# set JLUNA_LIBRARIES
get_target_property(JLUNA_LIBRARIES jluna::jluna INTERFACE_LINK_LIBRARIES)
set(JLUNA_LIBRARIES "jluna::jluna;${JLUNA_LIBRARIES}")