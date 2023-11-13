cmake_minimum_required(VERSION 3.1)

# get the git commit version if available and write it in outfile
function(GenerateVersionInfo ver_major ver_minor ver_patch infile outfile)
    set(LGRAPH_VERSION_MAJOR ${ver_major})
    set(LGRAPH_VERSION_MINOR ${ver_minor})
    set(LGRAPH_VERSION_PATCH ${ver_patch})

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

    set(WEB_DIR ${CMAKE_SOURCE_DIR}/deps/tugraph-web)
    if (EXISTS "${WEB_DIR}/.git")
        # get WEB_GIT_COMMIT_HASH
        execute_process(
                COMMAND git log -1 --format=%h
                WORKING_DIRECTORY ${WEB_DIR}
                OUTPUT_VARIABLE WEB_GIT_COMMIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    else (EXISTS "${WEB_DIR}/.git")
        set(WEB_GIT_COMMIT_HASH "")
    endif (EXISTS "${WEB_DIR}/.git")

    set(CXX_COMPILER_ID ${CMAKE_CXX_COMPILER_ID})
    set(CXX_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})
    find_package(PythonInterp 3)
    find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)
    set(PYTHON_LIB_VERSION ${PYTHONLIBS_VERSION_STRING})

    message(STATUS "Generating ${outfile}")
    message(STATUS "  Main Version: ${LGRAPH_VERSION_MAJOR}.${LGRAPH_VERSION_MINOR}.${LGRAPH_VERSION_PATCH}")
    message(STATUS "  Main git current branch: ${GIT_BRANCH}")
    message(STATUS "  Main git commit hash: ${GIT_COMMIT_HASH}")
    message(STATUS "  Web git commit hash: ${WEB_GIT_COMMIT_HASH}")
    message(STATUS "  GCC version: ${CXX_COMPILER_ID} ${CXX_COMPILER_VERSION}")
    message(STATUS "  Python lib version: ${PYTHON_LIB_VERSION}")

    configure_file(${infile} ${outfile})
endfunction(GenerateVersionInfo)
