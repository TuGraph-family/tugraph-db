cmake_minimum_required(VERSION 3.1)

find_package(PythonLibs 3 REQUIRED)

find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
find_package(OpenSSL)

# boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.68 REQUIRED COMPONENTS system filesystem)

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
        core/field_extractor.cpp
        core/full_text_index.cpp
        core/global_config.cpp
        core/graph.cpp
        core/graph_edge_iterator.cpp
        core/graph_vertex_iterator.cpp
        core/index_manager.cpp
        core/iterator_base.cpp
        core/kv_store_iterator.cpp
        core/kv_store_mdb.cpp
        core/kv_store_table.cpp
        core/kv_store_transaction.cpp
        core/kv_table_comparators.cpp
        core/lgraph_date_time.cpp
        core/lightning_graph.cpp
        core/schema.cpp
        core/sync_file.cpp
        core/thread_id.cpp
        core/transaction.cpp
        core/vertex_index.cpp
        core/wal.cpp
        core/lmdb/mdb.c
        core/lmdb/midl.c)

set(LGRAPH_DB_SRC
        db/acl.cpp
        db/db.cpp
        db/galaxy.cpp
        db/graph_manager.cpp
        db/token_manager.cpp)

set(LGRAPH_API_SRC
        lgraph_api/lgraph_db.cpp
        lgraph_api/lgraph_edge_iterator.cpp
        lgraph_api/lgraph_galaxy.cpp
        lgraph_api/olap_base.cpp
        lgraph_api/lgraph_vertex_index_iterator.cpp
        lgraph_api/lgraph_edge_index_iterator.cpp
        lgraph_api/lgraph_traversal.cpp
        lgraph_api/lgraph_txn.cpp
        lgraph_api/lgraph_utils.cpp
        lgraph_api/lgraph_vertex_iterator.cpp
        lgraph_api/lgraph_result.cpp
        lgraph_api/result_element.cpp)


set(LGRAPH_CYPHER_SRC   # find cypher/ -name "*.cpp" | sort
        cypher/arithmetic/agg_funcs.cpp
        cypher/arithmetic/arithmetic_expression.cpp
        cypher/execution_plan/execution_plan.cpp
        cypher/execution_plan/ops/op_aggregate.cpp
        cypher/execution_plan/ops/op_all_node_scan.cpp
        cypher/execution_plan/ops/op_apply.cpp
        cypher/execution_plan/ops/op_argument.cpp
        cypher/execution_plan/ops/op_cartesian_product.cpp
        cypher/execution_plan/ops/op_create.cpp
        cypher/execution_plan/ops/op_delete.cpp
        cypher/execution_plan/ops/op_distinct.cpp
        cypher/execution_plan/ops/op_expand_all.cpp
        cypher/execution_plan/ops/op_filter.cpp
        cypher/execution_plan/ops/op_node_index_seek.cpp
        cypher/execution_plan/ops/op_node_index_seek_dynamic.cpp
        cypher/execution_plan/ops/op_immediate_argument.cpp
        cypher/execution_plan/ops/op_inquery_call.cpp
        cypher/execution_plan/ops/op_limit.cpp
        cypher/execution_plan/ops/op_node_by_label_scan.cpp
        cypher/execution_plan/ops/op_optional.cpp
        cypher/execution_plan/ops/op_produce_results.cpp
        cypher/execution_plan/ops/op_project.cpp
        cypher/execution_plan/ops/op_relationship_count.cpp
        cypher/execution_plan/ops/op_remove.cpp
        cypher/execution_plan/ops/op_set.cpp
        cypher/execution_plan/ops/op_skip.cpp
        cypher/execution_plan/ops/op_sort.cpp
        cypher/execution_plan/ops/op_standalone_call.cpp
        cypher/execution_plan/ops/op_union.cpp
        cypher/execution_plan/ops/op_unwind.cpp
        cypher/execution_plan/ops/op_var_len_expand.cpp
        cypher/execution_plan/ops/op_var_len_expand_into.cpp
        cypher/execution_plan/ops/op_merge.cpp
        cypher/execution_plan/ops/op_node_by_id_seek.cpp
        cypher/execution_plan/ops/op_traversal.cpp
        cypher/execution_plan/scheduler.cpp
        cypher/filter/filter.cpp
        cypher/filter/iterator.cpp
        cypher/graph/graph.cpp
        cypher/graph/node.cpp
        cypher/graph/relationship.cpp
        cypher/grouping/group.cpp
        cypher/parser/cypher_base_visitor.cpp
        cypher/parser/cypher_error_listener.cpp
        cypher/parser/symbol_table.cpp
        cypher/parser/generated/LcypherLexer.cpp
        cypher/parser/generated/LcypherParser.cpp
        cypher/parser/generated/LcypherVisitor.cpp
        cypher/procedure/procedure.cpp
        cypher/resultset/record.cpp
        cypher/monitor/monitor_manager.cpp
        )

