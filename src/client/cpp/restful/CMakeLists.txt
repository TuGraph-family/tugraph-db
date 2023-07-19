cmake_minimum_required(VERSION 2.8)

project(restful_api)

set(DB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(DB_LIB_PATH "${DB_PATH}/build")
set(REST_SDK_PATH "${DB_PATH}/restful/cpprestsdk")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-I${REST_SDK_PATH}/Release/include \
    -I${DB_PATH}/fma-common -I${DB_PATH} -I${DB_PATH}/core \
    -std=c++17 -O0 -g ${CMAKE_CXX_FLAGS}")
set(EXTRA_LIBS "-lboost_system -lcrypto -lssl -L${DB_LIB_PATH}/lib -lcpprest")
set(EXTRA_LIBS "${EXTRA_LIBS} -L${DB_LIB_PATH} -llgraph -lpthread -static-libstdc++ -static-libgcc")
set(CMAKE_VERBOSE_MAKEFILE ON)

aux_source_directory(. DIR_SRCS)
add_executable(test_client ${DIR_SRCS})
target_link_libraries(test_client ${EXTRA_LIBS})
