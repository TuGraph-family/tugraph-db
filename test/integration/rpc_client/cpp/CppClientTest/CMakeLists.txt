cmake_minimum_required(VERSION 3.5)
PROJECT(CLIENTTEST)

SET(SRC_LIST client_test.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(INCLUDE_PATH ${CLIENTTEST_SOURCE_DIR}/../../../../../include)
set(LABRARY_PATH ${CLIENTTEST_SOURCE_DIR}/../../../../../build/output)



INCLUDE_DIRECTORIES(${INCLUDE_PATH})
LINK_DIRECTORIES(${LABRARY_PATH})

ADD_EXECUTABLE(clienttest ${SRC_LIST})

TARGET_LINK_LIBRARIES(clienttest lgraph_client_cpp_rpc)
