cmake_minimum_required(VERSION 3.1)

find_package(PythonInterp 3)
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)
#antlr4-runtime
find_package(antlr4-runtime REQUIRED)
# find_package(LLVM REQUIRED CONFIG)
set(ANTRL4_LIBRARY antlr4-runtime.a)

set(TARGET_LGRAPH_CYPHER_LIB lgraph_cypher_lib)
set(EXTERNAL_PROJECT_DIR "${CMAKE_SOURCE_DIR}/deps/buildit")
add_custom_target(buildit
    COMMAND ${CMAKE_COMMAND} -E chdir ${EXTERNAL_PROJECT_DIR} $(MAKE)
    COMMENT "Building external project"
)


set(LGRAPH_CYPHER_SRC   # find cypher/ -name "*.cpp" | sort
        cypher/arithmetic/agg_funcs.cpp
        cypher/arithmetic/arithmetic_expression.cpp
        cypher/arithmetic/ast_agg_expr_detector.cpp
        cypher/arithmetic/ast_expr_evaluator.cpp
        cypher/execution_plan/execution_plan.cpp
        cypher/execution_plan/execution_plan_v2.cpp
        cypher/execution_plan/execution_plan_maker.cpp
        cypher/execution_plan/pattern_graph_maker.cpp
        cypher/execution_plan/ops/op_aggregate.cpp
        cypher/execution_plan/ops/op_all_node_scan.cpp
        cypher/execution_plan/ops/op_apply.cpp
        cypher/execution_plan/ops/op_argument.cpp
        cypher/execution_plan/ops/op_cartesian_product.cpp
        cypher/execution_plan/ops/op_create.cpp
        cypher/execution_plan/ops/op_gql_create.cpp
        cypher/execution_plan/ops/op_delete.cpp
        cypher/execution_plan/ops/op_gql_delete.cpp
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
        cypher/execution_plan/ops/op_gql_set.cpp
        cypher/execution_plan/ops/op_skip.cpp
        cypher/execution_plan/ops/op_sort.cpp
        cypher/execution_plan/ops/op_standalone_call.cpp
        cypher/execution_plan/ops/op_gql_standalone_call.cpp
        cypher/execution_plan/ops/op_union.cpp
        cypher/execution_plan/ops/op_unwind.cpp
        cypher/execution_plan/ops/op_var_len_expand.cpp
        cypher/execution_plan/ops/op_var_len_expand_into.cpp
        cypher/execution_plan/ops/op_merge.cpp
        cypher/execution_plan/ops/op_gql_merge.cpp
        cypher/execution_plan/ops/op_node_by_id_seek.cpp
        cypher/execution_plan/ops/op_traversal.cpp
        cypher/execution_plan/ops/op_gql_remove.cpp
        cypher/execution_plan/scheduler.cpp
        cypher/experimental/data_type/field_data.h
        cypher/experimental/expressions/cexpr.cpp
        cypher/experimental/expressions/kernal/binary.cpp
        # cypher/experimental/jit/TuJIT.cpp
        cypher/filter/filter.cpp
        cypher/filter/iterator.cpp
        cypher/graph/graph.cpp
        cypher/graph/node.cpp
        cypher/graph/relationship.cpp
        cypher/grouping/group.cpp
        cypher/parser/cypher_base_visitor.cpp
        cypher/parser/cypher_base_visitor_v2.cpp
        cypher/parser/cypher_error_listener.cpp
        cypher/parser/expression.cpp
        cypher/parser/symbol_table.cpp
        cypher/parser/generated/LcypherLexer.cpp
        cypher/parser/generated/LcypherParser.cpp
        cypher/parser/generated/LcypherVisitor.cpp
        cypher/procedure/procedure.cpp
        cypher/procedure/utils.cpp
        cypher/resultset/record.cpp
        cypher/monitor/monitor_manager.cpp
        cypher/execution_plan/optimization/rewrite/schema_rewrite.cpp
        cypher/execution_plan/optimization/rewrite/graph.cpp
        )

add_library(${TARGET_LGRAPH_CYPHER_LIB} STATIC
        ${LGRAPH_CYPHER_SRC}
        ${PROTO_HEADERS})

set_target_properties(${TARGET_LGRAPH_CYPHER_LIB} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${TARGET_LGRAPH_CYPHER_LIB} PUBLIC
        ${ANTLR4_INCLUDE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/cypher)

include_directories(
        ${CMAKE_SOURCE_DIR}/deps/buildit/include
        ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

target_link_directories(${TARGET_LGRAPH_CYPHER_LIB} PUBLIC 
        ${CMAKE_SOURCE_DIR}/deps/buildit/lib)

add_dependencies(${TARGET_LGRAPH_CYPHER_LIB} buildit)

target_link_libraries(${TARGET_LGRAPH_CYPHER_LIB} PUBLIC
        ${ANTRL4_LIBRARY}
        geax_isogql
        # ${CMAKE_SOURCE_DIR}/deps/buildit/build/libbuildit.a
        lgraph)

target_link_libraries(${TARGET_LGRAPH_CYPHER_LIB} PRIVATE
        lgraph_server_lib)

# llvm_map_components_to_libnames(llvm_libs Core Support)
# target_link_libraries(${TARGET_LGRAPH_CYPHER_LIB} PRIVATE ${llvm_libs})