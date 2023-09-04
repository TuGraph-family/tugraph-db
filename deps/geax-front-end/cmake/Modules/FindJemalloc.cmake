include(FindPackageHandleStandardArgs)

if ("$ENV{Jemalloc_DIR}" STREQUAL "")
    set(Jemalloc_ROOT ${GEAX_THIRD_PARTY_DIR})
else()
    set(Jemalloc_ROOT "$ENV{Jemalloc_DIR}")
endif()

if (NOT JEMALLCO_SUB_DIR)
    set(Jemalloc_lib_path "")
else()
    set(Jemalloc_lib_path "jemalloc/${JEMALLCO_SUB_DIR}")
endif()

find_path(Jemalloc_INCLUDE_DIR jemalloc/jemalloc.h
    PATHS ${Jemalloc_ROOT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

find_library(Jemalloc_LIBRARY
    NAMES libjemalloc.a
    PATHS ${Jemalloc_ROOT}
    PATH_SUFFIXES lib/${Jemalloc_lib_path}
    NO_DEFAULT_PATH)

find_package_handle_standard_args(Jemalloc
    DEFAULT_MSG
    Jemalloc_INCLUDE_DIR
    Jemalloc_LIBRARY)


if(Jemalloc_FOUND)
    add_library(Jemalloc::jemalloc STATIC IMPORTED)
    set_target_properties(Jemalloc::jemalloc
        PROPERTIES
        IMPORTED_LOCATION ${Jemalloc_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${Jemalloc_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES "Threads::Threads;dl")
endif()
