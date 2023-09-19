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
#include "fma-common/logger.h"
#include "fma-common/string_formatter.h"

#include "import/import_v3.h"
#include "import/graphar_config.h"

#include "gtest/gtest.h"
#include "./test_tools.h"

using namespace fma_common;
using namespace lgraph;
using namespace import_v3;

class TestImportGar : public TuGraphTest {};

TEST_F(TestImportGar, TestGarConfig) {
    // test parse gar config
    UT_LOG() << "Parsing gar yaml config to lgraph_import json config";
    Importer::Config config;
    std::string tugraph_path = std::filesystem::path(__FILE__).parent_path().parent_path();
    config.config_file = tugraph_path + "/test/resource/data/ldbc/ldbc.graph.yml";
    config.is_graphar = true;
    config.delete_if_exists = true;

    nlohmann::json conf;
    UT_EXPECT_NO_THROW(ParserGraphArConf(conf, config.config_file));
}