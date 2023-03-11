cmake_minimum_required(VERSION 3.2)
project(TuGraph C CXX)

set(LGRAPH_API_SRC 
        ${CMAKE_SOURCE_DIR}/src/lgraph_api/olap_base.cpp
        ${CMAKE_SOURCE_DIR}/src/lgraph_api/lgraph_utils.cpp
        ${CMAKE_SOURCE_DIR}/src/lgraph_api/olap_profile.cpp)

function(add_standalone APP)
    add_executable(${APP}_standalone cpp/${APP}_core.cpp cpp/${APP}_standalone.cpp ${LGRAPH_API_SRC})
    target_link_libraries(${APP}_standalone libstdc++fs.a libgomp.a dl)
    set_target_properties( ${APP}_standalone PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
    target_include_directories(${APP}_standalone PUBLIC ${CMAKE_SOURCE_DIR}/include
                                                        ${CMAKE_SOURCE_DIR}/src)
endfunction()

function(add_embed APP)
    add_executable(${APP}_embed cpp/${APP}_core.cpp cpp/${APP}_procedure.cpp embed_main.cpp)
    target_link_libraries(${APP}_embed lgraph libgomp.a crypto dl)
    set_target_properties( ${APP}_embed PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
endfunction()

function(add_embed2 APP)
    add_executable(${APP}_embed cpp/${APP}.cpp embed_main.cpp)
    target_link_libraries(${APP}_embed lgraph libgomp.a crypto dl)
    set_target_properties( ${APP}_embed PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
    target_include_directories(${APP}_embed PUBLIC ${CMAKE_SOURCE_DIR}/include)
endfunction()

function(add_extension APP)
    add_cython_target(${APP} cython/${APP}.py CXX)
    add_library(${APP} MODULE ${APP})
    python_extension_module(${APP})
    target_link_libraries(${APP} lgraph libgomp.a dl)
    set_target_properties( ${APP} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
endfunction()

add_standalone(bfs)
add_standalone(pagerank)
add_standalone(sssp)
add_standalone(wcc)
add_standalone(lcc)
add_standalone(lpa)

add_embed(bfs)
add_embed(pagerank)
add_embed(sssp)
add_embed(wcc)
add_embed(lcc)
add_embed(lpa)

add_embed2(khop_kth)
add_embed2(khop_within)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src/cython/)
find_package(PythonInterp 3)
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)
set(CYTHON_FLAGS " -I${CMAKE_SOURCE_DIR}/src/cython/")
add_extension(bfs)
add_extension(pagerank)
add_extension(sssp)
add_extension(wcc)
add_extension(lcc)
add_extension(lpa)
