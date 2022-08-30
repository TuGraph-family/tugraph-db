find_package(PythonLibs 3 REQUIRED)

add_library(python_client SHARED
        client/python/rpc/client.cpp)

target_include_directories(python_client PRIVATE
        ${PYTHON_INCLUDE_DIRS}
        ${LGRAPH_INCLUDE_DIR})

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(python_client PUBLIC
            ${PYTHON_LIBRARIES}
            liblgraph_rpc_client)
else ()
    target_link_libraries(python_client PUBLIC
            liblgraph_rpc_client
            rt
            lgraph)
endif ()

set_target_properties(python_client
        PROPERTIES PREFIX "")
