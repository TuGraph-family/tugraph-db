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

#include "fma-common/logging.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "./ut_utils.h"

class TestMoveConstructors : public TuGraphTest {};

template <typename EIT>
void CheckEdgeMove(EIT& eit_) {
    using namespace lgraph;
    UT_EXPECT_TRUE(eit_.IsValid());
    VertexId src = eit_.GetSrc();
    VertexId dst = eit_.GetDst();
    EdgeId eid = eit_.GetEdgeId();
    Value prop = eit_.GetProperty();
    EIT eit = std::move(eit_);
    UT_EXPECT_TRUE(!eit_.IsValid());
    UT_EXPECT_EQ(eit.GetSrc(), src);
    UT_EXPECT_EQ(eit.GetDst(), dst);
    UT_EXPECT_EQ(eit.GetEdgeId(), eid);
    Value p = eit.GetProperty();
    UT_EXPECT_EQ(prop.Data(), p.Data());
    UT_EXPECT_EQ(prop.Size(), p.Size());
}

TEST_F(TestMoveConstructors, MoveConstructor) {
    using namespace lgraph;
    try {
        DBConfig config;
        config.dir = "./testdb";
        LightningGraph db(config);
        db.DropAllData();
        std::vector<FieldSpec> vfd = {{"name", FieldType::STRING, false},
                                      {"id", FieldType::INT64, false}};
        std::vector<FieldSpec> efd = {{"type", FieldType::INT8, false}};
        db.AddLabel("v", vfd, true, VertexOptions("id"));
        db.AddLabel("e", efd, false, EdgeOptions());

        {
            auto txn_ = db.CreateWriteTxn(false);
            auto txn = std::move(txn_);
            UT_EXPECT_TRUE(txn.IsValid());
            std::vector<std::string> vfields = {"name", "id"};
            VertexId vid0 =
                txn.AddVertex(std::string("v"), vfields, std::vector<std::string>({"a", "1"}));
            VertexId vid1_ =
                txn.AddVertex(std::string("v"), vfields, std::vector<std::string>({"b", "2"}));
            VertexId vid2_ =
                txn.AddVertex(std::string("v"), vfields, std::vector<std::string>({"c", "3"}));
            size_t ni, no;
            UT_EXPECT_TRUE(txn.DeleteVertex(vid0, &ni, &no));
            UT_EXPECT_EQ(ni, 0);
            UT_EXPECT_EQ(no, 0);
            {
                auto vit_ = txn.GetVertexIterator(vid0);
                UT_EXPECT_TRUE(!vit_.IsValid());
                auto vit = std::move(vit_);
                UT_EXPECT_TRUE(!vit.IsValid());
            }
            {
                auto vit_ = txn.GetVertexIterator(vid1_);
                UT_EXPECT_TRUE(vit_.IsValid());
                Value v = vit_.GetProperty();
                auto vit = std::move(vit_);
                UT_EXPECT_TRUE(vit.IsValid());
                Value v2 = vit.GetProperty();
                UT_EXPECT_EQ(v.Data(), v2.Data());
            }
            txn.AddEdge(vid1_, vid2_, std::string("e"), std::vector<std::string>({"type"}),
                        std::vector<std::string>({"1"}));
            txn.AddEdge(vid2_, vid1_, std::string("e"), std::vector<std::string>({"type"}),
                        std::vector<std::string>({"2"}));
            txn.AddEdge(vid1_, vid2_, std::string("e"), std::vector<std::string>({"type"}),
                        std::vector<std::string>({"3"}));
            {
                auto eit_ = txn.GetOutEdgeIterator(EdgeUid(vid1_, vid2_, 0, 0, 0), true);
                CheckEdgeMove(eit_);
            }
            {
                auto vit = txn.GetVertexIterator(vid1_);
                auto eit_ = vit.GetInEdgeIterator();
                CheckEdgeMove(eit_);
            }
        }
    } catch (std::exception& e) {
        UT_EXPECT_TRUE(false);
        UT_LOG() << "An error occurred: " << e.what();
    }
}
