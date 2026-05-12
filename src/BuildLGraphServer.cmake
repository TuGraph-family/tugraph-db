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

# protbuf
include(cmake/GenerateProtobuf.cmake)
GenerateProtobufCpp(${CMAKE_CURRENT_LIST_DIR}/protobuf
        PROTO_SRCS PROTO_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/protobuf/ha.proto
        ${CMAKE_CURRENT_LIST_DIR}/protobuf/tugraph_db_management.proto)

include_directories(${DEPS_INCLUDE_DIR})

# brpc
set(BRPC_LIB libbrpc.a)
set(BRAFT_LIB libbraft.a)

############### liblgraph_server_lib ######################

set(TARGET_SERVER_LIB lgraph_server_lib)

add_library(${TARGET_SERVER_LIB} STATIC
        plugin/plugin_context.cpp
        plugin/python_plugin.cpp
        plugin/cpp_plugin.cpp
        server/bolt_handler.cpp
        server/bolt_server.cpp
        server/bolt_raft_server.cpp
        server/lgraph_server.cpp
        server/state_machine.cpp
        server/ha_state_machine.cpp
        server/db_management_client.cpp
        import/import_online.cpp
        import/import_v2.cpp
        import/import_v3.cpp
        restful/server/rest_server.cpp
        restful/server/stdafx.cpp
        http/http_server.cpp
        http/import_manager.cpp
        http/import_task.cpp
        http/algo_task.cpp
        ${PROTO_SRCS})

if (OURSYSTEM STREQUAL "centos9")
        target_compile_options(${TARGET_SERVER_LIB} PUBLIC
        -DGFLAGS_NS=${GFLAGS_NS}
        -D__const__=__unused__
        -pipe
        -fPIC -fno-omit-frame-pointer)
else()
        target_compile_options(${TARGET_SERVER_LIB} PUBLIC
        -DGFLAGS_NS=${GFLAGS_NS}
        -D__const__=
        -pipe
        -fPIC -fno-omit-frame-pointer)
endif()

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
    target_link_libraries(${TARGET_SERVER_LIB}
            PUBLIC
            lgraph
            lgraph_cypher_lib
            geax_isogql
            bolt
            -laio
            /usr/local/lib64/libvsag_static.a
            /usr/local/lib64/libvsag_mockimpl.a
            libdiskann.a
            libcpuinfo.a
            libsimd.a
            libroaring.a
            /opt/OpenBLAS/lib/libopenblas.a
            faiss
            # begin static linking
            -Wl,-Bstatic
            cpprest
            ${BRAFT_LIB}
            ${BRPC_LIB}
            ${PROTOBUF_LIBRARY}
            ${GFLAGS_LIBRARY}
            ${GPERFTOOLS_LIBRARIES}
            ${LEVELDB_LIB}
            snappy
            OpenSSL::ssl
            OpenSSL::crypto
            # end static linking
            -Wl,-Bdynamic
            dl
            c
            )
else ()
    target_link_libraries(${TARGET_SERVER_LIB}
            PUBLIC
            lgraph
            lgraph_cypher_lib
            ${BRAFT_LIB}
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
            OpenSSL::ssl
            OpenSSL::crypto
            z
            )
endif ()

############### lgraph_server ######################

set(TARGET_SERVER lgraph_server)

add_executable(${TARGET_SERVER}
        server/server_main.cpp)

target_link_libraries(${TARGET_SERVER}
        ${TARGET_SERVER_LIB}
        librocksdb.a
        -laio
        /usr/local/lib64/libvsag_static.a
        /usr/local/lib64/libvsag_mockimpl.a
        libdiskann.a
        libcpuinfo.a
        libsimd.a
        libroaring.a
        /opt/OpenBLAS/lib/libopenblas.a
        faiss
)
