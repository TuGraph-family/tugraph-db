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
#include "fma-common/string_util.h"
#include "gtest/gtest.h"

#include "core/kv_store.h"

#include "lgraph/lgraph.h"
#include "./ut_utils.h"

class TestPerfMultiWriter : public TuGraphTest {};

using namespace lgraph;
using namespace fma_common;

static const size_t n = 1000;

void TestKvMultiWriter() {
    auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << 30, true);

    {
        auto txn = store->CreateWriteTxn();
        auto tables = store->ListAllTables(*txn);
        if (!tables.empty()) store->DropAll(*txn);
        txn->Commit();

        txn = store->CreateWriteTxn();
        auto table = store->OpenTable(*txn, "mw", true, ComparatorDesc::DefaultComparator());
        txn->Commit();

        auto t0 = fma_common::GetTime();
#pragma omp parallel for
        for (size_t i = 0; i < n; i++) {
            auto txn = store->CreateWriteTxn();
            ASSERT(table->AddKV(*txn, Value::ConstRef<int>(i), Value::ConstRef<int>(0)));
            txn->Commit();
        }
        auto t1 = fma_common::GetTime();
        UT_LOG() << "writing " << n << " keys with exclusive write transactions took " << (t1 - t0)
                 << " seconds";
    }

    {
        auto txn = store->CreateWriteTxn();
        auto tables = store->ListAllTables(*txn);
        if (!tables.empty()) store->DropAll(*txn);
        txn->Commit();

        txn = store->CreateWriteTxn();
        auto table = store->OpenTable(*txn, "mw", true, ComparatorDesc::DefaultComparator());
        txn->Commit();

        auto t0 = fma_common::GetTime();
#pragma omp parallel for
        for (size_t i = 0; i < n; i++) {
            auto txn = store->CreateWriteTxn(true);
            ASSERT(table->AddKV(*txn, Value::ConstRef<int>(i), Value::ConstRef<int>(0)));
            txn->Commit();
        }
        auto t1 = fma_common::GetTime();
        UT_LOG() << "writing " << n << " keys with optimistic write transactions took " << (t1 - t0)
                 << " seconds";
    }
}

void TestLGraphApiMultiWriter() {
    {
        auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 30, true);
        auto txn = store->CreateWriteTxn();
        auto tables = store->ListAllTables(*txn);
        if (!tables.empty()) store->DropAll(*txn);
        txn->Commit();
    }

    lgraph_api::Galaxy galaxy("./testdb",
                              lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS,
                              false, true);
    lgraph_api::GraphDB db = galaxy.OpenGraph("default");
    db.AddVertexLabel("vertex",
                      std::vector<FieldSpec>(
                          {{"id", FieldType::INT32, false}, {"name", FieldType::STRING, false}}),
                      VertexOptions("id"));
    db.AddVertexIndex("vertex", "name", lgraph::IndexType::NonuniqueIndex);

    {
        auto t0 = fma_common::GetTime();
#pragma omp parallel for
        for (int i = 0; i < 1000; i++) {
            auto txn = db.CreateWriteTxn(false);
            txn.AddVertex("vertex", std::vector<std::string>({"id", "name"}),
                          std::vector<std::string>({std::to_string(i), std::to_string(i)}));
            txn.Commit();
        }
        auto t1 = fma_common::GetTime();
        UT_LOG() << "Adding " << n << " vertices with exclusive write transactions took "
                 << (t1 - t0) << " seconds";
    }

    {
        auto t0 = fma_common::GetTime();
#pragma omp parallel for
        for (int i = 0; i < 1000; i++) {
            auto txn = db.CreateWriteTxn(false);
            auto vit = txn.GetVertexByUniqueIndex("vertex", "id", std::to_string(i));
            vit.Delete();
            txn.Commit();
        }
        auto t1 = fma_common::GetTime();
        UT_LOG() << "Deleting " << n << " vertices with exclusive write transactions took "
                 << (t1 - t0) << " seconds";
    }

    {
        auto t0 = fma_common::GetTime();
        // There may be conflicts, so we add vertices sequentially rather than concurrently
        // #pragma omp parallel for
        for (int i = 0; i < 1000; i++) {
            auto txn = db.CreateWriteTxn(true);
            txn.AddVertex("vertex", std::vector<std::string>({"id", "name"}),
                          std::vector<std::string>({std::to_string(i), std::to_string(i)}));
            txn.Commit();
        }
        auto t1 = fma_common::GetTime();
        UT_LOG() << "Adding " << n << " vertices with optimistic write transactions took "
                 << (t1 - t0) << " seconds";
    }

    {
        auto t0 = fma_common::GetTime();
#pragma omp parallel for
        for (int i = 0; i < 1000; i++) {
            auto txn = db.CreateWriteTxn(true);
            auto vit = txn.GetVertexByUniqueIndex("vertex", "id", std::to_string(i));
            vit.Delete();
            txn.Commit();
        }
        auto t1 = fma_common::GetTime();
        UT_LOG() << "Deleting " << n << " vertices with optimistic write transactions took "
                 << (t1 - t0) << " seconds";
    }
}

TEST_F(TestPerfMultiWriter, PerfMultiWriter) {
    TestKvMultiWriter();
    TestLGraphApiMultiWriter();
}
