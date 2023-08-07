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

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"
#include "lgraph/lgraph.h"

#include "./test_tools.h"
#include "./ut_utils.h"
using namespace lgraph_api;

class TestMaxDbSize : public TuGraphTest {};

TEST_F(TestMaxDbSize, LargerThanMaxSize) {
    const std::string dir = "./testdb";
    lgraph::AutoCleanDir _(dir);
    // open a graph with max_db_size = 1MB
    using namespace lgraph_api;
    Galaxy galaxy(dir, false, true);
    galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
        lgraph::_detail::DEFAULT_ADMIN_PASS);
    UT_ASSERT(galaxy.CreateGraph("test", "test", 1<<20));
    auto graph = galaxy.OpenGraph("test");
    FieldSpec f1("id", FieldType::INT64, false);
    FieldSpec f2("desc", FieldType::STRING, false);
    graph.AddVertexLabel("v", {f1, f2}, VertexOptions(f1.name));
    auto txn = graph.CreateWriteTxn();
    try {
        // insert 1MB data
        for (size_t i = 0; i < (1 << 20); i++) {
            std::string desc = std::string(100, 'a');
            txn.AddVertex("v", {f1.name, f2.name}, {FieldData::Int64(i), FieldData(desc)});
        }
        txn.Commit();
        UT_ERR() << "Expected full-db exception";
        UT_ASSERT(false);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
