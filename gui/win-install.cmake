function(parse_cmake_cache CACHE_FILE)
	file(STRINGS "${CACHE_FILE}" lines REGEX "=")

	foreach(line IN LISTS lines)
		string(FIND "${line}" ":" start_of_type)
		string(FIND "${line}" "=" start_of_assign)

		if(start_of_type EQUAL -1)
			set(start_of_type "${start_of_assign}")
		endif()

		string(SUBSTRING "${line}" 0 ${start_of_type} name)
		math(EXPR start_of_value "${start_of_assign} + 1")
		string(SUBSTRING "${line}" ${start_of_value} -1 value)

		set(${name} "${value}" PARENT_SCOPE)
	endforeach()
endfunction()

parse_cmake_cache("${CMAKE_CACHEFILE_DIR}/CMakeCache.txt")

get_filename_component(DIST_DIR "." ABSOLUTE)
message("RUN ${QT_BIN_DIR}/windeployqt.exe BurningTool.exe IN ${DIST_DIR}/bin")

execute_process(
	COMMAND "${QT_BIN_DIR}/windeployqt.exe" BurningTool.exe
	WORKING_DIRECTORY "${DIST_DIR}/bin"
	COMMAND_ECHO STDOUT
	COMMAND_ERROR_IS_FATAL ANY
)

# TODO
set(MINGW_PATH "C:/msys64/mingw64")
file(INSTALL "${MINGW_PATH}/bin/libwinpthread-1.dll" DESTINATION "${DIST_DIR}/bin")

# /mingw64/bin/libwinpthread-1.dll
# execute_process(
# COMMAND strings.exe
# "BurningTool.exe"
# OUTPUT_FILE "${CMAKE_CACHEFILE_DIR}/strings.txt"
# WORKING_DIRECTORY "${DIST_DIR}/bin"
# COMMAND_ECHO STDOUT
# COMMAND_ERROR_IS_FATAL ANY
# )
# file(STRINGS "${CMAKE_CACHEFILE_DIR}/strings.txt" lines REGEX "\\.dll")

# foreach(line IN LISTS lines)
# message(" !! ${line}")
# endforeach()
