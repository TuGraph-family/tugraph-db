# tcmalloc
option(LINK_TCMALLOC "Link tcmalloc if possible" ON)
if (LINK_TCMALLOC)
    find_path(GPERFTOOLS_INCLUDE_DIR NAMES gperftools/heap-profiler.h)
    find_library(GPERFTOOLS_LIBRARIES NAMES tcmalloc_and_profiler)
    if (GPERFTOOLS_INCLUDE_DIR AND GPERFTOOLS_LIBRARIES)
        set(CMAKE_CXX_FLAGS "-DBRPC_ENABLE_CPU_PROFILER")
        set(FMA_GPERFTOOLS_INCLUDE_DIR ${GPERFTOOLS_INCLUDE_DIR})
    else ()
        set(GPERFTOOLS_LIBRARIES "")
    endif ()
endif ()

# gflags
include(cmake/FindGFlags.cmake)

# leveldb
find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif ()

# openssl for cpprest
find_package(OpenSSL)

# protbuf
include(cmake/GenerateProtobuf.cmake)
GenerateProtobufCpp(${CMAKE_CURRENT_LIST_DIR}/protobuf
        PROTO_SRCS PROTO_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/protobuf/ha.proto)

include_directories(${DEPS_INCLUDE_DIR})

# brpc
set(BRPC_LIB libbrpc.a)

add_library(lgraph_server_lib STATIC
        plugin/plugin_context.cpp
        plugin/python_plugin.cpp
        plugin/cpp_plugin.cpp
        server/lgraph_server.cpp
        server/state_machine.cpp
        import/import_online.cpp
        import/import_v2.cpp
        restful/server/rest_server.cpp
        restful/server/stdafx.cpp)

target_compile_options(lgraph_server_lib PUBLIC
        -DGFLAGS_NS=${GFLAGS_NS}
        -D__const__=
        -pipe
        #-W -Wall -Wno-unused-parameter
        -fPIC -fno-omit-frame-pointer)

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
    target_link_libraries(lgraph_server_lib
            PUBLIC
            lgraph
            lgraph_cypher_lib
            # begin static linking
            -Wl,-Bstatic
            cpprest
            ${BRPC_LIB}
            ${PROTOBUF_LIBRARY}
            ${GFLAGS_LIBRARY}
            ${GPERFTOOLS_LIBRARIES}
            ${LEVELDB_LIB}
            snappy
            # end static linking
            libssl.a
            libcrypto.a
            -Wl,-Bdynamic
            krb5
            k5crypto
            dl
            c
            )
else ()
    target_link_libraries(lgraph_server_lib
            PUBLIC
            lgraph
            lgraph_cypher_lib
            ${BRPC_LIB}
            ${LEVELDB_LIB}
            ${PROTOBUF_LIBRARY}
            "-framework CoreFoundation"
            "-framework CoreGraphics"
            "-framework CoreData"
            "-framework CoreText"
            "-framework Security"
            "-framework Foundation"
            -Wl,-U,_MallocExtension_ReleaseFreeMemory
            -Wl,-U,_ProfilerStart
            -Wl,-U,_ProfilerStop
            gflags
            cpprest
            boost_thread
            boost_chrono
            profiler
            snappy
            pthread
            ssl
            z
            )
endif ()

add_executable(lgraph_server
        server/server_main.cpp)

target_link_libraries(lgraph_server
        lgraph_server_lib)


