cmake_minimum_required(VERSION 3.1)

set(TARGET_BOLT_LIB bolt)

set(BOLT_SRC
    bolt/connection.cpp
    bolt/hydrator.cpp
    bolt/pack.cpp
    bolt_ha/logger.cpp
    bolt_ha/raft_log_store.cpp
    bolt_ha/raft_driver.cpp
    lgraph_api/lgraph_exceptions.cpp)

add_library(${TARGET_BOLT_LIB} STATIC ${BOLT_SRC})

target_include_directories(${TARGET_BOLT_LIB} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${LGRAPH_INCLUDE_DIR}
        "${LGRAPH_ROOT_DIR}/deps"
)
