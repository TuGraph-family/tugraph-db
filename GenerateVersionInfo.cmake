cmake_minimum_required(VERSION 3.1)

# get the git commit version if available and write it in outfile
function(GenerateVersionInfo ver_major ver_minor ver_patch infile outfile)
    if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
        # get GIT_BRANCH
        execute_process(
                COMMAND git rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        # get GIT_COMMIT_HASH
        execute_process(
                COMMAND git log -1 --format=%h
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_COMMIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else (EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(GIT_BRANCH "")
        set(GIT_COMMIT_HASH "")
    endif (EXISTS "${CMAKE_SOURCE_DIR}/.git")

    set(CXX_COMPILER_ID ${CMAKE_CXX_COMPILER_ID})
    set(CXX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    set(BUILD_TYPE ${CMAKE_BUILD_TYPE})

    message(STATUS "Generating ${outfile}")
    message(STATUS "  Main Version: ${ver_major}.${ver_minor}.${ver_patch}")
    message(STATUS "  Main git current branch: ${GIT_BRANCH}")
    message(STATUS "  Main git commit hash: ${GIT_COMMIT_HASH}")
    message(STATUS "  GCC version: ${CXX_COMPILER_ID} ${CXX_COMPILER_VERSION}")

    configure_file(${infile} ${outfile})
endfunction(GenerateVersionInfo)
