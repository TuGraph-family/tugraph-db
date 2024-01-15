add_subdirectory(parser)

set(ISO_GQL_SRC
    GQLAstVisitor.cpp
    GEAXErrorCode.cpp
    ../common/FollyMallocImpl.cpp
)

add_library(geax_isogql
    ${ISO_GQL_SRC}
)
target_compile_features(geax_isogql PRIVATE cxx_std_17)
if (GEAX_ENABLE_GLOG)
    target_compile_definitions(geax_isogql PUBLIC ENABLE_GLOG=1)
endif()
target_link_libraries(geax_isogql
    PUBLIC
        isogql_parser
        Antlr4::runtime
)

if (ENABLE_ASAN)
    target_compile_definitions(geax_isogql PUBLIC FOLLY_SANITIZE)
else()
    if (GEAX_ENABLE_JEMALLOC)
        target_link_libraries(geax_isogql PUBLIC Jemalloc::jemalloc)
        target_compile_definitions(geax_isogql PUBLIC USE_JEMALLOC)
        target_compile_definitions(geax_isogql PUBLIC FOLLY_USE_JEMALLOC)
    else()
        target_compile_definitions(geax_isogql PUBLIC FOLLY_ASSUME_NO_JEMALLOC)
    endif()
    if (GEAX_ENABLE_TCMALLOC)
        target_compile_definitions(geax_isogql PUBLIC USE_TCMALLOC)
    else()
        target_compile_definitions(geax_isogql PUBLIC FOLLY_ASSUME_NO_TCMALLOC)
    endif()
endif()

if (GEAX_ENABLE_GLOG)
    target_link_libraries(geax_isogql PUBLIC GLog::glog)
endif()

target_include_directories(geax_isogql
    INTERFACE
        ${GEAX_PROJECT_DIR}/include
)

if(GEAX_INSTALL)
    install(
        TARGETS
            geax_isogql
        LIBRARY DESTINATION lib
    )
endif()
