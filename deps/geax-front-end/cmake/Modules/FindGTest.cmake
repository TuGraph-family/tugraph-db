include(FindPackageHandleStandardArgs)

if ("$ENV{GTest_DIR}" STREQUAL "")
    set(GTest_ROOT ${GEAX_THIRD_PARTY_DIR})
else()
    set(GTest_ROOT "$ENV{GTest_DIR}")
endif()

foreach(component gtest gtest_main)
    find_path(GTest_${component}_INCLUDE_DIR gtest/gtest.h
    PATHS ${GTest_ROOT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

    find_library(GTest_${component}_LIBRARY
    NAMES "lib${component}.a"
    PATHS ${GTest_ROOT}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
endforeach()


foreach(component gmock gmock_main)
    find_path(GTest_${component}_INCLUDE_DIR gmock/gmock.h
    PATHS ${GTest_ROOT}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

    find_library(GTest_${component}_LIBRARY
    NAMES "lib${component}.a"
    PATHS ${GTest_ROOT}
    PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
endforeach()




find_package_handle_standard_args(GTest
    DEFAULT_MSG
    GTest_gtest_INCLUDE_DIR
    GTest_gtest_LIBRARY
    GTest_gtest_main_INCLUDE_DIR
    GTest_gtest_main_LIBRARY
    GTest_gmock_INCLUDE_DIR
    GTest_gmock_LIBRARY
    GTest_gmock_main_INCLUDE_DIR
    GTest_gmock_main_LIBRARY)


if(GTest_FOUND)
    foreach(component gtest gtest_main gmock gmock_main)
        add_library(GTest::${component} STATIC IMPORTED)
        set_target_properties(GTest::${component}
            PROPERTIES
            IMPORTED_LOCATION ${GTest_${component}_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${GTest_${component}_INCLUDE_DIR}
            INTERFACE_LINK_LIBRARIES "Threads::Threads")
    endforeach()
endif()
