cmake_minimum_required(VERSION 3.5)

if(POLICY CMP0087)
	cmake_policy(SET CMP0087 NEW)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BurningTool VERSION 0.1 LANGUAGES CXX)

find_package(
	Qt6
	COMPONENTS Widgets LinguistTools Network Svg
	REQUIRED)

set(TS_FILES src/qt_zh_CN.ts)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp" "src/*.h" "src/*.ui")
set(PROJECT_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/main.qrc")

if(WIN32)
	enable_language("RC")
	set(WIN32_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/main.rc")
endif()

qt_add_executable(BurningTool MANUAL_FINALIZATION "${PROJECT_SOURCES}" "${WIN32_RESOURCES}" "${PROJECT_RESOURCES}")

qt_create_translation(QM_FILES "${CMAKE_CURRENT_LIST_DIR}/src" ${TS_FILES})

target_include_directories(BurningTool PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

target_link_libraries(BurningTool PRIVATE Qt6::Widgets Qt6::Network Qt6::Svg)

set_target_properties(
	BurningTool
	PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING
	${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE)

# if(WIN32)
# 	target_link_options(BurningTool BEFORE PRIVATE -static-libgcc -static-libstdc++)
# endif()

qt_finalize_executable(BurningTool)

target_include_directories(
	BurningTool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/BurningTool_autogen/include")

if(NOT DEFINED BUILD_PARENT)
	include("${CMAKE_TOOLCHAIN_FILE}")
	add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../library" "${CMAKE_CURRENT_BINARY_DIR}/library.dist")
endif()

add_dependencies(BurningTool canaan_burn)
target_link_libraries(BurningTool PRIVATE canaan_burn)

target_compile_definitions(BurningTool PRIVATE "SETTINGS_CATEGORY=\"kendryte\"")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(BurningTool PRIVATE "IS_DEBUG=0")
else()
	target_compile_definitions(BurningTool PRIVATE "IS_DEBUG=1")
endif()

install(
	TARGETS BurningTool
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

if(NOT DEFINED CACHE{QT_BIN_DIR})
	message(FATAL_ERROR "missing QT_BIN_DIR in cache, need reconfigure")
endif()

if(WIN32)
	install(CODE "
		execute_process(
			COMMAND \"${CMAKE_COMMAND}\"
			\"-DQT_BIN_DIR=${QT_BIN_DIR}\"
			\"-DCMAKE_CACHEFILE_DIR=${CMAKE_BINARY_DIR}\"
			-P \"${CMAKE_CURRENT_LIST_DIR}/win-install.cmake\"
			WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\"
			COMMAND_ECHO STDOUT
			COMMAND_ERROR_IS_FATAL ANY
		)
	")
endif()

set(MY_VERSION $DATE)

if(DEFINED ENV{CI})
	set(IS_CI TRUE)
else()
	set(IS_CI FALSE)
endif()

string(TIMESTAMP MY_VERSION_STRING "%Y-%m-%d.%H:%M")
set(GITHUB_VERSION "$ENV{GITHUB_SHA}")
configure_file("${CMAKE_CURRENT_LIST_DIR}/src/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/autogen/config.h")
target_include_directories(BurningTool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/autogen/")
