find_package(PythonLibs 3 REQUIRED)

add_library(lgraph_python SHARED
        python/python_api.cpp
        plugin/plugin_context.cpp)
target_include_directories(lgraph_python PRIVATE
        ${PYTHON_INCLUDE_DIRS})

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(lgraph_python PUBLIC
            ${PYTHON_LIBRARIES}
            lgraph)
else ()
    target_link_libraries(lgraph_python PUBLIC
            rt
            lgraph)
endif ()

set_target_properties(lgraph_python
        PROPERTIES PREFIX "")
