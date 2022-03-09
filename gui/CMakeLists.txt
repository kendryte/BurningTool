cmake_minimum_required(VERSION 3.5)

if(DEFINED ENV{VCPKG_ROOT})
	set(VCPKG_ROOT "$ENV{VCPKG_ROOT}"
		CACHE PATH "Vcpkg install folder" FORCE)
elseif(DEFINED VCPKG_ROOT)
	set(VCPKG_ROOT "$VCPKG_ROOT"
		CACHE PATH "Vcpkg install folder")
elseif(EXISTS C:/src/vcpkg/scripts/buildsystems/vcpkg.cmake)
	set(VCPKG_ROOT "C:/src/vcpkg"
		CACHE PATH "Vcpkg install folder" FORCE)
else()
	message(FATAL_ERROR "没有找到vcpkg，需要设置VCPKG_ROOT环境变量，或在运行cmake时设置-DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake")
endif()
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
mark_as_advanced(CMAKE_TOOLCHAIN_FILE)

project(BurningTool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)
find_package(libusb CONFIG REQUIRED)

set(TS_FILES src/qt_zh_CN.ts)

set(PROJECT_SOURCES
	src/main.cpp
	src/MainWindow.cpp
	src/test-list.cpp
	src/MainWindow.h
	src/MainWindow.ui
	${TS_FILES}
	)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(BurningTool
		MANUAL_FINALIZATION
		${PROJECT_SOURCES}
		)
	# Define target properties for Android with Qt 6 as:
	#    set_property(TARGET BurningTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
	#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
	# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

	qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
	if(ANDROID)
		add_library(BurningTool SHARED
			${PROJECT_SOURCES}
			)
		# Define properties for Android with Qt 5 after find_package() calls as:
		#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
	else()
		add_executable(BurningTool
			${PROJECT_SOURCES}
			)
	endif()

	qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(BurningTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(BurningTool PRIVATE ${LIBUSB_INCLUDE_DIRS})
target_link_libraries(BurningTool PRIVATE ${LIBUSB_LIBRARIES})

set_target_properties(BurningTool PROPERTIES
	MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
	)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(BurningTool)
endif()
