cmake_minimum_required(VERSION 3.1)

# threads
find_package(Threads REQUIRED)

if (ENABLE_FULLTEXT_INDEX)
    # jni
    if (APPLE)
        SET(JAVA_INCLUDE_PATH "$ENV{JAVA_INCLUDE_PATH}")
    endif ()
    find_package(JNI REQUIRED)
endif ()

# generate version info
include(cmake/GenerateVersionInfo.cmake)
GenerateVersionInfo(${LGRAPH_VERSION_MAJOR} ${LGRAPH_VERSION_MINOR} ${LGRAPH_VERSION_PATCH}
        ${CMAKE_CURRENT_LIST_DIR}/core/version.h.in
        ${CMAKE_CURRENT_LIST_DIR}/core/version.h)

# protbuf
include(cmake/GenerateProtobuf.cmake)
GenerateProtobufCpp(${CMAKE_CURRENT_LIST_DIR}/protobuf
        PROTO_SRCS PROTO_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/protobuf/ha.proto)

set(LGRAPH_CORE_SRC
        core/audit_logger.cpp
        core/data_type.cpp
        core/edge_index.cpp
        core/field_extractor_base.cpp
        core/field_extractor_v1.cpp
        core/field_extractor_v2.cpp
        core/full_text_index.cpp
        core/global_config.cpp
        core/graph.cpp
        core/graph_edge_iterator.cpp
        core/graph_vertex_iterator.cpp
        core/index_manager.cpp
        core/iterator_base.cpp
        core/lmdb_iterator.cpp
        core/lmdb_store.cpp
        core/lmdb_table.cpp
        core/lmdb_transaction.cpp
        core/kv_table_comparators.cpp
        core/lgraph_date_time.cpp
        core/lgraph_spatial.cpp
        core/lightning_graph.cpp
        core/schema.cpp
        core/sync_file.cpp
        core/thread_id.cpp
        core/transaction.cpp
        core/vertex_index.cpp
        core/vector_index.cpp
        core/faiss_ivf_flat.cpp
        core/vsag_hnsw.cpp
        core/wal.cpp
        core/lmdb/mdb.c
        core/lmdb/midl.c
        core/composite_index.cpp)

set(LGRAPH_DB_SRC
        db/acl.cpp
        db/db.cpp
        db/galaxy.cpp
        db/graph_manager.cpp
        db/token_manager.cpp)

set(LGRAPH_API_SRC
        lgraph_api/c.cpp
        lgraph_api/lgraph_db.cpp
        lgraph_api/lgraph_edge_iterator.cpp
        lgraph_api/lgraph_galaxy.cpp
        lgraph_api/olap_base.cpp
        lgraph_api/lgraph_vertex_index_iterator.cpp
        lgraph_api/lgraph_edge_index_iterator.cpp
        lgraph_api/lgraph_traversal.cpp
        lgraph_api/lgraph_txn.cpp
        lgraph_api/lgraph_types.cpp
        lgraph_api/lgraph_utils.cpp
        lgraph_api/lgraph_vertex_iterator.cpp
        lgraph_api/lgraph_result.cpp
        lgraph_api/lgraph_exceptions.cpp
        lgraph_api/result_element.cpp
        lgraph_api/lgraph_vertex_composite_index_iterator.cpp)

set(TARGET_LGRAPH lgraph)

add_library(${TARGET_LGRAPH} SHARED
        ${LGRAPH_API_SRC}
        ${LGRAPH_CORE_SRC}
        ${LGRAPH_DB_SRC}
        ${LGRAPH_ALGO_SRC}

        plugin/cpp_plugin.cpp
        plugin/plugin_context.cpp
        plugin/plugin_manager.cpp
        plugin/python_plugin.cpp

        tiny-process-library/process.cpp
        tiny-process-library/process_unix.cpp
        ${PROTO_SRCS})

target_include_directories(${TARGET_LGRAPH} PUBLIC
        ${DEPS_LOCAL_INCLUDE_DIR}
        ${DEPS_INCLUDE_DIR}
        ${LGRAPH_INCLUDE_DIR}
        ${LGRAPH_SRC_DIR}
        ${LGRAPH_SRC_DIR}/cypher
        ${JNI_INCLUDE_DIRS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${TARGET_LGRAPH} PUBLIC
            -laio
            /usr/local/lib64/libvsag_static.a
            /usr/local/lib64/libvsag_mockimpl.a
            libdiskann.a
            libcpuinfo.a
            libsimd.a
            libroaring.a
            libgomp.a
            -static-libstdc++
            -static-libgcc
            libstdc++fs.a
            ${Boost_LIBRARIES}
            -Wl,-Bdynamic
            -Wl,--whole-archive
            ${PROTOBUF_LIBRARY}
            -Wl,--no-whole-archive
            ${JAVA_JVM_LIBRARY}
            OpenSSL::crypto
            pthread
            rt
            z
            /opt/OpenBLAS/lib/libopenblas.a
            faiss
            )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_link_libraries(${TARGET_LGRAPH} PUBLIC
                -laio
                /usr/local/lib64/libvsag_static.a
                /usr/local/lib64/libvsag_mockimpl.a
                libdiskann.a
                libcpuinfo.a
                libsimd.a
                libroaring.a
                /opt/OpenBLAS/lib/libopenblas.a
                faiss
                ${Boost_LIBRARIES}
                omp
                pthread
                OpenSSL::crypto
                ${PROTOBUF_LIBRARY}
                ${JAVA_JVM_LIBRARY})
    else ()
        target_link_libraries(${TARGET_LGRAPH} PUBLIC
                -laio
                /usr/local/lib64/libvsag_static.a
                /usr/local/lib64/libvsag_mockimpl.a
                libdiskann.a
                libcpuinfo.a
                libsimd.a
                libroaring.a
                /opt/OpenBLAS/lib/libopenblas.a
                faiss
                rt
                omp
                pthread
                -Wl,-Bstatic
                ${Boost_LIBRARIES}
                -Wl,-Bdynamic
                c++experimental
                c++fs
                OpenSSL::crypto
                -Wl,--whole-archive
                ${PROTOBUF_LIBRARY}
                -Wl,--no-whole-archive
                ${JAVA_JVM_LIBRARY}
                z)
    endif ()
endif ()

if (ENABLE_SQL_IO)
    find_library(ODBC_LIBRARIES NAMES odbc)
    if (ODBC_LIBRARIES_NOT_FOUND)
        message(FATAL_ERROR "ODBC enabled, but unable to find ODBC library.")
    endif (ODBC_LIBRARIES_NOT_FOUND)
    target_sources(${TARGET_LGRAPH} PRIVATE ${DEPS_INCLUDE_DIR}/fma-common/sql_stream.cpp)
    target_link_libraries(${TARGET_LGRAPH} PUBLIC odbc)
endif (ENABLE_SQL_IO)

set_target_properties(lgraph PROPERTIES OUTPUT_NAME "lgraph")
target_compile_definitions(lgraph PUBLIC -DBOOST_STACKTRACE_USE_ADDR2LINE=1)
