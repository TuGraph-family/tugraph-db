cmake_minimum_required(VERSION 3.16)
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

add_definitions(-DGAR_NAMESPACE=GraphArchive)

include_directories(SYSTEM ${DEPS_INCLUDE_DIR} ${DEPS_LOCAL_INCLUDE_DIR}) # just for IDE
link_directories(${DEPS_LIBRARY_DIR} ${DEPS_LIBRARY64_DIR})
if (OURSYSTEM STREQUAL "centos")
    link_directories(${DEPS_LIBRARY64_DIR})
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    link_directories(${DEPS_LOCAL_LIBRARY_DIR})
endif ()

# output
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output)

# target
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(toolkits)
add_subdirectory(plugins)
add_subdirectory(release)
add_subdirectory(learn)

# fix dependency
set(LGRAPH_TOOLKITS lgraph_import lgraph_backup lgraph_warmup lgraph_peek lgraph_export lgraph_binlog)
add_dependencies(unit_test ${LGRAPH_TOOLKITS} lgraph_server)
