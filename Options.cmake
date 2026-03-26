set(LGRAPH_VERSION_MAJOR 5)
set(LGRAPH_VERSION_MINOR 0)
set(LGRAPH_VERSION_PATCH 0)

# Detect build type, fallback to release and throw a warning if use didn't specify any
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose the type of build." FORCE
  )
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  set(COMPILE_COMMANDS_LINK "${CMAKE_SOURCE_DIR}/compile_commands.json")
  if(IS_SYMLINK "${COMPILE_COMMANDS_LINK}")
    file(REMOVE "${COMPILE_COMMANDS_LINK}")
  endif()

  if(NOT EXISTS "${COMPILE_COMMANDS_LINK}")
    execute_process(
      COMMAND
        "${CMAKE_COMMAND}" -E create_symlink "${CMAKE_BINARY_DIR}/compile_commands.json"
        "${COMPILE_COMMANDS_LINK}"
      RESULT_VARIABLE CREATE_COMPILE_COMMANDS_LINK_RESULT
      ERROR_VARIABLE CREATE_COMPILE_COMMANDS_LINK_ERROR
    )
    if(NOT CREATE_COMPILE_COMMANDS_LINK_RESULT EQUAL 0)
      message(
        WARNING
          "Failed to create compile_commands.json symlink at ${COMPILE_COMMANDS_LINK}: ${CREATE_COMPILE_COMMANDS_LINK_ERROR}"
      )
    endif()
  endif()
endif()

# check OpenMP
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fopenmp>)

# Address Sanitizer
option(ENABLE_ASAN "Enable Address Sanitizer." OFF)
if(ENABLE_ASAN)
  message(STATUS "Address Sanitizer is enabled.")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libasan"
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -static-libasan ")
else(ENABLE_ASAN)
  message(STATUS "Address Sanitizer is disabled.")
endif(ENABLE_ASAN)

# check c++17
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
if(COMPILER_SUPPORTS_CXX17)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
  message(
    SEND_ERROR
      "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler."
  )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-unused-variable")
endif()
