cmake_minimum_required(VERSION 3.13)
PROJECT(CLIENTTEST)

SET(SRC_LIST client_test.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(INCLUDE_PATH ${CLIENTTEST_SOURCE_DIR}/../../../../include)
set(LGRAPH_SRC_DIR ${CLIENTTEST_SOURCE_DIR}/../../../../src)
set(LABRARY_PATH ${CLIENTTEST_SOURCE_DIR}/../../../../build/output)



INCLUDE_DIRECTORIES(${INCLUDE_PATH} ${LGRAPH_SRC_DIR})
LINK_DIRECTORIES(${LABRARY_PATH})

ADD_EXECUTABLE(clienttest ${SRC_LIST})

TARGET_LINK_LIBRARIES(clienttest lgraph_client_cpp_rpc)
