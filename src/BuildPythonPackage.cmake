find_package(PythonLibs 3 REQUIRED)

add_library(lgraph_python SHARED
        python/python_api.cpp
        plugin/plugin_context.cpp
        cypher/execution_plan/execution_plan.cpp
        cypher/execution_plan/scheduler.cpp
        server/state_machine.cpp
        )
target_include_directories(lgraph_python PRIVATE
        ${PYTHON_INCLUDE_DIRS}
        ${DEPS_INCLUDE_DIR}/antlr4-runtime
        ${DEPS_INCLUDE_DIR}/cypher
	)

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(lgraph_python PUBLIC
            ${PYTHON_LIBRARIES}
            boost_system
            cpprest
            lgraph_server_lib
            )
else ()
    target_link_libraries(lgraph_python PUBLIC
            boost_system
            cpprest
            lgraph_server_lib
            )
endif ()

set_target_properties(lgraph_python
        PROPERTIES PREFIX "")
