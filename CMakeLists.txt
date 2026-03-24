cmake_minimum_required(VERSION 3.13)

project(tugraph-db LANGUAGES C CXX)

# Project setup
include(Options.cmake)
include(GenerateVersionInfo.cmake)

generateversioninfo(
  ${LGRAPH_VERSION_MAJOR}
  ${LGRAPH_VERSION_MINOR}
  ${LGRAPH_VERSION_PATCH}
  ${PROJECT_SOURCE_DIR}/src/common/version.h.in
  ${PROJECT_SOURCE_DIR}/src/common/version.h
)

add_subdirectory(release)

# Dependencies
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Threads REQUIRED)
find_package(antlr4-runtime REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
set(GFLAGS_SHARED FALSE)
find_package(gflags CONFIG REQUIRED)
find_package(date CONFIG REQUIRED)

set(Protobuf_USE_STATIC_LIBS ON)
include(FindProtobuf)
find_package(Protobuf REQUIRED)

# Shared build settings
set(PROTOBUF_INCLUDE_DIRS_LOCAL ${PROTOBUF_INCLUDE_DIR} ${Protobuf_INCLUDE_DIR}
                                ${Protobuf_INCLUDE_DIRS}
)

set(COMMON_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/deps/geax-front-end/include)

add_library(project_common INTERFACE)
target_include_directories(project_common INTERFACE ${COMMON_INCLUDE_DIRS})

function(add_static_library target_name)
  add_library(${target_name} STATIC ${ARGN})
  target_link_libraries(${target_name} PUBLIC project_common)
endfunction()

function(add_executable_target target_name)
  add_executable(${target_name} ${ARGN})
  target_link_libraries(${target_name} PRIVATE project_common)
endfunction()

function(enable_folly_sanitize target_name)
  target_compile_definitions(${target_name} PUBLIC FOLLY_SANITIZE)
endfunction()

function(find_static_library out_var)
  set(_saved_find_library_suffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
  set(_find_var "${out_var}_FOUND_PATH")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  find_library(${_find_var} NAMES ${ARGN} REQUIRED)
  set(CMAKE_FIND_LIBRARY_SUFFIXES "${_saved_find_library_suffixes}")
  set(${out_var}
      "${${_find_var}}"
      PARENT_SCOPE
  )
  unset(${_find_var} CACHE)
endfunction()

function(add_imported_static_archive target_name archive_path)
  add_library(
    ${target_name}
    STATIC
    IMPORTED
    GLOBAL
  )
  set_target_properties(${target_name} PROPERTIES IMPORTED_LOCATION "${archive_path}")
endfunction()

function(
  get_compiler_static_library
  out_var
  compiler
  library_name
)
  execute_process(
    COMMAND "${compiler}" -print-file-name=${library_name}
    OUTPUT_VARIABLE _library_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT _library_path
     OR _library_path STREQUAL "${library_name}"
     OR NOT EXISTS "${_library_path}"
  )
    message(FATAL_ERROR "Could not locate ${library_name} via ${compiler}")
  endif()

  set(${out_var}
      "${_library_path}"
      PARENT_SCOPE
  )
endfunction()

set(FTINDEX_DIR "${PROJECT_SOURCE_DIR}/src/graphdb/ftindex")
set(FTINDEX_LIB "${FTINDEX_DIR}/target/release/libftindex.a")
set(FTINDEX_HEADER "${FTINDEX_DIR}/include/lib.rs.h")
set(FTINDEX_GENERATED_HEADER "${FTINDEX_DIR}/target/cxxbridge/ftindex/src/lib.rs.h")

find_static_library(ROCKSDB_LIBRARY rocksdb)
find_static_library(LZ4_LIBRARY lz4)
find_static_library(ZSTD_LIBRARY zstd)
find_static_library(FAISS_LIBRARY faiss)
find_static_library(OPENBLAS_LIBRARY openblas)
get_compiler_static_library(GOMP_LIBRARY "${CMAKE_CXX_COMPILER}" libgomp.a)
find_static_library(GFORTRAN_LIBRARY gfortran)

if(NOT ENABLE_ASAN)
  find_static_library(JEMALLOC_LIBRARY jemalloc)
endif()

add_imported_static_archive(rocksdb_archive "${ROCKSDB_LIBRARY}")
add_imported_static_archive(lz4_archive "${LZ4_LIBRARY}")
add_imported_static_archive(zstd_archive "${ZSTD_LIBRARY}")
add_imported_static_archive(faiss_archive "${FAISS_LIBRARY}")
add_imported_static_archive(openblas_archive "${OPENBLAS_LIBRARY}")
add_imported_static_archive(gomp_archive "${GOMP_LIBRARY}")
add_imported_static_archive(gfortran_archive "${GFORTRAN_LIBRARY}")

if(NOT ENABLE_ASAN)
  add_imported_static_archive(jemalloc_archive "${JEMALLOC_LIBRARY}")
endif()

file(
  GLOB
  FTINDEX_MANIFEST_FILES
  CONFIGURE_DEPENDS
  "${FTINDEX_DIR}/Cargo.toml"
  "${FTINDEX_DIR}/Cargo.lock"
  "${FTINDEX_DIR}/build.rs"
)
file(
  GLOB_RECURSE
  FTINDEX_SOURCE_FILES
  CONFIGURE_DEPENDS
  "${FTINDEX_DIR}/src/*.rs"
)

add_custom_command(
  OUTPUT "${FTINDEX_LIB}" "${FTINDEX_HEADER}"
  COMMAND cargo build --release
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different "${FTINDEX_GENERATED_HEADER}"
    "${FTINDEX_HEADER}"
  WORKING_DIRECTORY "${FTINDEX_DIR}"
  DEPENDS ${FTINDEX_MANIFEST_FILES} ${FTINDEX_SOURCE_FILES}
  COMMENT "Building ftindex static library and syncing bridge header"
  VERBATIM
)

add_custom_target(ftindex_static_build DEPENDS "${FTINDEX_LIB}" "${FTINDEX_HEADER}")
add_imported_static_archive(ftindex_archive "${FTINDEX_LIB}")
add_dependencies(ftindex_archive ftindex_static_build)

set(CORE_LINK_LIBS
    bolt
    graphdb
    common
    cypher
    deps
    transaction
    server
    proto
)

set(THIRD_PARTY_STATIC_LIBS
    rocksdb_archive
    spdlog::spdlog
    lz4_archive
    zstd_archive
    gflags::gflags
    protobuf::libprotobuf
    date::date-tz
    ftindex_archive
    antlr4_static
    faiss_archive
    openblas_archive
    gomp_archive
    gfortran_archive
)

if(NOT ENABLE_ASAN)
  list(APPEND THIRD_PARTY_STATIC_LIBS jemalloc_archive)
endif()

set(RUNTIME_GROUP_LIBS ${THIRD_PARTY_STATIC_LIBS} ${CORE_LINK_LIBS})

set(RUNTIME_SYSTEM_LIBS
    -static-libstdc++
    -static-libgcc
    pthread
    dl
)

set(RUNTIME_LINK_LIBS
    -Wl,--start-group
    ${RUNTIME_GROUP_LIBS}
    -Wl,--end-group
    ${RUNTIME_SYSTEM_LIBS}
)

set(TEST_LINK_LIBS
    -Wl,--start-group
    ${RUNTIME_GROUP_LIBS}
    GTest::gtest
    -Wl,--end-group
    ${RUNTIME_SYSTEM_LIBS}
)

set(CLI_LINK_LIBS
    bolt
    common
    gflags::gflags
    date::date-tz
    -static-libstdc++
    -static-libgcc
    pthread
)

# Source lists
set(BOLT_SOURCES
    src/bolt/connection.cpp
    src/bolt/hydrator.cpp
    src/bolt/pack.cpp
    src/bolt/bolt_server.cpp
)

set(GRAPHDB_SOURCES
    src/graphdb/graph_db.cpp
    src/graphdb/id_generator.cpp
    src/graphdb/vertex_iterator.cpp
    src/graphdb/edge_iterator.cpp
    src/graphdb/graph_entity.cpp
    src/graphdb/meta_info.cpp
    src/graphdb/index.cpp
    src/graphdb/hnsw_index.cpp
)

set(COMMON_SOURCES
    src/common/exceptions.cpp
    src/common/value.cpp
    src/common/result.cpp
    src/common/flags.cpp
    src/common/logger.cpp
    src/common/temporal/date.cpp
    src/common/temporal/datetime.cpp
    src/common/temporal/localdatetime.cpp
    src/common/temporal/time.cpp
    src/common/temporal/localtime.cpp
    src/common/temporal/duration.cpp
)

set(CYPHER_SOURCES
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

set(DEPS_SOURCES deps/geax-front-end/src/geax-front-end/isogql/GEAXErrorCode.cpp)

set(TRANSACTION_SOURCES src/transaction/transaction.cpp)

set(SERVER_SOURCES src/server/galaxy.cpp src/server/bolt_handler.cpp)

set(PROTO_SOURCES src/proto/meta.pb.cc)

set(TEST_CASE_SOURCES
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
    test/test_benchmark.cpp
)

set(LGRAPH_CLI_SOURCES src/toolkits/lgraph_cli.cpp src/toolkits/linenoise/linenoise.c)

# Libraries
add_static_library(bolt ${BOLT_SOURCES})
add_static_library(graphdb ${GRAPHDB_SOURCES})
add_static_library(common ${COMMON_SOURCES})

add_static_library(cypher ${CYPHER_SOURCES})
enable_folly_sanitize(cypher)
target_include_directories(cypher PUBLIC ${ANTLR4_INCLUDE_DIR})

add_static_library(deps ${DEPS_SOURCES})

add_static_library(transaction ${TRANSACTION_SOURCES})
enable_folly_sanitize(transaction)

add_static_library(server ${SERVER_SOURCES})
enable_folly_sanitize(server)

add_static_library(proto ${PROTO_SOURCES})
target_include_directories(proto PUBLIC ${PROTOBUF_INCLUDE_DIRS_LOCAL})

target_include_directories(graphdb PUBLIC ${PROTOBUF_INCLUDE_DIRS_LOCAL})
add_dependencies(graphdb ftindex_static_build)
add_dependencies(transaction ftindex_static_build)
add_dependencies(cypher ftindex_static_build)
add_dependencies(server ftindex_static_build)

# Executables
add_executable_target(lgraph_server src/server/server_main.cpp)
target_link_libraries(lgraph_server PRIVATE ${RUNTIME_LINK_LIBS})

add_executable_target(test_case ${TEST_CASE_SOURCES})
enable_folly_sanitize(test_case)
add_dependencies(test_case ftindex_static_build)
target_link_libraries(test_case PRIVATE ${TEST_LINK_LIBS})

add_executable_target(lgraph_cli ${LGRAPH_CLI_SOURCES})
target_link_libraries(lgraph_cli PRIVATE ${CLI_LINK_LIBS})

include(${PROJECT_SOURCE_DIR}/cmake/Format.cmake)

# Installation
install(TARGETS lgraph_server lgraph_cli RUNTIME DESTINATION bin)
