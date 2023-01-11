cmake_minimum_required(VERSION 3.1)

find_package(PythonLibs REQUIRED)

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
        server/state_machine.cpp
        )

#antlr4-runtime
set(ANTRL4_LIBRARY antlr4-runtime.a)

add_library(lgraph_cypher_lib STATIC
        ${LGRAPH_CYPHER_SRC}
        ${PROTO_HEADERS}
        ${DEPS_INCLUDE_DIR}/antlr4-runtime/support/Any.h)
set_target_properties(lgraph_cypher_lib PROPERTIES POSITION_INDEPENDENT_CODE ON LINKER_LANGUAGE CXX)
add_custom_command(
        OUTPUT ${DEPS_INCLUDE_DIR}/antlr4-runtime/support/Any.h
        COMMAND cp -p ${CMAKE_CURRENT_LIST_DIR}/cypher/Any.h.491+ ${DEPS_INCLUDE_DIR}/antlr4-runtime/support/Any.h
        DEPENDS ${CMAKE_CURRENT_LIST_DIR}/cypher/Any.h.491+
)
target_include_directories(lgraph_cypher_lib PUBLIC
        ${DEPS_INCLUDE_DIR}/antlr4-runtime
        ${CMAKE_CURRENT_LIST_DIR}/cypher)

target_link_libraries(lgraph_cypher_lib PUBLIC
        ${ANTRL4_LIBRARY}
        cpprest
        lgraph)
