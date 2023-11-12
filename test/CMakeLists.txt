cmake_minimum_required(VERSION 3.1)
project(LightningGraphTests C CXX)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_path(GTEST_HEADER NAMES gtest/gtest.h)
set(GTEST_LIB libgtest.a)
set(GTEST_MAIN_LIB libgtest_main.a)

#antlr4-runtime
find_package(antlr4-runtime REQUIRED)
include_directories( ${ANTLR4_INCLUDE_DIR} )

add_compile_options(-Wno-unused-local-typedefs)
add_compile_options(-Wno-sign-compare)
add_compile_options(-Wno-maybe-uninitialized)

add_subdirectory(test_fma)

add_executable(unit_test
        main.cpp
        ut_utils.cpp
        test_audit_logger.cpp
        test_backup_restore.cpp
        test_batch_vertex_index.cpp
        test_batch_edge_index.cpp
        test_blob_manager.cpp
        test_c.cpp
        test_cache_aligned_vector.cpp
        test_concurrent_gettime.cpp
        test_core_exception.cpp
        test_cpp_procedure.cpp
        test_cppresetsdk_json.cpp
        test_cypher.cpp
        test_cypher_plan.cpp
        test_data_type.cpp
        test_db_management_client.cpp
        test_dense_string.cpp
        test_edge_index.cpp
        test_embed.cpp
        test_field_data_helper.cpp
        test_field_extractor.cpp
        test_fulltext.cpp
        test_galaxy.cpp
        test_global_config.cpp
        test_graph.cpp
        test_graph_data_pack.cpp
        test_graph_edge_iterator.cpp
        test_graph_simple.cpp
        test_graph_traversal.cpp
        test_graph_vertex_iterator.cpp
        test_import_column_parser.cpp
        test_import_config_parser.cpp
        test_import_data_file.cpp
        test_import_online.cpp
        test_import_parse_delimiter.cpp
        test_import_planner.cpp
        test_import_v2.cpp
        test_import_v3.cpp
        test_type_convert.cpp
        test_json_convert.cpp
        test_lgraph.cpp
        test_lgraph_api.cpp
        test_lgraph_backup.cpp
        test_lgraph_date_time.cpp
        test_lgraph_export.cpp
        test_lgraph_monkey.cpp
        test_lgraph_server.cpp
        test_lgraph_utils.cpp
        test_kv_store.cpp
        test_lgraph.cpp
        test_lgraph_api.cpp
        test_managed_object.cpp
        test_max_db_size.cpp
        test_mdb.cpp
        test_mdb_wal_perf.cpp
        test_memorylimit.cpp
        test_merge.cpp
        test_move_constructors.cpp
        test_mthread.cpp
        test_olap_on_disk.cpp
        test_olap_on_db.cpp
        test_parallelvector.cpp
        test_perf_db.cpp
        test_perf_graph.cpp
        test_perf_kv.cpp
        test_perf_kv_fatkey.cpp
        test_perf_multi_writer.cpp
        test_perf_unaligned.cpp
        test_proto_convert.cpp
        test_python_plugin_manager.cpp
        test_python_plugin_manager_impl.cpp
        test_query.cpp
        test_random_delete.cpp
        test_rest_client.cpp
        test_restful_abnormal_branch.cpp
        test_restful_base_operation.cpp
        test_restful_import_online.cpp
        test_rpc.cpp
        test_schema.cpp
        test_schema_change.cpp
        test_schema_manager.cpp
        test_service.cpp
        test_snapshot.cpp
        test_sync_file_implementations.cpp
        test_static_vector.cpp
        test_task_tracker.cpp
        test_token_manager.cpp
        test_schema_common.cpp
        test_transaction.cpp
        test_txn_fork.cpp
        test_type_convert.cpp
        test_value.cpp
        test_vertex_index.cpp
        test_vid_table.cpp
        test_edge_constraint.cpp
        test_olap_vertex_traversal.cpp
        test_lgraph_log.cpp
        test_detach_property.cpp
        test_ha_base.cpp
        test_global_ha_config.cpp
        # test_ha.cpp
        ${LGRAPH_ROOT_DIR}/src/client/cpp/rpc/lgraph_rpc_client.cpp
        ${LGRAPH_ROOT_DIR}/src/client/cpp/restful/rest_client.cpp
)

target_link_libraries(unit_test
        lgraph_server_lib
        geax_isogql
        librocksdb.a)

target_link_libraries(unit_test
        ${GTEST_MAIN_LIB}
        ${GTEST_LIB})

target_compile_definitions(unit_test PRIVATE
        FMA_IN_UNIT_TEST=1)
# install
install(TARGETS unit_test
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib64)
