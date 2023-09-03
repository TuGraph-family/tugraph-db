include(FindPackageHandleStandardArgs)

if ("$ENV{GLog_DIR}" STREQUAL "")
    set(GLog_ROOT ${GEAX_THIRD_PARTY_DIR})
else()
    set(GLog_ROOT "$ENV{GLog_DIR}")
endif()


find_path(GLog_INCLUDE_DIR glog/logging.h
    PATHS ${GLog_ROOT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

find_library(GLog_LIBRARY
    NAMES libglog.a
    PATHS ${GLog_ROOT}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH)

find_package_handle_standard_args(GLog DEFAULT_MSG GLog_INCLUDE_DIR GLog_LIBRARY)


if(GLog_FOUND)
    add_library(GLog::glog STATIC IMPORTED)
    set_target_properties(GLog::glog
        PROPERTIES
        IMPORTED_LOCATION ${GLog_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${GLog_INCLUDE_DIR})
endif()
