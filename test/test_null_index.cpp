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
#include "lgraph/lgraph.h"
#include "core/defs.h"
#include "core/data_type.h"
using namespace lgraph_api;

class TestNullIndex : public TuGraphTest {};

TEST_F(TestNullIndex, deta) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    Galaxy galaxy(path);
    galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
    GraphDB db = galaxy.OpenGraph("default");
    UT_EXPECT_TRUE(db.AddVertexLabel("Person",
                                     std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                             {"null_index", FieldType::STRING, true},
                                                             {"null_unique", FieldType::STRING, true}}),
                                     VertexOptions("id")));
    std::vector<std::string> vp({"id", "null_index", "null_unique"});
    auto txn = db.CreateWriteTxn();
    txn.AddVertex(std::string("Person"), vp,
                              {FieldData((int32_t)1), FieldData(), FieldData()});
    txn.AddVertex(std::string("Person"), vp,
                              {FieldData((int32_t)2), FieldData("val2"), FieldData("val2")});
    txn.AddVertex(std::string("Person"), vp,
                              {FieldData((int32_t)3), FieldData(), FieldData()});
    txn.AddVertex(std::string("Person"), vp,
                              {FieldData((int32_t)4), FieldData("val4"), FieldData("val4")});
    txn.Commit();
    UT_EXPECT_TRUE(db.AddVertexIndex("Person", "null_index", lgraph::IndexType::NonuniqueIndex));
    UT_EXPECT_TRUE(db.AddVertexIndex("Person", "null_unique", lgraph::IndexType::GlobalUniqueIndex));
    txn = db.CreateReadTxn();
    {
        std::vector<FieldData> fds;
        for (auto iter =
                 txn.GetVertexIndexIterator("Person", "null_index", FieldData(), FieldData("z"));
             iter.IsValid(); iter.Next()) {
            fds.push_back(iter.GetIndexValue());
        }
	std::vector<FieldData> expected {FieldData("val2"),FieldData("val4")};
        UT_EXPECT_EQ(fds, expected);
    }
    {
        std::vector<FieldData> fds;
        for (auto iter =
                 txn.GetVertexIndexIterator("Person", "null_unique", FieldData(), FieldData("z"));
             iter.IsValid(); iter.Next()) {
            fds.push_back(iter.GetIndexValue());
        }
        std::vector<FieldData> expected {FieldData("val2"),FieldData("val4")};
        UT_EXPECT_EQ(fds, expected);
    }

    UT_EXPECT_TRUE(db.AddEdgeLabel("Relation",
                                   std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                           {"null_index", FieldType::STRING, true},
                                                           {"null_unique", FieldType::STRING, true}}),
                                   EdgeOptions()));
    
}
