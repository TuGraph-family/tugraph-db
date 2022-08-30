cmake_minimum_required(VERSION 3.2)
project(TuGraph C CXX)

function(add_standalone APP)
    add_executable(${APP}_standalone cpp/${APP}_core.cpp cpp/${APP}_standalone.cpp)
    target_link_libraries(${APP}_standalone lgraph_server_lib)
    set_target_properties( ${APP}_standalone PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
endfunction()

function(add_embed APP)
    add_executable(${APP}_embed cpp/${APP}_core.cpp cpp/${APP}_procedure.cpp embed_main.cpp)
    target_link_libraries(${APP}_embed lgraph_server_lib)
    set_target_properties( ${APP}_embed PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/algo")
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
