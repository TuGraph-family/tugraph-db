find_package(PythonLibs 3 REQUIRED)

set(TARGET_PYTHON_API lgraph_python_api)

add_library(${TARGET_PYTHON_API} SHARED
        python/python_api.cpp
        plugin/plugin_context.cpp)
target_include_directories(${TARGET_PYTHON_API} PRIVATE
        ${PYTHON_INCLUDE_DIRS})

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(${TARGET_PYTHON_API} PUBLIC
            ${PYTHON_LIBRARIES}
            lgraph)
else ()
    target_link_libraries(${TARGET_PYTHON_API} PUBLIC
            rt
            lgraph)
endif ()
