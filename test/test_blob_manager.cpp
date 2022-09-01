/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
        KvStore store(dir);
        auto txn = store.CreateWriteTxn();
        auto tbl = BlobManager::OpenTable(txn, store, tbl_name);
        std::vector<std::string> strs;
        for (size_t i = 0; i < 4; i++) strs.emplace_back(1024 << i, (char)('0' + i));
        BlobManager bm(txn, tbl);
        BlobManager::BlobKey k1 = bm.Add(txn, Value::ConstRef(strs[0]));
        BlobManager::BlobKey k2 = bm.Add(txn, Value::ConstRef(strs[1]));
        BlobManager::BlobKey k3 = bm.Add(txn, Value::ConstRef(strs[3]));

        UT_EXPECT_EQ(bm.Get(txn, k1).AsString(), strs[0]);
        UT_EXPECT_EQ(bm.Get(txn, k3).AsString(), strs[3]);
        bm.Replace(txn, k2, Value::ConstRef(strs[2]));
        UT_EXPECT_EQ(bm.Get(txn, k2).AsString(), strs[2]);
        bm.Delete(txn, k1);
        bm.Delete(txn, k2);
        auto it = tbl.GetIterator(txn);
        UT_EXPECT_TRUE(it.IsValid());
        UT_EXPECT_EQ(it.GetKey().AsType<BlobManager::BlobKey>(), k3);
        UT_EXPECT_EQ(it.GetValue().AsString(), strs[3]);
        it.Close();
        txn.Commit();
    }
}
