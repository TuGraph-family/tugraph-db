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

#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "core/blob_manager.h"

#include "./test_tools.h"

class TestBlobManager : public TuGraphTest {};

TEST_F(TestBlobManager, BlobManager) {
    using namespace lgraph;
    std::string dir = "./testdb";
    std::string tbl_name = "blob";

    AutoCleanDir cleaner(dir);
    {
        auto store = std::make_unique<LMDBKvStore>(dir);
        auto txn = store->CreateWriteTxn();
        auto tbl = BlobManager::OpenTable(*txn, *store, tbl_name);
        std::vector<std::string> strs;
        for (size_t i = 0; i < 4; i++) strs.emplace_back(1024 << i, (char)('0' + i));
        BlobManager bm(*txn, std::move(tbl));
        BlobManager::BlobKey k1 = bm.Add(*txn, Value::ConstRef(strs[0]));
        BlobManager::BlobKey k2 = bm.Add(*txn, Value::ConstRef(strs[1]));
        BlobManager::BlobKey k3 = bm.Add(*txn, Value::ConstRef(strs[3]));

        UT_EXPECT_EQ(bm.Get(*txn, k1).AsString(), strs[0]);
        UT_EXPECT_EQ(bm.Get(*txn, k3).AsString(), strs[3]);
        bm.Replace(*txn, k2, Value::ConstRef(strs[2]));
        UT_EXPECT_EQ(bm.Get(*txn, k2).AsString(), strs[2]);
        bm.Delete(*txn, k1);
        bm.Delete(*txn, k2);
        auto it = bm.table_->GetIterator(*txn);
        it->GotoFirstKey();
        UT_EXPECT_TRUE(it->IsValid());
        UT_EXPECT_EQ(it->GetKey().AsType<BlobManager::BlobKey>(), k3);
        UT_EXPECT_EQ(it->GetValue().AsString(), strs[3]);
        it->Close();
        txn->Commit();
    }
    fma_common::file_system::RemoveDir(dir);
}
