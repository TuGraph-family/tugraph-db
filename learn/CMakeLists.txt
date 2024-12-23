cmake_minimum_required(VERSION 3.13)
project(TuGraph C CXX)

function(add_extension2 APP)
    add_cython_target(${APP} ${CMAKE_SOURCE_DIR}/learn/procedures/${APP}.py CXX)
    add_library(${APP} MODULE ${APP})
    python_extension_module(${APP})
    target_link_libraries(${APP} lgraph libgomp.a dl)
    set_target_properties( ${APP} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
endfunction()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src/cython/)
find_package(PythonInterp 3)
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)
set(CYTHON_FLAGS "-I${CMAKE_SOURCE_DIR}/src/cython/ -I${CMAKE_SOURCE_DIR}/include/cython/" )

add_extension2(edge_sampling)
add_extension2(getdb)
add_extension2(negative_sampling)
add_extension2(neighbors_sampling)
add_extension2(random_walk)
add_extension2(node2vec_sampling)
