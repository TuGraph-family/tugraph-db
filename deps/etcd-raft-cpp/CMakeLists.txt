cmake_minimum_required(VERSION 3.13)
project(raft)
set(CMAKE_CXX_STANDARD 17)
#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=address -Wno-format-security -Wno-format-truncation")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-format-security -Wno-format-truncation")
set(Protobuf_USE_STATIC_LIBS ON)
include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(all_test
    test_main.cc
    raftpb/raft.pb.cc
    raftpb/confstate_test.cpp
    tracker/inflights_test.cpp
    tracker/progress_test.cpp
    util_test.cc
    storage_test.cc
    log_unstable_test.cpp
    log_test.cpp
    raft_test.cpp
    rawnode_test.cpp
    raft_snap_test.cpp
    raft_flow_control_test.cpp
    raft_paper_test.cpp
)
target_compile_definitions(all_test PRIVATE RAFT_TEST=1 RAFT_LOG=1)
target_link_libraries(all_test
    gtest.a
    protobuf.a
    pthread
)

