cmake_minimum_required(VERSION 3.13)
project(tugraph-db)

include(Options.cmake)
# generate version info
include(GenerateVersionInfo.cmake)
GenerateVersionInfo(${LGRAPH_VERSION_MAJOR} ${LGRAPH_VERSION_MINOR} ${LGRAPH_VERSION_PATCH}
		${PROJECT_SOURCE_DIR}/src/common/version.h.in
		${PROJECT_SOURCE_DIR}/src/common/version.h)
add_subdirectory(release)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(Threads REQUIRED)

find_package(antlr4-runtime REQUIRED)
include_directories( ${ANTLR4_INCLUDE_DIR} )

set(Protobuf_USE_STATIC_LIBS ON)
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

include_directories(src)
include_directories(deps/geax-front-end/include)

set(FT_INDEX_LIB ${PROJECT_SOURCE_DIR}/src/graphdb/ftindex/target/release/libftindex.a)

add_library(bolt STATIC
		src/bolt/connection.cpp
		src/bolt/hydrator.cpp
		src/bolt/pack.cpp
		src/bolt/bolt_server.cpp
)

add_library(graphdb STATIC
		src/graphdb/graph_db.cpp
		src/graphdb/id_generator.cpp
		src/graphdb/vertex_iterator.cpp
		src/graphdb/edge_iterator.cpp
		src/graphdb/graph_entity.cpp
		src/graphdb/meta_info.cpp
		src/graphdb/index.cpp
		src/graphdb/uuid_generator.cpp
		src/graphdb/embedding/delete_map.cpp
		src/graphdb/embedding/delta.cpp
		src/graphdb/embedding/faiss_index_chunk.cpp
		src/graphdb/embedding/id_mapper.cpp
		src/graphdb/embedding/index.cpp
		src/graphdb/embedding/mem_index_chunk.cpp
		src/graphdb/embedding/vsag_index_chunk.cpp
)

add_library(common STATIC
		src/common/exceptions.cpp
		src/common/value.cpp
		src/common/result.cpp
		src/common/flags.cpp
		src/common/logger.cpp
		src/common/temporal/date.cpp
		src/common/temporal/datetime.cpp
		src/common/temporal/localdatetime.cpp
		src/common/temporal/time.cpp
		src/common/temporal/localtime.cpp src/common/temporal/duration.cpp)

add_library(cypher STATIC
		src/cypher/parser/generated/LcypherLexer.cpp
		src/cypher/parser/generated/LcypherParser.cpp
		src/cypher/parser/generated/LcypherVisitor.cpp
		src/cypher/parser/cypher_base_visitor_v2.cpp
		src/cypher/parser/cypher_error_listener.cpp
		src/cypher/parser/symbol_table.cpp
		src/cypher/execution_plan/pattern_graph_maker.cpp
		src/cypher/execution_plan/execution_plan_maker.cpp
		src/cypher/execution_plan/execution_plan.cpp
		src/cypher/graph/node.cpp
		src/cypher/graph/relationship.cpp
		src/cypher/graph/graph.cpp
		src/cypher/execution_plan/ops/op_create.cpp
		src/cypher/execution_plan/ops/op_merge.cpp
		src/cypher/execution_plan/ops/op_set.cpp
		src/cypher/execution_plan/ops/op_remove.cpp
		src/cypher/execution_plan/ops/op_delete.cpp
		src/cypher/execution_plan/ops/op_produce_results.cpp
		src/cypher/execution_plan/ops/op_project.cpp
		src/cypher/execution_plan/ops/op_node_scan.cpp
		src/cypher/execution_plan/ops/op_expand_all.cpp
		src/cypher/execution_plan/ops/op_cartesian_product.cpp
		src/cypher/execution_plan/ops/op_filter.cpp
		src/cypher/execution_plan/ops/op_inquery_call.cpp
		src/cypher/execution_plan/ops/op_standalone_call.cpp
		src/cypher/execution_plan/ops/op_argument.cpp
		src/cypher/execution_plan/ops/op_apply.cpp
		src/cypher/execution_plan/ops/op_limit.cpp
		src/cypher/execution_plan/ops/op_skip.cpp
		src/cypher/execution_plan/ops/op_aggregate.cpp
		src/cypher/execution_plan/ops/op_optional.cpp
		src/cypher/execution_plan/ops/op_unwind.cpp
		src/cypher/execution_plan/ops/op_var_len_expand.cpp
		src/cypher/execution_plan/ops/op_sort.cpp
		src/cypher/execution_plan/ops/op_union.cpp
        src/cypher/execution_plan/result_iterator.cpp
		src/cypher/arithmetic/arithmetic_expression.cpp
		src/cypher/arithmetic/ast_expr_evaluator.cpp
		src/cypher/arithmetic/ast_agg_expr_detector.cpp
		src/cypher/resultset/record.cpp
		src/cypher/procedure/procedure.cpp
)
target_compile_definitions(cypher PUBLIC FOLLY_SANITIZE)

