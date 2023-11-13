include(FindPackageHandleStandardArgs)

if ("$ENV{OpenSSL_DIR}" STREQUAL "")
    set(OpenSSL_ROOT ${TUGRAPH_THIRD_PARTY_DIR})
else()
    set(OpenSSL_ROOT "$ENV{OpenSSL_DIR}")
endif()

foreach(component ssl crypto)
    find_path(OpenSSL_${component}_INCLUDE_DIR openssl/ssl.h
        PATHS ${OpenSSL_ROOT}
        PATH_SUFFIXES include
        NO_DEFAULT_PATH)

    find_library(OpenSSL_${component}_LIBRARY
        NAMES lib${component}.a
        PATHS ${OpenSSL_ROOT}
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH)
endforeach()

find_package_handle_standard_args(OpenSSL
    DEFAULT_MSG
    OpenSSL_ssl_INCLUDE_DIR
    OpenSSL_ssl_LIBRARY
    OpenSSL_crypto_INCLUDE_DIR
    OpenSSL_crypto_LIBRARY)

if(OpenSSL_FOUND)
    add_library(OpenSSL::crypto STATIC IMPORTED)
    set_target_properties(OpenSSL::crypto
        PROPERTIES
        IMPORTED_LOCATION ${OpenSSL_crypto_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${OpenSSL_crypto_INCLUDE_DIR})
    add_library(OpenSSL::ssl STATIC IMPORTED)
    set_target_properties(OpenSSL::ssl
        PROPERTIES
        IMPORTED_LOCATION ${OpenSSL_ssl_LIBRARY}
        INTERFACE_INCLUDE_DIRECTORIES ${OpenSSL_ssl_INCLUDE_DIR}
        INTERFACE_LINK_LIBRARIES "OpenSSL::crypto")
endif()
