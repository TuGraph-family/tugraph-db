find_program(CLANG_FORMAT_BIN NAMES clang-format)
find_program(PROJECT_CMAKE_FORMAT_BIN NAMES cmake-format)

set(CLANG_FORMAT_SOURCE_DIRS
    ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/test
    ${PROJECT_SOURCE_DIR}/release ${PROJECT_SOURCE_DIR}/deps)

set(CLANG_FORMAT_FILES)
foreach(clang_format_dir IN LISTS CLANG_FORMAT_SOURCE_DIRS)
  if(EXISTS "${clang_format_dir}")
    file(
      GLOB_RECURSE
      dir_clang_format_files
      CONFIGURE_DEPENDS
      "${clang_format_dir}/*.h"
      "${clang_format_dir}/*.hpp"
      "${clang_format_dir}/*.c"
      "${clang_format_dir}/*.cc"
      "${clang_format_dir}/*.cpp")
    list(APPEND CLANG_FORMAT_FILES ${dir_clang_format_files})
  endif()
endforeach()

if(CLANG_FORMAT_FILES)
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/generated/")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "\\.pb\\.(cc|h)$")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/target/")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/build/")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/out/")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/src/graphdb/ftindex/")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/src/common/version\\.h$")
  list(FILTER CLANG_FORMAT_FILES EXCLUDE REGEX "/src/toolkits/linenoise/")
  list(REMOVE_DUPLICATES CLANG_FORMAT_FILES)
endif()

list(LENGTH CLANG_FORMAT_FILES CLANG_FORMAT_FILE_COUNT)

set(CMAKE_FORMAT_FILES
    ${PROJECT_SOURCE_DIR}/CMakeLists.txt ${PROJECT_SOURCE_DIR}/Options.cmake
    ${PROJECT_SOURCE_DIR}/GenerateVersionInfo.cmake)

set(CMAKE_FORMAT_SOURCE_DIRS ${PROJECT_SOURCE_DIR}/cmake
                             ${PROJECT_SOURCE_DIR}/release)

foreach(cmake_format_dir IN LISTS CMAKE_FORMAT_SOURCE_DIRS)
  if(EXISTS "${cmake_format_dir}")
    file(GLOB_RECURSE dir_cmake_format_files CONFIGURE_DEPENDS
         "${cmake_format_dir}/CMakeLists.txt" "${cmake_format_dir}/*.cmake"
         "${cmake_format_dir}/*.cmake.in")
    list(APPEND CMAKE_FORMAT_FILES ${dir_cmake_format_files})
  endif()
endforeach()

if(CMAKE_FORMAT_FILES)
  list(REMOVE_DUPLICATES CMAKE_FORMAT_FILES)
endif()

list(LENGTH CMAKE_FORMAT_FILES CMAKE_FORMAT_FILE_COUNT)

set(FORMAT_SCRIPT_CONTENT
    [=[
set(CLANG_FORMAT_BIN "@CLANG_FORMAT_BIN@")
set(PROJECT_CMAKE_FORMAT_BIN "@PROJECT_CMAKE_FORMAT_BIN@")
set(CLANG_FORMAT_FILES "@CLANG_FORMAT_FILES@")
set(CLANG_FORMAT_FILE_COUNT "@CLANG_FORMAT_FILE_COUNT@")
set(CMAKE_FORMAT_FILES "@CMAKE_FORMAT_FILES@")
set(CMAKE_FORMAT_FILE_COUNT "@CMAKE_FORMAT_FILE_COUNT@")

if(CLANG_FORMAT_BIN MATCHES "-NOTFOUND$")
  message(FATAL_ERROR "clang-format not found. Please install clang-format and reconfigure.")
endif()

foreach(file IN LISTS CLANG_FORMAT_FILES)
  execute_process(COMMAND "${CLANG_FORMAT_BIN}" -i "${file}"
                  RESULT_VARIABLE clang_format_result)
  if(NOT clang_format_result EQUAL 0)
    message(FATAL_ERROR "clang-format failed for ${file}")
  endif()
endforeach()

message(STATUS "Formatted ${CLANG_FORMAT_FILE_COUNT} files with clang-format")

if(PROJECT_CMAKE_FORMAT_BIN MATCHES "-NOTFOUND$")
  message(FATAL_ERROR "cmake-format not found. Please install cmake-format and reconfigure.")
endif()

foreach(file IN LISTS CMAKE_FORMAT_FILES)
  execute_process(COMMAND "${PROJECT_CMAKE_FORMAT_BIN}" -i "${file}"
                  RESULT_VARIABLE cmake_format_result)
  if(NOT cmake_format_result EQUAL 0)
    message(FATAL_ERROR "cmake-format failed for ${file}")
  endif()
endforeach()

message(STATUS "Formatted ${CMAKE_FORMAT_FILE_COUNT} CMake files with cmake-format")
]=])

string(CONFIGURE "${FORMAT_SCRIPT_CONTENT}" FORMAT_SCRIPT_CONTENT @ONLY)
file(WRITE "${PROJECT_BINARY_DIR}/RunFormat.cmake" "${FORMAT_SCRIPT_CONTENT}")

add_custom_target(
  format
  COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/RunFormat.cmake
  COMMENT "Formatting C/C++ and CMake files"
  VERBATIM)
