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

#pragma once

#include <tuple>
#include "lgraph/olap_profile.h"
#include "lgraph/lgraph_utils.h"
#include "lgraph/olap_base.h"
#include "fma-common/configuration.h"

namespace lgraph_api {
namespace olap {

enum SourceType {
    BINARY_FILE,
    TEXT_FILE,
    ODPS
};

/**
 * @brief   Parser for unweighted edges.
 * @param               p           Beginning pointer of input edge.
 * @param               end         Ending pointer of input edge.
 * @param   [in,out]    e           Edge to store the result.
 * @return
 */

template <typename EdgeData>
std::tuple<size_t, bool> parse_line_unweighted(const char *p,
                        const char *end, EdgeUnit<EdgeData> &e) {
    const char *orig = p;
    int64_t t = 0;
    p += fma_common::TextParserUtils::ParseInt64(p, end, t);
    e.src = t;

    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;

    p += fma_common::TextParserUtils::ParseInt64(p, end, t);
    e.dst = t;

    while (p != end && *p != '\n') p++;

    return std::tuple<size_t, bool>(p - orig, p != orig);
}

/**
 * @brief   Parser for weighted edges.
 * @param               EdgeData    Type of the edge data.
 * @param               p           Beginning pointer of input edge.
 * @param               end         Ending pointer of input edge.
 * @param   [in,out]    e           Edge to store the result.
 * @return
 */
template <typename EdgeData>
std::tuple<size_t, bool> parse_line_weighted(const char* p,
                                             const char* end, EdgeUnit<EdgeData>& e) {
    const char* orig = p;
    int64_t t = 0;
    p += fma_common::TextParserUtils::ParseInt64(p, end, t);
    e.src = t;

    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += fma_common::TextParserUtils::ParseInt64(p, end, t);
    e.dst = t;
    double w = 1.0;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ','))  p++;
    p += fma_common::TextParserUtils::ParseDouble(p, end, w);
    e.edge_data = w;
    while (p != end && *p != '\n') p++;

    return std::tuple<size_t, bool>(p - orig, p != orig);
}

/**
 * @brief   Parser for unweighted edges. The data type of the vertices is string.
 * @param               p           Beginning pointer of input edge.
 * @param               end         Ending pointer of input edge.
 * @param   [in,out]    e           Edge to store the result.
 * @return
 */
template <typename EdgeData>
std::tuple<size_t, bool> parse_string_line_unweighted(const char* p,
        const char* end, EdgeStringUnit<EdgeData>& e) {
    const char* orig = p;
    std::string t = "";
    p += fma_common::TextParserUtils::ParseCsvString(p, end, t);
    e.src = t;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += fma_common::TextParserUtils::ParseCsvString(p, end, t);
    e.dst = t;

    while (p != end && *p != '\n') p++;

    return std::tuple<size_t, bool>(p - orig, p != orig);
}

/**
 * @brief   Parser for weighted edges. The data type of the vertices is string.
 * @param               EdgeData    Type of the edge data.
 * @param               p           Beginning pointer of input edge.
 * @param               end         Ending pointer of input edge.
 * @param   [in,out]    e           Edge to store the result.
 * @return
 */
template <typename EdgeData>
std::tuple<size_t, bool> parse_string_line_weighted(const char* p,
        const char* end, EdgeStringUnit<EdgeData>& e) {
    const char* orig = p;
    std::string t = "";
    double k = 1.0;
    p += fma_common::TextParserUtils::ParseCsvString(p, end, t);
    e.src = t;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += fma_common::TextParserUtils::ParseCsvString(p, end, t);
    e.dst = t;
    while (p != end && (*p == ' ' || *p == '\t' || *p == ','))  p++;
    p += fma_common::TextParserUtils::ParseDouble(p, end, k);
    e.edge_data = k;
    while (p != end && *p != '\n') p++;

    return std::tuple<size_t, bool>(p - orig, p != orig);
}

template<typename EdgeData>
class ConfigBase {
 public:
    size_t num_vertices = 0;
    SourceType  type;
    std::string name = std::string("app");

    // FILE
    std::string input_dir = "";
    std::string output_dir = "";
    bool id_mapping = false;
    std::function<std::tuple<size_t, bool>(const char *, const char *, EdgeUnit<EdgeData> &)>
            parse_line = parse_line_unweighted<EdgeData>;
    std::function<std::tuple<size_t, bool>(const char *, const char *, EdgeStringUnit<EdgeData> &)>
            parse_string_line = parse_string_line_unweighted<EdgeData>;

    ConfigBase(int &argc, char** &argv) {
        fma_common::Configuration config;
        this->AddParameterType(config);
        config.ParseAndRemove(&argc, &argv);
        config.ExitAfterHelp(false);
        config.Finalize();
        this->GetTypeFromString();
    }

    ConfigBase() {this->GetTypeFromString(); }

    virtual void AddParameter(fma_common::Configuration& config) {
        AddParameterFile(config);
    }

    virtual void Print() {
        std::cout << "parameter:" << std::endl;
        PrintFile();
    }

    SourceType GetType() {
        return type;
    }

    void GetTypeFromString() {
        if (type_str == "text") {
            type = TEXT_FILE;
        } else if (type_str == "binary") {
            type = BINARY_FILE;
        } else {
            printf("Unrecognized source type: %s\n", type_str.c_str());
            exit(-1);
        }
    }

    void AddParameterType(fma_common::Configuration& config) {
        config.Add(type_str, "type", true)
                .Comment("source type, can be text/binary");
    }

 private:
    std::string type_str = "text";
    void AddParameterFile(fma_common::Configuration& config) {
        config.Add(num_vertices, "vertices", true)
                .Comment("total number of vertices");
        config.Add(input_dir, "input_dir", false)
                .Comment("input dir or input file of graph edgelist in txt");
        config.Add(id_mapping, "id_mapping", true)
                .Comment("input file of graph edgelist in txt if need id mapping");
        config.Add(output_dir, "output_dir", true)
                .Comment("output dir of result");
    }

    void PrintFile() {
        if (num_vertices > 0) {
            std::cout << "  vertices:     " << num_vertices   << std::endl;
            std::cout << "  input_dir:    " << input_dir      << std::endl;
            std::cout << "  output_dir:   " << output_dir     << std::endl;
        }
    }
};
}  // namespace olap
}  // namespace lgraph_api
