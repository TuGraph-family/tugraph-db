cmake_minimum_required(VERSION 3.13)

project(graph-front-end LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()
if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()
if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# output
if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
endif()
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
endif()
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)
endif()

# When Geax is a top-level project, enable the non-library targets by default.
# If it is embedded with `add_subdirectory`, disable them.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set (GEAX_MASTER_PROJECT ON)
else()
    set (GEAX_MASTER_PROJECT OFF)
endif()

# option
option(GEAX_WITH_TESTS "build with tests" OFF)

option(GEAX_ENABLE_GLOG "build with glog" ON)

option(GEAX_ENABLE_JEMALLOC "build with jemalloc" ON)
option(GEAX_ENABLE_TCMALLOC "build with tcmalloc" OFF)

option(GEAX_ENABLE_GFLAGS "build with gflags" ON)

option(GEAX_ENABLE_UNWIND "build with unwind" ON)

option(GEAX_INSTALL "install geax" ${GEAX_MASTER_PROJECT})

option(ENABLE_BUILD_ON_AARCH64 "enable building and linking on aarch64 " OFF)
if (ENABLE_BUILD_ON_AARCH64)
    message(STATUS "Enable compiling on aarch64...")
    add_compile_definitions(
        AARCH64
    )
else()
    add_compile_options(
        -msse4.2
    )
endif()

option(ENABLE_ASAN "Enable Address Sanitizer." OFF)
if (ENABLE_ASAN)
    message(STATUS "Address Sanitizer is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libasan")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -static-libasan ")
else (ENABLE_ASAN)
    message(STATUS "Address Sanitizer is disabled.")
endif (ENABLE_ASAN)

get_directory_property(compile_defs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
if(NOT compile_defs)
    add_compile_definitions(
        ANTLR4_USE_THREAD_LOCAL_CACHE=1
    )
endif()

# remove -Wextra
get_directory_property(compile_opts DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_OPTIONS)
if (compile_opts)
    list(REMOVE_ITEM compile_opts -Wextra)
    set_directory_properties(PROPERTIES COMPILE_OPTIONS "${compile_opts}")
else()
    add_compile_options(
        -pthread
        -Wall
        -Werror
        -Wfatal-errors
        -Wno-noexcept-type
        -fdiagnostics-color=auto
        -Wno-parentheses
    )
endif()

# third-party
if(NOT DEFINED GEAX_THIRD_PARTY_DIR)
    set(GEAX_THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/.third-party CACHE STRING "Choose the third party directory." FORCE)
endif()

# modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)
if (NOT TARGET Threads::Threads)
    find_package(Threads REQUIRED)
endif()
if (GEAX_WITH_TESTS AND (NOT TARGET GTest::gtest))
    find_package(GTest REQUIRED)
endif()
if (GEAX_ENABLE_JEMALLOC AND (NOT TARGET Jemalloc::jemalloc))
    find_package(Jemalloc REQUIRED)
endif()
if (GEAX_ENABLE_GFLAGS AND (NOT TARGET GFlags::gflags))
    find_package(GFlags REQUIRED)
endif()
if (GEAX_ENABLE_GLOG AND (NOT TARGET GLog::glog))
    find_package(GLog REQUIRED)
endif()
if (NOT TARGET Antlr4::runtime)
    find_package(Antlr4 4.13.0 REQUIRED)
endif()

include_directories(${GEAX_THIRD_PARTY_DIR}/include)
include_directories(src)
include_directories(include)
set(GEAX_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# target
add_subdirectory(src)
if (GEAX_WITH_TESTS)
    add_subdirectory(test)
endif()

if(GEAX_INSTALL)
    install(DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/include/geax-front-end
            DESTINATION include)
endif()
