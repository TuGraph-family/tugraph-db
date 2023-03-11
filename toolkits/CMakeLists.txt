cmake_minimum_required(VERSION 3.2)
project(TuGraph C CXX)

############### lgraph_import ######################

set(TARGET_LGRAPH_IMPORT lgraph_import)

add_executable(${TARGET_LGRAPH_IMPORT}
        lgraph_import.cpp
        ${LGRAPH_ROOT_DIR}/src/client/cpp/rpc/lgraph_rpc_client.cpp
        ${LGRAPH_ROOT_DIR}/src/client/cpp/restful/rest_client.cpp
        ${LGRAPH_ROOT_DIR}/src/import/import_client.cpp)

# TODO: break circular dependency between cypher and state machine
target_link_libraries(${TARGET_LGRAPH_IMPORT}
        lgraph_server_lib
        lgraph_cypher_lib
        lgraph_server_lib
        librocksdb.a)

if (NOT ENABLE_ASAN)
    target_link_libraries(${TARGET_LGRAPH_IMPORT} libjemalloc.a)
endif()

############### lgraph_monitor ######################

set(TARGET_LGRAPH_MONITOR lgraph_monitor)

add_executable(${TARGET_LGRAPH_MONITOR}
         lgraph_monitor.cpp
         ${LGRAPH_ROOT_DIR}/src/monitor/prometheus_monitor.cpp)

target_include_directories(${TARGET_LGRAPH_MONITOR} PUBLIC
         ${LGRAPH_ROOT_DIR}/include
         ${LGRAPH_ROOT_DIR}/src)

target_link_libraries(${TARGET_LGRAPH_MONITOR}
         lgraph_client_cpp_rpc
         libprometheus-cpp-pull.a
         libprometheus-cpp-core.a)

############### lgraph_backup ######################

set(TARGET_LGRAPH_BACKUP lgraph_backup)

add_executable(${TARGET_LGRAPH_BACKUP} lgraph_backup.cpp)
target_link_libraries(${TARGET_LGRAPH_BACKUP} lgraph_server_lib)

############### lgraph_warmup ###################

set(TARGET_LGRAPH_WARMUP lgraph_warmup)

add_executable(${TARGET_LGRAPH_WARMUP} lgraph_warmup.cpp)
target_link_libraries(${TARGET_LGRAPH_WARMUP} lgraph_server_lib)

############### lgraph_peek ######################

set(TARGET_LGRAPH_PEEK lgraph_peek)

add_executable(${TARGET_LGRAPH_PEEK} lgraph_peek.cpp)
target_link_libraries(${TARGET_LGRAPH_PEEK} lgraph_server_lib)

############### lgraph_validate ######################

set(TARGET_LGRAPH_VALIDATE lgraph_validate)

add_executable(${TARGET_LGRAPH_VALIDATE} lgraph_validate.cpp)
target_link_libraries(${TARGET_LGRAPH_VALIDATE} lgraph_server_lib)

############### lgraph_export ######################

set(TARGET_LGRAPH_EXPORT lgraph_export)

add_executable(${TARGET_LGRAPH_EXPORT} lgraph_export.cpp)
target_link_libraries(${TARGET_LGRAPH_EXPORT} lgraph_server_lib)

############### lgraph_binlog ######################

set(TARGET_LGRAPH_BINLOG lgraph_binlog)

add_executable(${TARGET_LGRAPH_BINLOG} lgraph_binlog.cpp
               ${LGRAPH_ROOT_DIR}/src/client/cpp/rpc/lgraph_rpc_client.cpp
               ${LGRAPH_ROOT_DIR}/src/client/cpp/restful/rest_client.cpp)
target_link_libraries(${TARGET_LGRAPH_BINLOG} lgraph_server_lib)

set(LGRAPH_TOOLKITS ${TARGET_LGRAPH_IMPORT} ${TARGET_LGRAPH_BACKUP} ${TARGET_LGRAPH_WARMUP} ${TARGET_LGRAPH_PEEK} ${TARGET_LGRAPH_EXPORT} ${TARGET_LGRAPH_BINLOG} ${TARGET_LGRAPH_MONITOR})

# install
install(TARGETS ${LGRAPH_TOOLKITS}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib64)


