add_library(lgraph-cbinding STATIC c.cpp)
target_include_directories(lgraph-cbinding PRIVATE ${LGRAPH_INCLUDE_DIR})
target_link_libraries(lgraph-cbinding PRIVATE lgraph)