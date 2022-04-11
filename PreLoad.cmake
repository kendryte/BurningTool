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

if(NOT DEFINED CMAKE_MAKE_PROGRAM)
	if(WIN32)
		set(CMAKE_MAKE_PROGRAM
			"mingw32-make.exe"
			CACHE FILEPATH "the make.exe location" FORCE)
	else()
		set(CMAKE_MAKE_PROGRAM
			"make"
			CACHE FILEPATH "the make location" FORCE)
	endif()
endif()

message("CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")
message("CMAKE_GENERATOR=${CMAKE_GENERATOR}")

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
