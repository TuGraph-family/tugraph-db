set(LGRAPH_VERSION_MAJOR 4)
set(LGRAPH_VERSION_MINOR 3)
set(LGRAPH_VERSION_PATCH 2)

# options
option(ENABLE_WALL "Enable all compiler's warning messages." ON)
if (ENABLE_WALL)
    message("Wall is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
else (ENABLE_WALL)
    message("Wall is disabled.")
endif (ENABLE_WALL)

option(USE_MOCK_KV "Use mock kv-store." OFF)
if (USE_MOCK_KV)
    message("MockKV is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLGRAPH_USE_MOCK_KV=1")
else (USE_MOCK_KV)
    message("MockKV is disabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLGRAPH_USE_MOCK_KV=0")
endif (USE_MOCK_KV)

option(ENABLE_VALGRIND "Enable Valgrind profiling." OFF)
if (ENABLE_VALGRIND)
    message("Valgrind is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_VALGRIND=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_VALGRIND=1")
else (ENABLE_VALGRIND)
    message("Valgrind is disabled.")
endif (ENABLE_VALGRIND)

option(ENABLE_SQL_IO "Enable SQL import/export." OFF)
if (ENABLE_SQL_IO)
    message("SQL import/export is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFMA_HAS_LIBMYSQL=1")
else (ENABLE_SQL_IO)
    message("SQL import/export is disabled.")
endif (ENABLE_SQL_IO)

option(ENABLE_ASAN "Enable Address Sanitizer." OFF)
if (ENABLE_ASAN)
    message("Address Sanitizer is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libasan")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -static-libasan ")
else (ENABLE_ASAN)
    message("Address Sanitizer is disabled.")
endif (ENABLE_ASAN)

option(ENABLE_PYTHON_PLUGIN "Enable Python plugin." ON)
if (ENABLE_ASAN)
    set(ENABLE_PYTHON_PLUGIN 0)
endif (ENABLE_ASAN)
if (ENABLE_PYTHON_PLUGIN)
    message("Python support is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLGRAPH_ENABLE_PYTHON_PLUGIN=1")
else (ENABLE_PYTHON_PLUGIN)
    message("Python support is disabled.")
endif (ENABLE_PYTHON_PLUGIN)

option(ENABLE_SHARE_DIR "Enable different server instances to share the same data directory." OFF)
if (ENABLE_SHARE_DIR)
    message("Data dir sharing is enabled. HA will work only in shared-dir mode.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLGRAPH_SHARE_DIR=1")
else (ENABLE_SHARE_DIR)
    message("Data dir sharing is disabled. HA will work in replication mode.")
endif (ENABLE_SHARE_DIR)

option(ENABLE_FULLTEXT_INDEX "Enable fulltext index." OFF)
if (ENABLE_FULLTEXT_INDEX)
    message("Fulltext index is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLGRAPH_ENABLE_FULLTEXT_INDEX=1")
else (ENABLE_FULLTEXT_INDEX)
    message("Fulltext index is disabled.")
endif (ENABLE_FULLTEXT_INDEX)

option(BUILD_JAVASDK "Build lgraph4jni.so for javasdk" OFF)
if (BUILD_JAVASDK)
    message("Build lgraph4jni.so for javasdk.")
endif (BUILD_JAVASDK)

option(BUILD_PROCEDURE "Build procedure & learn" ON)
if (BUILD_PROCEDURE)
    message("Build procedures.")
endif (BUILD_PROCEDURE)

option(WITH_TESTS "build with tests" ON)
if (WITH_TESTS)
    message("Build with tests.")
endif (WITH_TESTS)

# disable krb5
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENSSL_NO_KRB5=1")

# remove prefix in macro __FILE__
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmacro-prefix-map=${CMAKE_CURRENT_LIST_DIR}/=")

# Detect build type, fallback to release and throw a warning if use didn't specify any
if (NOT CMAKE_BUILD_TYPE)
    message(WARNING "Build type not set, falling back to RelWithDebInfo mode.
 To specify build type use:
 -DCMAKE_BUILD_TYPE=<mode> where <mode> is in (Debug, Release, Coverage, RelWithDebInfo).")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
            "Choose the type of build."
            FORCE)
endif (NOT CMAKE_BUILD_TYPE)
message("CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")

# coverage flags
SET(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O3")
SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS} -lgcov -O3")
MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE)

# support PRId64
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_FORMAT_MACROS")

# make sure cmake prefers static lib
IF (WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ELSE (WIN32)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF (WIN32)

## ---------------------------
## check compiler options
## ---------------------------
# check compiler version
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # require at least gcc 4.8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
        message(FATAL_ERROR "GCC is too old, requires GCC 5.0 or above.")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # CMAKE_CXX_COMPILER_ID might be AppleClang, use "MATCHES" instead of "STREQUAL"
    # require at least clang 3.3
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
        message(FATAL_ERROR "Clang is too old, requires Clang 3.3 or above.")
    endif ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_ASIO_HAS_STD_STRING_VIEW=1 -D__GNUC__=7 -D_GNU_SOURCE=1")
else ()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif ()

# check OpenMP
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# compiling rocksdb needs c++17
# check c++17
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
if (COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else ()
    message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif ()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GNU: static link libstdc++ and libgcc
# Clang: static link libc++
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    CHECK_CXX_COMPILER_FLAG("-static-libstdc++ -static-libgcc" COMPILER_SUPPORTS_STATIC_GCC)
    if (COMPILER_SUPPORTS_STATIC_GCC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
    else ()
        message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no static libgcc support. Please use a different C++ compiler.")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    CHECK_CXX_COMPILER_FLAG("-stdlib=libc++" COMPILER_SUPPORTS_STATIC_CLANG)
    if (COMPILER_SUPPORTS_STATIC_CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    else ()
        message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no static libc++ support. Please use a different C++ compiler.")
    endif ()
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    string(REPLACE " " ";" CMAKE_CXX_FLAG_LIST ${CMAKE_CXX_FLAGS})
    FOREACH (CXX_COMPILE_FLAG ${CMAKE_CXX_FLAG_LIST})
        ADD_COMPILE_OPTIONS($<$<COMPILE_LANGUAGE:CXX>:${CXX_COMPILE_FLAG}>)
    ENDFOREACH ()
endif ()
