set(LGRAPH_VERSION_MAJOR 5)
set(LGRAPH_VERSION_MINOR 0)
set(LGRAPH_VERSION_PATCH 0)

# Detect build type, fallback to release and throw a warning if use didn't specify any
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif (NOT CMAKE_BUILD_TYPE)
message(STATUS "CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

# Address Sanitizer
option(ENABLE_ASAN "Enable Address Sanitizer." OFF)
if (ENABLE_ASAN)
    message(STATUS "Address Sanitizer is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libasan")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -static-libasan ")
else (ENABLE_ASAN)
    message(STATUS "Address Sanitizer is disabled.")
endif (ENABLE_ASAN)

# check c++17
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
if (COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else ()
    message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif ()

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-unused-variable")
endif()
