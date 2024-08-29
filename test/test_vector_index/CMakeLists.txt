# unit test cmake
cmake_minimum_required(VERSION 3.10)
project(VectorIndexUnitTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O0")

include_directories(../../src ../../include /usr/local/include /usr/local/lib/)

find_path(GTEST_HEADER NAMES gtest/gtest.h)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Boost REQUIRED COMPONENTS log system thread filesystem)

set(GTEST_LIB libgtest.a)
set(GTEST_MAIN_LIB libgtest_main.a)

add_executable(vector_index_unit_test
        test_faiss_index.cpp
        test_vsag_index.cpp
        ../../src/core/Vsag_HNSW.cpp
        ../../src/core/Faiss_IVF_FLAT.cpp
        ../../src/core/vector_index_layer.cpp
        ../../src/core/vector_index_counter.cpp
)

target_link_libraries(vector_index_unit_test
        vsag
        /opt/OpenBLAS/lib/libopenblas.a
        libfaiss_avx2.a
        /usr/local/lib/libboost_log.a
        /usr/local/lib/libboost_system.a
        /usr/local/lib/libboost_thread.a
        ${Boost_LIBRARIES}
        ${GTEST_LIB}
        ${GTEST_MAIN_LIB}
        pthread
        gfortran
        m
        mkl_intel_lp64
        mkl_core
        mkl_sequential
        iomp5
        dl
)