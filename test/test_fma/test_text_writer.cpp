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

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/fma_stream.h"
#include "fma-common/local_file_stream.h"
#include "fma-common/logging.h"
#include "fma-common/text_writer.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

struct Edge {
    int64_t srcId;
    int64_t dstId;
};

const std::string &GetVertexType(int t) {
    static std::string dummy = "dummy";
    return dummy;
}

void WriteOneEdge(std::string &buf, const Edge &edge, const int &type) {
    buf += ToString(edge.srcId);
    buf += "\t";
    buf += ToString(edge.dstId);
    buf += "\t";
    buf += GetVertexType(type);
}

FMA_SET_TEST_PARAMS(TextWriter, "-f tmpfile --type int -m csvWriter -s 15 -n 1025 -w 4 -c 1023",
                    "-f tmpfile --type int -m formatter",
                    "-f tmpfile --type int -m formatter -s 15 -n 1025 -w 4 -c 1023");

FMA_UNIT_TEST(TextWriter) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    ArgParser parser;
    parser.Add<std::string>("file,f").Comment("Output file path");
    parser.Add<std::string>("type,t")
        .Comment("Type of data to write")
        .SetPossibleValues({"int", "string"});
    parser.Add<std::string>("method,m")
        .Comment("Method to write text file")
        .SetPossibleValues({"fprintf", "ofstream", "csvWriter", "formatter"});
    parser.Add<int>("size,s").Comment("Size of the string").SetDefault(16);
    parser.Add<int>("nElements,n").Comment("Number of elements to write").SetDefault(1024);
    parser.Add<int>("nWriters,w").Comment("Number of cvsWriters to use").SetDefault(4);
    parser.Add<size_t>("chunkSize,c")
        .Comment("Size of each chunk when using csvWriter")
        .SetDefault(1 << 16);
    parser.Parse(argc, argv);
    parser.Finalize();

    std::string path = parser.GetValue("file");
    std::string type = parser.GetValue("type");
    std::string method = parser.GetValue("method");
    int size = parser.GetValue<int>("size");
    int n_elements = parser.GetValue<int>("nElements");
    size_t chunk_size = parser.GetValue<size_t>("chunkSize");

    int element_size = type == "string" ? size : sizeof(int);
    LOG() << "Writing file " << path << "\n"
          << "\tData type: " << type << "\n"
          << "\tSize of each element: " << element_size << "\n"
          << "\tTotal number of elements: " << n_elements;

    std::string str;
    int x = 198702;
    str.resize(size, 'a');

    double t1, t2;
    size_t total_bytes = 0;

    t1 = GetTime();
    if (method == "ofstream") {
        std::ofstream out(path);
        if (type == "string") {
            for (int i = 0; i < n_elements; i++) {
                out << str << "\n";
            }
        } else {
            for (int i = 0; i < n_elements; i++) {
                out << x << "\n";
            }
        }
        total_bytes = out.tellp();
    } else if (method == "fprintf") {
        FILE *file;
        file = fopen(path.c_str(), "w");
        if (type == "string") {
            for (int i = 0; i < n_elements; i++) {
                fprintf(file, "%s\n", str.c_str());
            }
        } else {
            for (int i = 0; i < n_elements; i++) {
                fprintf(file, "%d\n", x);
            }
        }
        total_bytes = fseek(file, 0L, SEEK_END);
        total_bytes = ftell(file);
        fclose(file);
    } else if (method == "csvWriter") {
        std::vector<int> vi(n_elements);
        for (size_t i = 0; i < vi.size(); i++) vi[i] = (int)i;
        std::vector<std::string> vs(n_elements, str);
        t1 = GetTime();
        OutputLocalFileStream out(path);
        int n_writers = parser.GetValue<int>("nWriters");
        WriteTsv(out, n_writers, chunk_size, vi, vs);
        total_bytes = out.Size();
        out.Close();
    } else if (method == "formatter") {
        OutputLocalFileStream file(path);
        TextFileFormatter formatter;
        if (type == "string") {
            for (int i = 0; i < n_elements; i++) {
                total_bytes += formatter.Write(file, "{}\n", str);
            }
        } else {
            for (int i = 0; i < n_elements; i++) {
                total_bytes += formatter.Write(file, "{}\n", x);
            }
        }
    }
    t2 = GetTime();
    double MB = (double)total_bytes / 1024 / 1024;
    LOG() << "Wrote " << MB << " MB, at " << MB / (t2 - t1) << " MB/s";

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
