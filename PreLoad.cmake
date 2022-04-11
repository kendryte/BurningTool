cmake_policy(PUSH)
cmake_policy(SET CMP0126 NEW)

if(NOT DEFINED CMAKE_GENERATOR)
	if(WIN32)
		set(CMAKE_GENERATOR
			"MinGW Makefiles"
			CACHE INTERNAL "" FORCE)
	else()
		set(CMAKE_GENERATOR
			"Unix Makefiles"
			CACHE INTERNAL "" FORCE)
	endif()
endif()

macro(find_qt_at WHERE)
	get_filename_component(WHERE1 "${WHERE}" ABSOLUTE)
	message("  - ${WHERE1}")

	file(GLOB_RECURSE QT_CFG "${WHERE1}/*/Qt6Config.cmake")
	list(POP_FRONT QT_CFG QT_CFG1)

	if(QT_CFG1)
		message("found QT at ${QT_CFG1}")
		get_filename_component(QT_DIR_V "${QT_CFG1}" DIRECTORY)

		file(WRITE "${CMAKE_BINARY_DIR}/qt.path" "${QT_DIR_V}")

		set(QT_DIR_FOUND "${QT_DIR_V}" PARENT_SCOPE)
		return()
	endif()
endmacro()

function(find_qt)
	message("finding QT install...")

	if(DEFINED QT_QMAKE_EXECUTABLE)
		get_filename_component(QT_MAYBE "${QT_QMAKE_EXECUTABLE}/../.." ABSOLUTE)
		find_qt_at("${QT_MAYBE}")
	else()
		message("  - no QT_QMAKE_EXECUTABLE")
	endif()

	if(EXISTS "${QT_DIR}")
		set(QT_MAYBE "${QT_DIR}")
		find_qt_at("${QT_MAYBE}")
	else()
		message("  - no QT_DIR")
	endif()

	if(EXISTS "${QT_BIN_DIR}")
		get_filename_component(QT_MAYBE "${QT_BIN_DIR}" DIRECTORY)
		find_qt_at("${QT_MAYBE}")
	else()
		message("  - no QT_BIN_DIR")
	endif()

	if(EXISTS "$ENV{QT_INSTALL_DIR}")
		find_qt_at("$ENV{QT_INSTALL_DIR}")
	else()
		message("  - no ENV{QT_INSTALL_DIR}")
	endif()

	if(WIN32)
		find_qt_at("C:/Qt")
	else()
		find_qt_at("/opt/Qt")
	endif()

	if(UNIX AND NOT APPLE)
		find_qt_at("/usr/lib64/cmake")
	endif()

	message(FATAL_ERROR "Can not find QT install path, set QT_INSTALL_DIR")
endfunction()

if(NOT DEFINED CACHE{QT_DIR})
	if(EXISTS "${CMAKE_BINARY_DIR}/qt.path")
		file(READ "${CMAKE_BINARY_DIR}/qt.path" QT_DIR_READ)
	endif()

	if(EXISTS "${QT_DIR_READ}")
		set(QT_DIR_FOUND "${QT_DIR_READ}")
	else()
		find_qt()
	endif()

	set(QT_DIR "${QT_DIR_FOUND}" CACHE PATH "QT library path" FORCE)
	mark_as_advanced(FORCE QT_DIR)
	set(Qt6_DIR "${QT_DIR_FOUND}" CACHE PATH "QT library path" FORCE)
	mark_as_advanced(FORCE Qt6_DIR)

	get_filename_component(QT_BIN_DIR "${QT_DIR_FOUND}/../../../bin" ABSOLUTE)
	get_filename_component(QT_PREFIX_DIR "${QT_BIN_DIR}/.." ABSOLUTE)
	get_filename_component(QT_INSTALL_DIR "${QT_BIN_DIR}/../.." ABSOLUTE)

	set(QT_INSTALL_DIR "${QT_INSTALL_DIR}" CACHE PATH "QT install location" FORCE)
	set(QT_BIN_DIR "${QT_BIN_DIR}" CACHE PATH "QT binary path" FORCE)
	mark_as_advanced(FORCE QT_BIN_DIR)

	list(APPEND CMAKE_PREFIX_PATH "${QT_PREFIX_DIR}")
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" CACHE PATH "" FORCE)

	list(APPEND CMAKE_PROGRAM_PATH "${QT_BIN_DIR}")
endif()

if(NOT DEFINED CMAKE_MAKE_PROGRAM)
	if(WIN32)
		find_program(CMAKE_MAKE_PROGRAM mingw32-make.exe HINTS "C:/msys64/mingw64/bin" "${QT_BIN_DIR}" ENV MINGW REQUIRED)
		set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM}" CACHE FILEPATH "the make.exe location" FORCE)
	else()
		set(CMAKE_MAKE_PROGRAM "make" CACHE FILEPATH "the make location" FORCE)
	endif()
endif()

get_filename_component(MAKE_BIN "${CMAKE_MAKE_PROGRAM}" DIRECTORY)
list(APPEND CMAKE_PROGRAM_PATH "${MAKE_BIN}")
set(CMAKE_PROGRAM_PATH "${CMAKE_PROGRAM_PATH}" CACHE PATH "" FORCE)
mark_as_advanced(FORCE CMAKE_PROGRAM_PATH)

message("CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
message("CMAKE_GENERATOR=${CMAKE_GENERATOR}")

# function(set_exe_path VAR TYPE)
# string(REPLACE "mingw32-make.exe" "${TYPE}.exe" OUTVAL "${CMAKE_MAKE_PROGRAM}")
# set("${VAR}" "${OUTVAL}" CACHE FILEPATH "the ${TYPE} location" FORCE)
# endfunction()
set(DISABLE_TERM_HYPERLINK yes CACHE BOOL "disable terminal escape for files" FORCE)
set(OpenGL_GL_PREFERENCE GLVND CACHE STRING "" FORCE)

set(VCPKG_ROOT
	"${CMAKE_CURRENT_LIST_DIR}/vcpkg"
	CACHE PATH "vcpkg install root" FORCE)
set(CMAKE_TOOLCHAIN_FILE
	"${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
	CACHE PATH "vcpkg toolchain file" FORCE)

set(VCPKG_MANIFEST_DIR
	"${CMAKE_CURRENT_LIST_DIR}"
	CACHE PATH "" FORCE)
message("VCPKG_ROOT=${VCPKG_ROOT}")

set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON CACHE BOOL "X_VCPKG_APPLOCAL_DEPS_INSTALL" FORCE)

cmake_policy(POP)
