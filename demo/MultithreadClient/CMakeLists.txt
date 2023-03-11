cmake_minimum_required(VERSION 3.5)
PROJECT(CLIENTDEMO)

SET(SRC_LIST multi_thread_client.cpp
        client.cpp
        cypher_sender.cpp
        load_plugin.cpp
        delete_plugin.cpp
        plugin_sender.cpp)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g2 -ggdb")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(INCLUDE_PATH ${CLIENTDEMO_SOURCE_DIR}/../../include)
set(LABRARY_PATH ${CLIENTDEMO_SOURCE_DIR}/../../build/output)


INCLUDE_DIRECTORIES(${INCLUDE_PATH})
LINK_DIRECTORIES(${LABRARY_PATH})

ADD_EXECUTABLE(multi_thread_client ${SRC_LIST})

TARGET_LINK_LIBRARIES(multi_thread_client lgraph_client_cpp_rpc pthread)
