cmake_minimum_required(VERSION 3.13)
project(TuGraph C CXX)

message("Community version.")

# options
include(Options.cmake)

# env
set(LGRAPH_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(DEPS_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/install/include)
set(DEPS_LIBRARY_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/install/lib)
set(DEPS_LIBRARY64_DIR ${CMAKE_CURRENT_LIST_DIR}/deps/install/lib64)
set(LGRAPH_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(CMAKEFILES_COMPILER_DIR ${PROJECT_BINARY_DIR})
set(DEPS_LOCAL_INCLUDE_DIR /usr/local/include)
set(DEPS_LOCAL_LIBRARY_DIR /usr/local/lib)

include_directories(SYSTEM ${DEPS_INCLUDE_DIR} ${DEPS_LOCAL_INCLUDE_DIR}) # just for IDE
link_directories(${DEPS_LIBRARY_DIR} ${DEPS_LIBRARY64_DIR})
if (OURSYSTEM MATCHES "^centos")
    if (OURSYSTEM STREQUAL "centos9")
        add_compile_definitions(USING_CENTOS9=1)
        add_definitions(-D__const__=__unused__)
        message(STATUS "Compiling for OURSYSTEM=centos9: Setting __const__=__unused__")
    else()
        add_compile_definitions(USING_CENTOS9=0)
    endif()

    link_directories(${DEPS_LIBRARY64_DIR})
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    link_directories(${DEPS_LOCAL_LIBRARY_DIR})
endif ()

# output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)

# third-party
if(NOT DEFINED TUGRAPH_THIRD_PARTY_DIR)
    set(TUGRAPH_THIRD_PARTY_DIR "/usr/local;/usr")
endif()
if (NOT DEFINED GEAX_THIRD_PARTY_DIR)
    set(GEAX_THIRD_PARTY_DIR ${TUGRAPH_THIRD_PARTY_DIR})
endif()

# modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(OpenSSL REQUIRED)
# boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.68 REQUIRED COMPONENTS log system filesystem)

# target
set(GEAX_ENABLE_GLOG OFF)
add_subdirectory(deps/geax-front-end)
add_subdirectory(src)
add_subdirectory(toolkits)
add_subdirectory(release)
if (BUILD_PROCEDURE)
    add_subdirectory(learn)
    add_subdirectory(procedures)
endif (BUILD_PROCEDURE)

# unit_test
if (WITH_TESTS)
    add_subdirectory(test)
endif (WITH_TESTS)