set(ANTRL4_LIBRARY antlr4-runtime.a)
set(BRPC_LIB libbrpc.a)

add_library(lgraph_python SHARED
        ${LGRAPH_API_SRC}
        ${LGRAPH_CORE_SRC}
        ${LGRAPH_DB_SRC}
        ${LGRAPH_ALGO_SRC}

        plugin/cpp_plugin.cpp
        plugin/plugin_context.cpp
        plugin/plugin_manager.cpp
        plugin/python_plugin.cpp

        ${LGRAPH_CYPHER_SRC}
        server/state_machine.cpp
        python/python_api.cpp
        import/import_online.cpp
        import/import_v2.cpp

        ${DEPS_INCLUDE_DIR}/tiny-process-library/process.cpp
        ${DEPS_INCLUDE_DIR}/tiny-process-library/process_unix.cpp
        ${DEPS_INCLUDE_DIR}/antlr4-runtime/support/Any.h
        ${PROTO_SRCS})

add_custom_command(
        OUTPUT ${DEPS_INCLUDE_DIR}/antlr4-runtime/support/Any.h
        COMMAND cp -p ${CMAKE_CURRENT_LIST_DIR}/cypher/Any.h.491+ ${DEPS_INCLUDE_DIR}/antlr4-runtime/support/Any.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/cypher/Any.h.491+
)

target_include_directories(lgraph_python PUBLIC
        ${PYTHON_INCLUDE_DIRS}
        ${DEPS_LOCAL_INCLUDE_DIR}
        ${DEPS_INCLUDE_DIR}
        ${LGRAPH_INCLUDE_DIR}
        ${LGRAPH_SRC_DIR}
        ${DEPS_INCLUDE_DIR}/antlr4-runtime
        ${LGRAPH_SRC_DIR}/cypher
        ${JNI_INCLUDE_DIRS})

target_link_libraries(lgraph_python PUBLIC
        libgomp.a
        -static-libstdc++
        -static-libgcc
        libstdc++fs.a
	-Wl,-Bstatic
        ${ANTRL4_LIBRARY}
        ${BRPC_LIB}
        ${LEVELDB_LIB}
        cpprest
        -Wl,-Bdynamic
        -Wl,--whole-archive
        ${PROTOBUF_LIBRARY}
        libboost_system.a
        ${JAVA_JVM_LIBRARY}
        -Wl,--no-whole-archive
        gflags
        snappy
        crypto
        pthread
        ssl
        rt
        z
        )

if (ENABLE_SQL_IO)
    find_library(ODBC_LIBRARIES NAMES odbc)
    if (ODBC_LIBRARIES_NOT_FOUND)
        message(FATAL_ERROR "ODBC enabled, but unable to find ODBC library.")
    endif (ODBC_LIBRARIES_NOT_FOUND)
    target_sources(lgraph_python PRIVATE ${DEPS_INCLUDE_DIR}/fma-common/sql_stream.cpp)
    target_link_libraries(lgraph_python PUBLIC odbc)
endif (ENABLE_SQL_IO)

set_target_properties(lgraph_python PROPERTIES PREFIX "")
target_compile_definitions(lgraph_python PUBLIC -DBOOST_STACKTRACE_USE_ADDR2LINE=1)
target_compile_options(lgraph_python PUBLIC -fPIC -fno-omit-frame-pointer)

