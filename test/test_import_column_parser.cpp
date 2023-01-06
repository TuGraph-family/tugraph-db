/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "import/column_parser.h"

#include "./test_tools.h"
#include "./ut_utils.h"
using namespace fma_common;
using namespace lgraph;
using namespace import_v2;

#ifdef UT_ERR
#undef UT_ERR
#define UT_ERR() UT_LOG()
#endif

class TestImportColumnParser : public TuGraphTest {};

void WriteFiles() {
    auto WriteFilesImpl = [](std::string file_name, std::string data) {
        fma_common::OutputFmaStream stream;
        std::ifstream is(file_name);
        stream.Open(file_name);
        stream.Write(data.c_str(), data.size());
        stream.Close();
    };
    WriteFilesImpl("test.csv", "1, 2, 3");
    WriteFilesImpl("empty.csv", "");
}

std::vector<std::vector<FieldData>> ParseAllLines(const std::string& str,
                                                  const std::vector<FieldSpec>& specs,
                                                  size_t n_header = 0) {
    InputMemoryFileStream stream(str);
    ColumnParser parser(&stream, specs, 1 << 20, 1, n_header, false, ",");
    std::vector<std::vector<FieldData>> block;
    std::vector<std::vector<FieldData>> b;
    bool k = true;
    while (k) {
        k = parser.ReadBlock(b);
        block.insert(block.end(), b.begin(), b.end());
    }
    return block;
}

void TestParserPath() {
    std::string test_csv = "test.csv";
    std::string empty_csv = "empty.csv";
    std::string notexist_csv = "notexist.csv";
    size_t block_size = 345;
    size_t n_threads = 0;
    size_t n_header_lines = 1;

    {
        std::vector<FieldSpec> specs;
        ColumnParser parser(test_csv, specs, block_size, n_threads, n_header_lines, false, ",");
        ColumnParser parser1(empty_csv, specs, block_size, n_threads, n_header_lines, false, ",");
        UT_EXPECT_ANY_THROW(ColumnParser parser1(notexist_csv, specs, block_size, n_threads,
                                                 n_header_lines, false, ","));
    }

    std::vector<FieldSpec> specs = {
        FieldSpec("id", FieldType::INT32, false), FieldSpec("name", FieldType::STRING, false),
        FieldSpec("age", FieldType::FLOAT, true), FieldSpec("age16", FieldType::INT16, true)};
    std::string data_csv = "id, name, age, age16\n 0001,n0001,1.1,65536";
    UT_EXPECT_ANY_THROW(ParseAllLines(data_csv, specs, 1));

    data_csv = "id, name, age, age16\n 2147483648,n0001,1.1,636";
    UT_EXPECT_ANY_THROW(ParseAllLines(data_csv, specs, 1));
}

class ParseOneLineTester : public lgraph::import_v2::ColumnParser {
 public:
    ParseOneLineTester() : lgraph::import_v2::ColumnParser(0) {}

    std::tuple<size_t, bool> ParseOneLine(const std::string& input, std::vector<FieldData>& output,
                                          const std::vector<FieldSpec>& fs,
                                          const std::string& delim, bool forgiving) {
        field_specs_ = fs;
        forgiving_ = forgiving;
        delimiter_ = delim;
        CheckDelimAndResetErrors();
        auto parse = GetParseFunc();
        return parse(input.data(), input.data() + input.size(), output);
    }
};

