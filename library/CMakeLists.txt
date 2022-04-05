cmake_minimum_required(VERSION 3.5)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
add_compile_definitions("PROJECT_ROOT=\"${PROJECT_ROOT}\"")
add_compile_definitions("KBURN_ASSERT_SIZE")

if(${DISABLE_TERM_HYPERLINK})
	add_compile_definitions("DISABLE_TERM_HYPERLINK")
endif()

project(
	BurningLibrary
	VERSION 0.1
	LANGUAGES C)

# ##############################################################################
add_compile_definitions(BURN_LIB_COMPILING)

function(push_source_code target src_dir)
	file(GLOB_RECURSE NEW_SOURCES "src/${src_dir}/*.c")

	if(WIN32)
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.macos\\.c$")
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.linux\\.c$")
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.posix\\.c$")
	elseif(APPLE)
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.win32\\.c$")
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.linux\\.c$")
	elseif(UNIX)
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.win32\\.c$")
		list(FILTER NEW_SOURCES EXCLUDE REGEX "\\.macos\\.c$")
	else()
		message(FATAL_ERROR "不支持当前平台")
	endif()

	target_sources("${target}" PRIVATE "${NEW_SOURCES}")

	set(inc_dirs "${CMAKE_CURRENT_LIST_DIR}/src/${src_dir}")

	foreach(dir IN LISTS ARGN)
		list(APPEND inc_dirs "${CMAKE_CURRENT_LIST_DIR}/src/${dir}")
	endforeach()

	foreach(file IN LISTS NEW_SOURCES)
		set_source_files_properties("${file}" PROPERTIES INCLUDE_DIRECTORIES "${inc_dirs}")
	endforeach()
endfunction()

function(load_binary target variable src)
	file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_LIST_DIR}" "${src}")
	set(tgt "${CMAKE_CURRENT_BINARY_DIR}/${rel_path}.generate.c")
	set(tgt_hdr "${CMAKE_CURRENT_BINARY_DIR}/gen-include/generated.${variable}.h")

	if("${src}" IS_NEWER_THAN "${tgt}")
		file(READ "${src}" content HEX)
		file(SIZE "${src}" size)

		string(REGEX REPLACE "[0-9a-f][0-9a-f]" "0x\\0," content_rep "${content}")

		file(GENERATE
			WRITE "${tgt}"
			"#include <stdint.h>\n#include <stddef.h>\nconst uint8_t ${variable}[${size}] = {\n${content_rep}\n};\nconst size_t ${variable}_size = ${size};"
		)
		file(
			WRITE "${tgt_hdr}"
			"#include <stdint.h>\n#include <stddef.h>\nextern const uint8_t ${variable}[${size}];\nextern const size_t ${variable}_size;"
		)
	endif()

	target_include_directories("${target}"
		PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/gen-include")
	target_sources("${target}" PRIVATE "${tgt}")
endfunction()

add_library(canaan_burn SHARED)
set_property(TARGET canaan_burn PROPERTY C_STANDARD 17)
set_property(TARGET canaan_burn PROPERTY LINKER_LANGUAGE C)

load_binary(canaan_burn "usb_isp_buffer" "${CMAKE_CURRENT_LIST_DIR}/usbisp.bin")

file(GLOB_RECURSE PUBLIC_HEADER "include/canaan-burn/*.h")
set_target_properties(canaan_burn PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADER}")

push_source_code(canaan_burn lib)
push_source_code(canaan_burn usb lib)
push_source_code(canaan_burn serial lib)
push_source_code(canaan_burn bridge lib)

target_include_directories(canaan_burn
	PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

target_include_directories(
	canaan_burn PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include/canaan-burn/parts")

# ##############################################################################
find_package(libusb CONFIG REQUIRED)
target_include_directories(canaan_burn PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(canaan_burn PRIVATE ${LIBUSB_LIBRARIES})

if(WIN32)
	include(
		"${VCPKG_INSTALLED_DIR}/x64-windows/share/libsercomm/sercommConfig.cmake")
elseif(UNIX)
	include(
		"${VCPKG_INSTALLED_DIR}/x64-linux/share/libsercomm/sercommConfig.cmake")
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
	set(SANITIZE_LINK_STATIC "Off")

	set(CMAKE_MODULE_PATH
		"${CMAKE_CURRENT_LIST_DIR}/3rdparty/sanitizers-cmake/cmake"
		${CMAKE_MODULE_PATH})
	find_package(Sanitizers REQUIRED)

	add_sanitizers(canaan_burn)
endif()

target_compile_options(canaan_burn PRIVATE -Wall -Wextra -pedantic -Werror
) # -Wno-error=format-zero-length=0

target_compile_definitions(canaan_burn PRIVATE _GNU_SOURCE)
