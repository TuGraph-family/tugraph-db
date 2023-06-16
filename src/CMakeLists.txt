cmake_minimum_required(VERSION 3.1)
project(TuGraphSrc C CXX)

set(LGRAPH_SRC_DIR ${PROJECT_SOURCE_DIR})

include(BuildLGraphApi.cmake)
include(BuildPythonApi.cmake)
include(BuildLGraphServer.cmake)
include(BuildCypherLib.cmake)
include(BuildClients.cmake)
include(BuildCythonExtension.cmake)

install(TARGETS lgraph lgraph_python_api lgraph_server lgraph_client_cpp_rpc lgraph_client_cpp_rest lgraph_client_python lgraph_db_python
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib64)
install(DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/restful/server/resource
        DESTINATION share/lgraph)
install(DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}/python/FMA_shell
        DESTINATION share/)
install(FILES
        ${CMAKE_CURRENT_LIST_DIR}/python/lgraph_task_runner.py
        ${CMAKE_CURRENT_LIST_DIR}/../plugins/lgraph_plugin.py
        DESTINATION bin)
install(FILES
        ${CMAKE_CURRENT_LIST_DIR}/../release/local/etc/lgraph.json
        DESTINATION etc)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../include
        DESTINATION ./)

add_custom_command(TARGET lgraph_server
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_LIST_DIR}/python/lgraph_task_runner.py
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lgraph_task_runner.py)
add_custom_command(TARGET lgraph_server
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_LIST_DIR}/restful/server/resource
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource)
add_custom_command(TARGET lgraph_server
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_LIST_DIR}/server/lgraph_daemon.json
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lgraph_daemon.json)
add_custom_command(TARGET lgraph_server
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_LIST_DIR}/server/lgraph_standalone.json
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lgraph_standalone.json)

add_custom_command(TARGET lgraph_server
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_LIST_DIR}/lucene/tugraph_lucene-1.0-SNAPSHOT.jar
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tugraph_lucene-1.0-SNAPSHOT.jar)
if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_custom_command(TARGET lgraph_server
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lgraph_python.dylib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lgraph_python.so)
    add_custom_command(TARGET lgraph_server
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/liblgraph.dylib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/liblgraph.so)
endif ()
