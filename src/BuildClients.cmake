# brpc
set(BRPC_LIB libbrpc.a)

# boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.68 REQUIRED COMPONENTS system filesystem)

if (ENABLE_FULLTEXT_INDEX)
    # jni
    find_package(JNI REQUIRED)
endif ()

# protbuf
include(cmake/GenerateProtobuf.cmake)
GenerateProtobufCpp(${CMAKE_CURRENT_LIST_DIR}/protobuf
        PROTO_SRCS PROTO_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/protobuf/ha.proto)

# leveldb
find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif ()

find_library(SNAPPY NAMES snappy)

set(TARGET_OBJ liblgraph_rpc_client)

add_library(${TARGET_OBJ} SHARED
        client/cpp/rpc/lgraph_rpc_client.cpp
        ${PROTO_SRCS})

add_library(lgraph_rest_client SHARED
        client/cpp/restful/rest_client.cpp
        ${PROTO_SRCS})

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
    target_link_libraries(${TARGET_OBJ}
            PUBLIC
            # begin static linking
            -Wl,-Bstatic
            ${BRPC_LIB}
            ${GFLAGS_LIBRARY}
            ${LEVELDB_LIB}
            -Wl,--whole-archive
            ${PROTOBUF_LIBRARY}
            -Wl,--no-whole-archive
            gflags
            snappy
            libssl.a
            libcrypto.a
            -Wl,-Bstatic
            ${Boost_LIBRARIES}
            -static-libstdc++
            -static-libgcc
            libstdc++fs.a
            -Wl,-Bdynamic
            krb5
            k5crypto
            rt
            dl
            z
            )
else ()
    target_link_libraries(${TARGET_OBJ}
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

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
target_link_libraries(lgraph_rest_client PUBLIC
        lgraph_server_lib
        ${BRPC_LIB}
        boost_system
        boost_filesystem
        ${JAVA_JVM_LIBRARY})
endif()

target_include_directories(${TARGET_OBJ} PRIVATE
        ${DEPS_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cypher    # for FieldDataConvert
        ${LGRAPH_INCLUDE_DIR}
        ${JNI_INCLUDE_DIRS})
set_target_properties(${TARGET_OBJ} PROPERTIES PREFIX "")

target_include_directories(lgraph_rest_client PRIVATE
        ${DEPS_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cypher
        ${LGRAPH_INCLUDE_DIR}
        ${JNI_INCLUDE_DIRS})
