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

find_package(PythonLibs 3 REQUIRED)

############### liblgraph_client_cpp_rpc ######################

set(TARGET_CPP_CLIENT_RPC lgraph_client_cpp_rpc)

add_library(${TARGET_CPP_CLIENT_RPC} SHARED
        client/cpp/rpc/lgraph_rpc_client.cpp
        ${PROTO_SRCS})

target_include_directories(${TARGET_CPP_CLIENT_RPC} PRIVATE
        ${DEPS_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cypher    # for FieldDataConvert
        ${LGRAPH_INCLUDE_DIR}
        ${JNI_INCLUDE_DIRS})

if (NOT (CMAKE_SYSTEM_NAME STREQUAL "Darwin"))
    target_link_libraries(${TARGET_CPP_CLIENT_RPC}
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
            -Wl,-Bstatic
            ${Boost_LIBRARIES}
            -static-libstdc++
            -static-libgcc
            libstdc++fs.a
            -Wl,-Bdynamic
            ssl
            crypto
            rt
            dl
            z
            )
else ()
    target_link_libraries(${TARGET_CPP_CLIENT_RPC}
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

############### liblgraph_client_cpp_rest ######################

set(TARGET_CPP_CLIENT_REST lgraph_client_cpp_rest)

add_library(${TARGET_CPP_CLIENT_REST} SHARED
        client/cpp/restful/rest_client.cpp
        ${PROTO_SRCS})

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
target_link_libraries(${TARGET_CPP_CLIENT_REST} PUBLIC
        lgraph_server_lib
        ${BRPC_LIB}
        boost_system
        boost_filesystem
        ${JAVA_JVM_LIBRARY})
endif()

target_include_directories(${TARGET_CPP_CLIENT_REST} PRIVATE
        ${DEPS_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cypher
        ${LGRAPH_INCLUDE_DIR}
        ${JNI_INCLUDE_DIRS})

############### liblgraph_client_python ######################

set(TARGET_PYTHON_CLIENT lgraph_client_python)

add_library(${TARGET_PYTHON_CLIENT} SHARED
        client/python/rpc/client.cpp)

target_include_directories(${TARGET_PYTHON_CLIENT} PRIVATE
        ${PYTHON_INCLUDE_DIRS}
        ${LGRAPH_INCLUDE_DIR})

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(${TARGET_PYTHON_CLIENT} PUBLIC
            ${PYTHON_LIBRARIES}
            lgraph_client_cpp_rpc)
else ()
    target_link_libraries(${TARGET_PYTHON_CLIENT} PUBLIC
            lgraph_client_cpp_rpc
            rt
            lgraph)
endif ()
