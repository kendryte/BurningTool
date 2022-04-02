cmake_minimum_required(VERSION 3.5)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(
  BurningTool
  VERSION 0.1
  LANGUAGES CXX)

find_package(
  QT NAMES Qt6
  COMPONENTS Widgets LinguistTools
  REQUIRED)
find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Widgets LinguistTools
  REQUIRED)

set(TS_FILES src/qt_zh_CN.ts)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp" "src/*.h" "src/*.ui")

qt_add_executable(BurningTool MANUAL_FINALIZATION ${PROJECT_SOURCES})

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

target_include_directories(BurningTool PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src")

target_link_libraries(BurningTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(
  BurningTool
  PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING
             ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
             MACOSX_BUNDLE TRUE
             WIN32_EXECUTABLE TRUE)

qt_finalize_executable(BurningTool)

target_include_directories(BurningTool PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/BurningTool_autogen/include")

if(NOT DEFINED BUILD_PARENT)
  include("${CMAKE_TOOLCHAIN_FILE}")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../library"
                   "${CMAKE_CURRENT_BINARY_DIR}/library.dist")
endif()

add_dependencies(BurningTool canaan_burn)
target_link_libraries(BurningTool PRIVATE canaan_burn)
