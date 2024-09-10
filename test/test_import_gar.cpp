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

#include <filesystem>
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"

#include "import/import_v3.h"
#include "import/graphar_config.h"

#include "gtest/gtest.h"
#include "./test_tools.h"

using namespace fma_common;
using namespace lgraph;
using namespace import_v3;

class TestImportGar : public TuGraphTest {};

// The path "/test/resource/data/gar_test/edge_test" is for TestEdgeLabel
// The path "/test/resource/data/gar_test/ldbc_parquet" is for TestGarData

TEST_F(TestImportGar, TestEdgeLabel) {
    // reject the same edge label with different properties
    Importer::Config config;
    std::string tugraph_path = std::filesystem::current_path().parent_path().parent_path();
    UT_LOG() << tugraph_path;
    config.config_file = tugraph_path + "/test/resource/data/gar_test/edge_test/movie.graph.yml";
    config.is_graphar = true;
    config.delete_if_exists = true;

    nlohmann::json conf;
    UT_EXPECT_ANY_THROW(ParserGraphArConf(conf, config.config_file));
}

TEST_F(TestImportGar, TestGarData) {
    UT_LOG() << "Read gar data";
    Importer::Config config;
    std::string tugraph_path = std::filesystem::current_path().parent_path().parent_path();
    UT_LOG() << tugraph_path;
    config.config_file =
        tugraph_path + "/test/resource/data/gar_test/ldbc_parquet/ldbc_sample.graph.yml";
    config.is_graphar = true;
    config.delete_if_exists = true;

    nlohmann::json conf;
    ParserGraphArConf(conf, config.config_file);

    // test the first vertex data in gar config
    std::vector<import_v2::CsvDesc> data_files = import_v2::ImportConfParser::ParseFiles(conf);
    import_v2::GraphArParser parser = import_v2::GraphArParser(data_files.front());
    std::vector<std::vector<FieldData>> block;
    UT_EXPECT_NO_THROW(parser.ReadBlock(block));
    UT_EXPECT_EQ(block.size(), 903);
    UT_EXPECT_EQ(block[0][0].ToString(), "933");
    UT_EXPECT_EQ(block[0][1].ToString(), "Mahinda");
}