TEST_F(TestImportColumnParser, ImportColumnParser) {
    {
        std::vector<FieldSpec> fs = {FieldSpec("f1", lgraph_api::INT32, false),
                                     FieldSpec("f2", lgraph_api::DOUBLE, true),
                                     FieldSpec("f3", lgraph_api::STRING, true)};
        ParseOneLineTester test;
        std::vector<FieldData> output;
        auto check_eq = [&](int i, double d, const std::string& s) {
            UT_EXPECT_EQ(output[0], FieldData::Int32(i));
            UT_EXPECT_EQ(output[1], FieldData::Double(d));
            UT_EXPECT_EQ(output[2], FieldData::String(s));
        };
        size_t size;
        bool success;
        MarkTestBegin("GetParseFunc()");
        {
            MarkTestBegin("illegal delimiter");
            UT_EXPECT_THROW(test.ParseOneLine("1\002\0032\002\0033\n", output, fs, "\r", false),
                            lgraph::InputError);
            UT_EXPECT_THROW(test.ParseOneLine("1\002\0032\002\0033\n", output, fs, "\n", false),
                            lgraph::InputError);
            UT_EXPECT_THROW(test.ParseOneLine("1\002\0032\002\0033\n", output, fs, "x\n", false),
                            lgraph::InputError);
        }
        {
            MarkTestBegin("parsing CSV");
            {
                std::tie(size, success) = test.ParseOneLine("1,2,3 4 5\n", output, fs, ",", false);
                check_eq(1, 2, "3 4 5");
                std::tie(size, success) = test.ParseOneLine("1,2,3\n", output, fs, ",", false);
                check_eq(1, 2, "3");
                UT_EXPECT_EQ(size, 6);
                std::tie(size, success) = test.ParseOneLine(" 1,2,3 \n", output, fs, ",", false);
                check_eq(1, 2, "3");
                UT_EXPECT_EQ(size, 8);
                std::tie(size, success) =
                    test.ParseOneLine("\002 1,2,3\nabc", output, fs, ",", false);
                check_eq(1, 2, "3");
                UT_EXPECT_EQ(size, 8);
                std::tie(size, success) =
                    test.ParseOneLine("1,2,\"3,4,5\"\n", output, fs, ",", false);
                check_eq(1, 2, "3,4,5");
                UT_EXPECT_ANY_THROW(test.ParseOneLine("\002 1,2ab,3\nabc", output, fs, ",", false));
            }
        }
        {
            MarkTestBegin("parsing with single char delim");
            std::tie(size, success) = test.ParseOneLine("1 2 3\n", output, fs, " ", false);
            check_eq(1, 2, "3");
            UT_EXPECT_EQ(size, 6);
            std::tie(size, success) = test.ParseOneLine(" 1\t2\t3 \n", output, fs, "\t", false);
            check_eq(1, 2, "3");
            UT_EXPECT_EQ(size, 8);
            std::tie(size, success) = test.ParseOneLine("\002 1|2|3\nabc", output, fs, "|", false);
            check_eq(1, 2, "3");
            UT_EXPECT_EQ(size, 8);
            std::tie(size, success) =
                test.ParseOneLine("\002 1|2|\"3\"\"\"\nabc", output, fs, "|", false);
            check_eq(1, 2, "3\"");
            UT_EXPECT_EQ(size, 12);
            UT_EXPECT_ANY_THROW(test.ParseOneLine("\002 1|2|3|\nabc", output, fs, ",", false));
        }
        {
            MarkTestBegin("parsing with two char delim");
            std::tie(size, success) =
                test.ParseOneLine("1\002\0032\002\0033\n", output, fs, "\002\003", false);
            check_eq(1, 2, "3");
            UT_EXPECT_EQ(size, 8);
            std::tie(size, success) = test.ParseOneLine("1!@2!@3", output, fs, "!@", false);
            check_eq(1, 2, "3");
            UT_EXPECT_EQ(size, 7);
            std::tie(size, success) = test.ParseOneLine("1!@2!@\"3!@\"", output, fs, "!@", false);
            check_eq(1, 2, "3!@");
            UT_EXPECT_ANY_THROW(test.ParseOneLine("1!@2!@3!@", output, fs, "!@", false));
            UT_EXPECT_ANY_THROW(test.ParseOneLine("1!@2", output, fs, "!@", false));
        }
        {
            MarkTestBegin("parsing with general string delim");
            std::tie(size, success) = test.ParseOneLine("1kkk2kkk3\n", output, fs, "kkk", false);
            check_eq(1, 2, "3");
            UT_EXPECT_EQ(size, 10);
            std::tie(size, success) =
                test.ParseOneLine("1kkk2kkk\"3kkk3\"\n", output, fs, "kkk", false);
            check_eq(1, 2, "3kkk3");
            std::tie(size, success) = test.ParseOneLine("1:-:2:-:\n", output, fs, ":-:", false);
            UT_EXPECT_TRUE(output[2].IsNull());
            UT_EXPECT_EQ(size, 9);
        }
    }
    {
        std::vector<FieldSpec> fs = {FieldSpec("id", FieldType::INT8, false),
                                     FieldSpec("", FieldType::NUL, true),
                                     FieldSpec("age", FieldType::INT16, true)};
        ParseOneLineTester test;
        std::vector<FieldData> output;
        auto check_eq = [&](int i, int j) {
            UT_EXPECT_EQ(output[0], FieldData::Int32(i));
            UT_EXPECT_EQ(output[1], FieldData::Int32(j));
        };
        size_t size;
        bool success;
        MarkTestBegin("parsing with delim with SKIP");
        std::tie(size, success) = test.ParseOneLine("1,3,2\n", output, fs, ",", false);
        check_eq(1, 2);
        std::tie(size, success) = test.ParseOneLine("1,\"2  ,  ,, \",2\n", output, fs, ",", false);
        check_eq(1, 2);
        std::tie(size, success) =
            test.ParseOneLine("1||\"2  ,  ||, \"||2\n", output, fs, "||", false);
        check_eq(1, 2);
        std::tie(size, success) =
            test.ParseOneLine("1kkk\"3kkk3\"kkk2\n", output, fs, "kkk", false);
        check_eq(1, 2);
        std::tie(size, success) =
            test.ParseOneLine("1,\"2  , \"\" ,, \",2\n", output, fs, ",", false);
        check_eq(1, 2);
        std::tie(size, success) =
            test.ParseOneLine("1||\"2  ,\"\"  ||, \"\"\"||2\n", output, fs, "||", false);
        check_eq(1, 2);
        std::tie(size, success) =
            test.ParseOneLine("1kkk\"3kkk3\"\"\"kkk2\n", output, fs, "kkk", false);
        check_eq(1, 2);
        std::tie(size, success) = test.ParseOneLine("1kkk3kkk2\n", output, fs, "kkk", false);
        check_eq(1, 2);
    }

    std::vector<FieldSpec> specs = {FieldSpec("id", FieldType::INT8, false),
                                    FieldSpec("name", FieldType::STRING, false),
                                    FieldSpec("age", FieldType::FLOAT, true)};
    {
        MarkTestBegin("ColumnParser");
        std::string data_csv =
            "id, name, age\n 0001,n0001,1.1\n 0002,n0002,2.2\n 0003,n0003,\n 0004,n0004,4.4";
        auto block = ParseAllLines(data_csv, specs, 1);
        for (size_t i = 1; i <= 4; i++) {
            UT_EXPECT_EQ(block[i - 1][0].AsInt8(), i);
            std::string name = UT_FMT("n000{}", i);
            UT_EXPECT_EQ(block[i - 1][1].AsString(), name);
        }
        UT_EXPECT_LT(fabs(block[0][2].AsFloat() - 1.1), 1e-6);
        UT_EXPECT_LT(fabs(block[1][2].AsFloat() - 2.2), 1e-6);
        UT_EXPECT_EQ(block[2][2].type, FieldType::NUL);
        UT_EXPECT_LT(fabs(block[3][2].AsFloat() - 4.4), 1e-6);
    }
    {
        UT_LOG() << "Testing empty id in csv";
        UT_EXPECT_ANY_THROW(ParseAllLines(",name2,2\n", specs));
        UT_LOG() << "Testing unclean";
        UT_EXPECT_ANY_THROW(ParseAllLines("0001,n0001,1.1,\n", specs));
        UT_LOG() << "Testing FailToParse";
        UT_EXPECT_ANY_THROW(ParseAllLines("0002,n0002,2.2a", specs));
        UT_LOG() << "Testing MissingDelimiter";
        UT_EXPECT_ANY_THROW(ParseAllLines("0001,n0001", specs));
        UT_LOG() << "Testing OutOfBound";
        UT_EXPECT_ANY_THROW(ParseAllLines("257,n0001,", specs));
    }
    {
        std::vector<FieldSpec> specs = {FieldSpec("", FieldType::FLOAT, true),
                                        FieldSpec("id", FieldType::INT8, false),
                                        FieldSpec("name", FieldType::STRING, false)};
        auto block = ParseAllLines(",1,\006n0234\003\nnothing,2,n324\n",
                                   specs);  // \006 and \003 will be skipped
        UT_EXPECT_EQ(block[0].size(), 2);   // SKIP column is not returned in the result
        UT_EXPECT_EQ(block[0][0].AsInt8(), 1);
        UT_EXPECT_EQ(block[0][1].AsString(), "n0234");
        UT_EXPECT_EQ(block[1].size(), 2);  // SKIP column is not returned in the result
        UT_EXPECT_EQ(block[1][0].AsInt8(), 2);
        UT_EXPECT_EQ(block[1][1].AsString(), "n324");
    }

    {
        WriteFiles();
        TestParserPath();
    }

    {
        MarkTestBegin("'forgiving' option");
        const std::string in =
            "123, string, not_double, extra\n"
            "123, name, column3";
        {
            std::vector<FieldSpec> field_specs = {
                FieldSpec("id", FieldType::INT16, false),
                FieldSpec("name", FieldType::STRING, false),
                FieldSpec("column3", FieldType::STRING, false),
            };
            {
                std::vector<std::vector<FieldData>> block;
                InputMemoryFileStream ins(in);
                ColumnParser parser(&ins, field_specs, 1024, 1, 0, false, ",");
                UT_EXPECT_ANY_THROW(parser.ReadBlock(block));
            }
            {
                std::vector<std::vector<FieldData>> block;
                InputMemoryFileStream ins(in);
                ColumnParser parser(&ins, field_specs, 1024, 1, 0, true, ",");
                UT_EXPECT_TRUE(parser.ReadBlock(block));
                UT_EXPECT_EQ(block.size(), 1);
                UT_EXPECT_EQ(block[0][2].AsString(), "column3");
            }
        }
        {
            std::vector<FieldSpec> field_specs = {
                FieldSpec("id", FieldType::INT16, false),
                FieldSpec("name", FieldType::STRING, false),
                FieldSpec("column3", FieldType::DOUBLE, false),
            };
            {
                std::vector<std::vector<FieldData>> block;
                InputMemoryFileStream ins(in);
                ColumnParser parser(&ins, field_specs, 1024, 1, 0, true, ",");
                UT_EXPECT_TRUE(!parser.ReadBlock(
                    block));  // only one block, and is empty, so false is returned
                UT_EXPECT_EQ(block.size(), 0);
            }
        }
    }
}
