cmake_minimum_required(VERSION 3.5)

set(VCPKG_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../vcpkg"
    CACHE PATH "Vcpkg install folder")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
mark_as_advanced(CMAKE_TOOLCHAIN_FILE)

if(MSVC)
  # warning level 4 and all warnings as errors
  add_compile_options(/W4 /WX)
else()
  # lots of warnings and all warnings as errors
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
add_compile_definitions("PROJECT_ROOT=\"${PROJECT_ROOT}\"")
if(${DISABLE_TERM_HYPERLINK})
  add_compile_definitions("DISABLE_TERM_HYPERLINK")
endif()

project(
  BurningLibrary
  VERSION 0.1
  LANGUAGES C)

# ##############################################################################
add_compile_definitions(BURN_LIB_COMPILING)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.c")
add_library(canaan_burn SHARED ${PROJECT_SOURCES})

set_target_properties(canaan_burn PROPERTIES LINKER_LANGUAGE C)

file(GLOB_RECURSE PUBLIC_HEADER "include/canaan-burn/*.h")
set_target_properties(canaan_burn PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER}")
target_include_directories(canaan_burn INTERFACE "include")

target_include_directories(canaan_burn PRIVATE "include/canaan-burn")
target_include_directories(canaan_burn PRIVATE "src/lib")
# ##############################################################################

find_package(libusb CONFIG REQUIRED)
target_include_directories(canaan_burn PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(canaan_burn PRIVATE ${LIBUSB_LIBRARIES})

include("${VCPKG_INSTALLED_DIR}/x64-linux/share/libsercomm/sercommConfig.cmake")
# find_package(sercomm CONFIG REQUIRED)
target_link_libraries(canaan_burn PRIVATE sercomm)
