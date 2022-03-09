cmake_minimum_required(VERSION 3.5)

project(
  KBurningTool
  VERSION 0.1
  LANGUAGES CXX)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.h" "src/*.cpp" "src/*.c")
add_executable(KBurningTool ${PROJECT_SOURCES})

target_include_directories(
  KBurningTool PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../library/include")
set_target_properties(KBurningTool PROPERTIES LINKER_LANGUAGE CXX)

add_dependencies(KBurningTool canaan_burn)
target_link_libraries(KBurningTool PRIVATE canaan_burn)

find_package(Argtable3 CONFIG REQUIRED)
target_link_libraries(KBurningTool PRIVATE argtable3::argtable3)
