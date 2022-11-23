cmake_minimum_required(VERSION 3.2)
project(TuGraph C CXX)

add_executable(lgraph_import
        lgraph_import.cpp
        ${LGRAPH_ROOT_DIR}/src/client/cpp/rpc/lgraph_rpc_client.cpp
        ${LGRAPH_ROOT_DIR}/src/client/cpp/restful/rest_client.cpp
        ${LGRAPH_ROOT_DIR}/src/import/import_client.cpp)

# TODO: break circular dependency between cypher and state machine
target_link_libraries(lgraph_import
        lgraph_server_lib
        lgraph_cypher_lib
        lgraph_server_lib)

add_executable(lgraph_monitor
        lgraph_monitor.cpp
        ${LGRAPH_ROOT_DIR}/src/monitor/prometheus_monitor.cpp)

target_include_directories(lgraph_monitor PUBLIC
        ${LGRAPH_ROOT_DIR}/include
        ${LGRAPH_ROOT_DIR}/src)

target_link_libraries(lgraph_monitor
        liblgraph_rpc_client
        libprometheus-cpp-pull.a
        libprometheus-cpp-core.a)

add_executable(lgraph_backup lgraph_backup.cpp)
target_link_libraries(lgraph_backup lgraph_server_lib)

add_executable(lgraph_warmup lgraph_warmup.cpp)
target_link_libraries(lgraph_warmup lgraph_server_lib)

add_executable(lgraph_peek lgraph_peek.cpp)
target_link_libraries(lgraph_peek lgraph_server_lib)

add_executable(lgraph_validate lgraph_validate.cpp)
target_link_libraries(lgraph_validate lgraph_server_lib)

add_executable(lgraph_export lgraph_export.cpp)
target_link_libraries(lgraph_export lgraph_server_lib)

add_executable(lgraph_binlog lgraph_binlog.cpp
               ${LGRAPH_ROOT_DIR}/src/client/cpp/rpc/lgraph_rpc_client.cpp
               ${LGRAPH_ROOT_DIR}/src/client/cpp/restful/rest_client.cpp)
target_link_libraries(lgraph_binlog lgraph_server_lib)

set(LGRAPH_TOOLKITS lgraph_import lgraph_backup lgraph_warmup lgraph_peek lgraph_export lgraph_binlog lgraph_monitor)

# install
install(TARGETS ${LGRAPH_TOOLKITS}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib64)