add_library(deps STATIC
		deps/geax-front-end/src/geax-front-end/isogql/GEAXErrorCode.cpp
)

add_library(transaction STATIC
		src/transaction/transaction.cpp
)
target_compile_definitions(transaction PUBLIC FOLLY_SANITIZE)

add_library(server STATIC
		src/server/galaxy.cpp
		src/server/bolt_handler.cpp
)
target_compile_definitions(server PUBLIC FOLLY_SANITIZE)

add_library(proto STATIC
		src/proto/meta.pb.cc
)

set(THIRD_STATIC_LIBS
	rocksdb.a
	spdlog.a
	lz4.a
	zstd.a
	gflags.a
	gtest.a
	protobuf.a
	date-tz.a
	${FT_INDEX_LIB}
	antlr4-runtime.a
	faiss.a
	openblas.a
	gomp
	gfortran
)

if (NOT ENABLE_ASAN)
	set(THIRD_STATIC_LIBS ${THIRD_STATIC_LIBS} libjemalloc.a)
endif ()

add_executable(lgraph_server
        src/server/server_main.cpp
)
target_link_libraries(lgraph_server PRIVATE
		vsag
		-Wl,--start-group
		${THIRD_STATIC_LIBS}
		bolt
		graphdb
		common
		cypher
		deps
		transaction
		server
		proto
		-static-libstdc++
		-static-libgcc
		pthread
		dl
		-Wl,--end-group
)

add_executable(test_case
    test/main.cpp
	test/test_graph_db.cpp
	test/test_kv_engine.cpp
	test/test_transaction.cpp
	test/test_index.cpp
	test/test_concurrency.cpp
	test/test_ftindex.cpp
	test/test_galaxy.cpp
	test/test_value.cpp
	test/test_vector_index.cpp
	test/test_var_len.cpp
	test/test_plan.cpp
	test/test_temporal.cpp
	test/test_benchmark.cpp)
target_compile_definitions(test_case PUBLIC FOLLY_SANITIZE)
target_link_libraries(test_case PRIVATE
		vsag
		-Wl,--start-group
		${THIRD_STATIC_LIBS}
		bolt
		graphdb
		common
		cypher
		deps
		transaction
		server
		proto
		-static-libstdc++
		-static-libgcc
		pthread
		dl
		-Wl,--end-group
)

add_executable(lgraph_cli
		src/toolkits/lgraph_cli.cpp
		src/toolkits/linenoise/linenoise.c
)

target_link_libraries(lgraph_cli PRIVATE
		-Wl,--start-group
		bolt
		common
		gflags.a
		date-tz.a
		-static-libstdc++
		-static-libgcc
		pthread
		-Wl,--end-group
)

if(NOT EXISTS ${FT_INDEX_LIB})
	add_custom_target(
		BuildFTIndex
		COMMAND cargo build --release
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/graphdb/ftindex
	)
	add_dependencies(lgraph_server BuildFTIndex)
	add_dependencies(test_case BuildFTIndex)
endif()

install(TARGETS lgraph_server lgraph_cli RUNTIME DESTINATION bin)
install(FILES /usr/local/lib64/libvsag.so DESTINATION lib64/lgraph)

