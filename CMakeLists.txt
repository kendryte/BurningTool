cmake_minimum_required(VERSION 3.5)

set(VCPKG_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/vcpkg"
    CACHE PATH "Vcpkg install folder")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
mark_as_advanced(CMAKE_TOOLCHAIN_FILE)

project(BurningToolMeta)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/library)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cli)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/gui)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test-binary)
