cmake_minimum_required(VERSION 3.5)

set(BUILD_PARENT On)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(BurningToolMeta)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/library)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cli)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gui)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test-binary)
