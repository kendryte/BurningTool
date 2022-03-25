cmake_minimum_required(VERSION 3.5)

set(VCPKG_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../vcpkg"
    CACHE PATH "Vcpkg install folder")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
mark_as_advanced(CMAKE_TOOLCHAIN_FILE)

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
if(WIN32)
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.macos\\.c$")
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.linux\\.c$")
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.posix\\.c$")
elseif(APPLE)
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.win32\\.c$")
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.linux\\.c$")
elseif(UNIX)
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.win32\\.c$")
  list(FILTER PROJECT_SOURCES EXCLUDE REGEX "\\.macos\\.c$")
else()
  message(FATAL_ERROR "不支持当前平台")
endif()
add_library(canaan_burn SHARED "${PROJECT_SOURCES}")
set_property(TARGET canaan_burn PROPERTY C_STANDARD 11)

macro(load_binary target variable src)
  file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_LIST_DIR}" "${src}")
  set(tgt "${CMAKE_CURRENT_BINARY_DIR}/${rel_path}.generate.c")
  set(tgt_hdr "${CMAKE_CURRENT_BINARY_DIR}/ginclude/${variable}.h")

  if("${src}" IS_NEWER_THAN "${tgt}")
    file(READ "${src}" content HEX)
    file(SIZE "${src}" size)

    string(REGEX REPLACE "[0-9a-f][0-9a-f]" "0x\\0," content_rep "${content}")

    file(
      WRITE "${tgt}"
      "#include <stdint.h>\n#include <stddef.h>\nconst uint8_t ${variable}[${size}] = {\n${content_rep}\n};\nconst size_t ${variable}_size = ${size};"
    )
    file(
      WRITE "${tgt_hdr}"
      "#include <stdint.h>\n#include <stddef.h>\nextern const uint8_t ${variable}[${size}];\nextern const size_t ${variable}_size;"
    )
  endif()

  target_include_directories("${target}"
                             PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/ginclude")
  target_sources("${target}" PRIVATE "${tgt}")
endmacro()
load_binary(canaan_burn "usb_isp_buffer" "${CMAKE_CURRENT_LIST_DIR}/usbisp.bin")

set_target_properties(canaan_burn PROPERTIES LINKER_LANGUAGE C)

file(GLOB_RECURSE PUBLIC_HEADER "include/canaan-burn/*.h")
set_target_properties(canaan_burn PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER}")
target_include_directories(canaan_burn INTERFACE "include")

file(GLOB_RECURSE new_list *.h)
set(dir_list "")
foreach(file_path ${new_list})
  get_filename_component(dir_path ${file_path} PATH)
  set(dir_list "${dir_list}" ${dir_path})
endforeach()
list(REMOVE_DUPLICATES dir_list)
target_include_directories(canaan_burn PRIVATE "${dir_list}")
# ##############################################################################

find_package(libusb CONFIG REQUIRED)
target_include_directories(canaan_burn PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(canaan_burn PRIVATE ${LIBUSB_LIBRARIES})

if(WIN32)
	include("${VCPKG_INSTALLED_DIR}/x64-windows/share/libsercomm/sercommConfig.cmake")
elseif(UNIX)
	include("${VCPKG_INSTALLED_DIR}/x64-linux/share/libsercomm/sercommConfig.cmake")
elseif(APPLE)
# todo
endif()
# find_package(sercomm CONFIG REQUIRED)
target_link_libraries(canaan_burn PRIVATE sercomm)

include("${CMAKE_CURRENT_LIST_DIR}/3rdparty/slip/CMakeLists.txt")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(SANITIZE_ADDRESS "On")
  # set(SANITIZE_MEMORY "On")

  # set(SANITIZE_THREAD "On")

  # set(SANITIZE_UNDEFINED "On")

  set(SANITIZE_LINK_STATIC "On")

  set(CMAKE_MODULE_PATH
      "${CMAKE_CURRENT_LIST_DIR}/3rdparty/sanitizers-cmake/cmake"
      ${CMAKE_MODULE_PATH})
  find_package(Sanitizers REQUIRED)

  add_sanitizers(canaan_burn)

  if(MSVC)
    # warning level 4 and all warnings as errors
    target_compile_options(canaan_burn PRIVATE /W4 /WX)
  else()
    # lots of warnings and all warnings as errors
    target_compile_options(canaan_burn PRIVATE -Wall -Wextra -pedantic -Werror)
  endif()
endif()
