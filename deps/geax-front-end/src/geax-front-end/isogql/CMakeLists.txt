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

target_link_libraries(geax_isogql
    PUBLIC
        isogql_parser
        Boost::boost
        Antlr4::runtime
)
if (ENABLE_ASAN)
    target_compile_definitions(geax_isogql PUBLIC FOLLY_SANITIZE=1)
endif()
if (GEAX_ENABLE_JEMALLOC AND NOT ENABLE_ASAN)
    target_link_libraries(geax_isogql PUBLIC Jemalloc::jemalloc)
    target_compile_definitions(geax_isogql PUBLIC USE_JEMALLOC=1)
else()
    target_compile_definitions(geax_isogql PUBLIC FOLLY_ASSUME_NO_JEMALLOC=1)
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
