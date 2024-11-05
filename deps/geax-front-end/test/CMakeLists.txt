include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(isogql_visitor_dumper_test GQLVisitorDumperTest.cpp)
target_compile_definitions(isogql_visitor_dumper_test PRIVATE cxx_std_17)
target_link_libraries(isogql_visitor_dumper_test
    PRIVATE
        geax_isogql
        GTest::gtest
        GTest::gtest_main
)

add_executable(isogql_dml_parser_test GQLDMLParserTest.cpp GQLParserTest.cpp)
target_compile_features(isogql_dml_parser_test PRIVATE cxx_std_17)
target_compile_definitions(isogql_dml_parser_test
    PRIVATE
        GTEST_HAS_TR1_TUPLE=0
)
target_link_libraries(isogql_dml_parser_test
    PRIVATE
        geax_isogql
        GTest::gtest
        GTest::gtest_main
)

add_executable(isogql_logical_operator_test GQLLogicalOperatorTest.cpp)
target_compile_features(isogql_logical_operator_test PRIVATE cxx_std_17)
target_link_libraries(isogql_logical_operator_test
    PRIVATE
        geax_isogql
        GTest::gtest
        GTest::gtest_main
)

set_target_properties(
    isogql_visitor_dumper_test isogql_dml_parser_test isogql_logical_operator_test
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin/test
)
