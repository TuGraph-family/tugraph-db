# this file generates protobuf files from .proto
include(FindProtobuf)
if (NOT PROTOBUF_PROTOC_EXECUTABLE)
    get_filename_component(PROTO_LIB_DIR ${PROTOBUF_LIBRARY} DIRECTORY)
    set(PROTOBUF_PROTOC_EXECUTABLE "${PROTO_LIB_DIR}/../bin/protoc")
endif ()

set(PROTOBUF_LIBRARY libprotobuf.a)

function(GenerateProtobufCpp PATH_OUT SRCS HDRS)
    if (NOT ARGN)
        message(SEND_ERROR "Error: PROTOBUF_GENERATE_CPP() called without any proto files")
        return()
    endif ()

    if (PROTOBUF_GENERATE_CPP_APPEND_PATH)
        # Create an include path for each file specified
        foreach (FIL ${ARGN})
            get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
            get_filename_component(ABS_PATH ${ABS_FIL} DIRECTORY)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if (${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif ()
        endforeach ()
    else ()
        set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
    endif ()

    if (DEFINED PROTOBUF_IMPORT_DIRS)
        foreach (DIR ${PROTOBUF_IMPORT_DIRS})
            get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if (${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif ()
        endforeach ()
    endif ()

    set(${SRCS})
    set(${HDRS})
    foreach (FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        list(APPEND ${SRCS} "${PATH_OUT}/${FIL_WE}.pb.cc")
        list(APPEND ${HDRS} "${PATH_OUT}/${FIL_WE}.pb.h")

        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${PATH_OUT})

        add_custom_command(
                OUTPUT "${PATH_OUT}/${FIL_WE}.pb.cc" "${PATH_OUT}/${FIL_WE}.pb.h"
                COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
                ARGS --cpp_out=${PATH_OUT} ${_protobuf_include_path} ${ABS_FIL}
                DEPENDS ${ABS_FIL}
                COMMENT "Running C++ protocol buffer compiler on ${FIL}"
                VERBATIM)
    endforeach ()

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()