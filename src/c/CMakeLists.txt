add_library(lgraph-cbindings STATIC c.cpp)
target_include_directories(lgraph-cbindings PRIVATE ${LGRAPH_INCLUDE_DIR})
target_link_libraries(lgraph-cbindings PRIVATE lgraph)