include(FindPackageHandleStandardArgs)

if ("$ENV{GFlags_DIR}" STREQUAL "")
    set(GFlags_ROOT ${GEAX_THIRD_PARTY_DIR})
elseif()
    set(GFlags_ROOT "$ENV{GFlags_DIR}")
endif()


find_path(GFlags_INCLUDE_DIR gflags/gflags.h
    PATHS ${GFlags_ROOT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

find_library(GFlags_LIBRARY
    NAMES libgflags.a
    PATHS ${GFlags_ROOT}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH)

find_package_handle_standard_args(GFlags DEFAULT_MSG GFlags_INCLUDE_DIR GFlags_LIBRARY)


if(GFlags_FOUND)
    add_library(GFlags::gflags STATIC IMPORTED)
    set_target_properties(GFlags::gflags
        PROPERTIES
        IMPORTED_LOCATION ${GFlags_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${GFlags_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES "Threads::Threads")
endif()
