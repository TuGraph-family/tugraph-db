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

#include <cstdlib>
#include <tuple>

#include "fma-common/configuration.h"
#include "fma-common/fma_stream.h"
#include "fma-common/logging.h"
#include "fma-common/text_writer.h"
#include "fma-common/text_parser.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

struct Edge {
    int64_t src;
    int64_t dst;
    int type;
};

int GetTypeWithString(const char *b, const char *e) { return 0; }

size_t ParseOneEdge(const char *beg, const char *end, Edge &e) {
    const char *p = beg;
    p += TextParserUtils::ParseInt64(p, end, e.src);
    p += TextParserUtils::ParseInt64(p, end, e.dst);
    while (p != end && (*p == ' ' || *p == '\t')) p++;  // skip blankspaces
    const char *type_beg = p;
    while (p != end && TextParserUtils::IsGraphical(*p)) p++;  // scan the type string
    e.type = GetTypeWithString(type_beg, p);
    return p - beg;
}

FMA_SET_TEST_PARAMS(TextParser, "--create true");

FMA_UNIT_TEST(TextParser) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    // the following should generate an out-of-range error
    /*
    char small;
    std::string str = "1234";
    TextParserUtils::ParseDigit(&str[0], &str[4], small);
    return 0;
    */

    {
        // test whether the SkipOneLine is faulty
        static const size_t n = 100;
        std::string l = "fullline\n";
        {
            OutputFmaStream tmp("tmp.txt");
            for (size_t i = 0; i < n; i++) {
                tmp.Write(l.data(), l.size());
            }
        }
        InputFmaStream stream("tmp.txt");
        std::function<std::tuple<size_t, bool>(const char *, const char *, std::string &)> func =
            [](const char *beg, const char *end, std::string &ret) -> std::tuple<size_t, bool> {
            const char *p = beg;
            while (p != end && !TextParserUtils::IsNewLine(*p)) p++;
            ret.assign(beg, p);
            return std::tuple<size_t, bool>(p - beg, p != beg);
        };
        auto parser = MakeTextParser<std::string>(stream, func, 1 << 20, 2, 1);
        std::string data;
        size_t rn = 0;
        while (parser->Read(data)) {
            rn++;
            FMA_UT_CHECK_EQ(data, "fullline");
        }
        FMA_UT_CHECK_EQ(rn, n - 1);
    }

    /* The following code shows how to parse a string into multiple fields */
    std::string line = "-rw-r--r--   1 hct supergroup      15429 2017-05-31 13:43 /LICENSE.txt";
    TextParserUtils::DropField _;
    char c = '0';
    int64_t fsize = 0;
    std::string fpath;

    TextParserUtils::ParseAsTuple(&line[0], &line[line.size()], _, c, _, _, fsize, _, _, fpath);
    FMA_UT_CHECK_EQ(c, '1');
    FMA_UT_CHECK_EQ(fsize, 15429);
    FMA_UT_CHECK_EQ(fpath, "/LICENSE.txt");

    /* The following code shows how to parse a Csv file */
    bool use_created_file = false;
    std::string file = "tmp.csv";
    int block_size = 1000;
    int n_parse_threads = 1;
    Configuration config;
    config.Add(use_created_file, "create", true).Comment("Use the created file");
    config.Add(file, "file", true).Comment("File to use");
    config.Add(block_size, "blockSize", true).Comment("Block size to use");
    config.Add(n_parse_threads, "parseThreads", true).Comment("Number of parsing threads to use");
    config.Parse(argc, argv);
    config.Finalize();
    double t1, t_write = 0, t_parse = 0, t_parse_tsv = 0;
    int64_t total_bytes = 0;

    // create the file ahead
    if (use_created_file) {
        OutputFmaStream csv_file(file);
        OutputFmaStream tsv_file(file + ".tsv");
        std::string header("src,dst,tag\n");
        csv_file.Write(header.data(), header.size());
        std::string theader("src dst tag\n");
        tsv_file.Write(theader.data(), theader.size());

        std::vector<int64_t> v1{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::vector<int64_t> v2{3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::string> v3{"s01", "s02", "s03", "s04", "s05",
                                    "s06", "s07", "s08", "s09", "s10"};
        t1 = GetTime();
        WriteCsv(csv_file, 2, block_size, v1, v2, v3);
        WriteTsv(tsv_file, 2, block_size, v1, v2, v3);
        total_bytes = csv_file.Size();
        csv_file.Close();
        tsv_file.Close();
        t_write = GetTime() - t1;
    }

    // now parse the csv file
    {
        t1 = GetTime();
        InputFmaStream stream(file, 0);
        auto text_parser = MakeCsvParser<int64_t, int64_t, TextParserUtils::_>(stream, block_size,
                                                                               n_parse_threads, 1);
        std::vector<decltype(text_parser)::element_type::ElementType> buf;
        size_t n = 0;
        try {
            while (text_parser->ReadBlock(buf)) {
                n += buf.size();
                if (use_created_file) {
                    int64_t ve1 = std::get<0>(buf[2]);
                    int64_t ve2 = std::get<1>(buf[2]);
                    FMA_UT_CHECK_EQ(ve1, 3) << "check ";
                    FMA_UT_CHECK_EQ(ve2, 5);
                }
            }
        } catch (std::exception &e) {
            LOG() << e.what();
        }
        t_parse = GetTime() - t1;
        LOG() << "Total lines: " << n;
        LOG() << "Time used: write " << t_write << ", parse " << t_parse;
        LOG() << (double)total_bytes / 1024 / 1024 / t_write << "MB/s, "
              << (double)stream.Size() / 1024 / 1024 / t_parse << "MB/s";
    }

    // now parse the tsv file
    {
        t1 = GetTime();
        InputFmaStream stream(file + ".tsv", 0);
        auto text_parser = MakeTsvParser<int64_t, int64_t, TextParserUtils::_>(stream, block_size,
                                                                               n_parse_threads, 1);
        std::vector<decltype(text_parser)::element_type::ElementType> buf;
        size_t n = 0;
        try {
            while (text_parser->ReadBlock(buf)) {
                n += buf.size();
                if (use_created_file) {
                    int64_t ve1 = std::get<0>(buf[2]);
                    int64_t ve2 = std::get<1>(buf[2]);
                    FMA_UT_CHECK_EQ(ve1, 3) << "check ";
                    FMA_UT_CHECK_EQ(ve2, 5);
                }
            }
        } catch (std::exception &e) {
            LOG() << e.what();
        }
        t_parse_tsv = GetTime() - t1;
        LOG() << "Total lines: " << n;
        LOG() << "Time used: write " << t_write << ", parse " << t_parse_tsv;
        LOG() << (double)total_bytes / 1024 / 1024 / t_write << "MB/s, "
              << (double)stream.Size() / 1024 / 1024 / t_parse << "MB/s";
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
