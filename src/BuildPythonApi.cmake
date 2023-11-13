find_package(PythonInterp 3)
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)

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
