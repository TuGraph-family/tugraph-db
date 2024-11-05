/**
* Copyright 2024 AntGroup CO., Ltd.
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

#include <gtest/gtest.h>
#include <filesystem>
#include <boost/endian/conversion.hpp>
#include "common/logger.h"
#include "test_util.h"
#include "common/value.h"
#include "graphdb/graph_db.h"
#include "transaction/transaction.h"

namespace fs = std::filesystem;
using boost::endian::big_to_native;
using boost::endian::native_to_big_inplace;
using namespace graphdb;
static std::string testdb = "testdb";
static std::unordered_map<std::string, Value> properties = {
        {"property1", Value::Bool(true)},
        {"property2", Value::Integer(100)},
        {"property3", Value::String("string")},
        {"property4", Value::Double(1.1314)},
        {"property5", Value::BoolArray({true, false})},
        {"property6", Value::IntegerArray({1, 2, 3})},
        {"property7", Value::StringArray({"string1", "string2"})},
        {"property8", Value::DoubleArray({11.11, 22.22})}
};

TEST(GraphDB, basicCreate) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    auto e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    auto e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    EXPECT_EQ(big_to_native(v1.GetId()), 1);
    EXPECT_EQ(big_to_native(v2.GetId()), 2);
    EXPECT_EQ(big_to_native(v3.GetId()), 3);
    EXPECT_EQ(big_to_native(v4.GetId()), 4);
    EXPECT_EQ(big_to_native(e1.GetId()), 1);
    EXPECT_EQ(big_to_native(e2.GetId()), 2);
    EXPECT_EQ(big_to_native(e3.GetId()), 3);
    EXPECT_EQ(big_to_native(e4.GetId()), 4);
    EXPECT_EQ(v1.GetAllProperty(), properties);
    EXPECT_EQ(v2.GetAllProperty(), properties);
    EXPECT_EQ(v3.GetAllProperty(), properties);
    EXPECT_EQ(v4.GetAllProperty(), properties);
    EXPECT_EQ(e1.GetAllProperty(), properties);
    EXPECT_EQ(e2.GetAllProperty(), properties);
    EXPECT_EQ(e3.GetAllProperty(), properties);
    EXPECT_EQ(e4.GetAllProperty(), properties);
    EXPECT_EQ(v1.GetLabels(), v1_labels);
    EXPECT_EQ(v2.GetLabels(), v2_labels);
    EXPECT_EQ(v3.GetLabels(), v3_labels);
    EXPECT_EQ(v4.GetLabels(), v4_labels);
    EXPECT_EQ(e1.GetType(), "edge_type12");
    EXPECT_EQ(e2.GetType(), "edge_type23");
    EXPECT_EQ(e3.GetType(), "edge_type34");
    EXPECT_EQ(e4.GetType(), "edge_type41");
    txn->Commit();
}

TEST(GraphDB, reOpen) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    auto e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    auto e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    txn->Commit();
    txn.reset();
    graphDB.reset();
    // reopen
    graphDB = GraphDB::Open(testdb, {});
    txn = graphDB->BeginTransaction();
    v1 = txn->GetVertexById(v1.GetId());
    v2 = txn->GetVertexById(v2.GetId());
    v3 = txn->GetVertexById(v3.GetId());
    v4 = txn->GetVertexById(v4.GetId());
    e1 = txn->GetEdgeById(e1.GetTypeId(), e1.GetId());
    e2 = txn->GetEdgeById(e2.GetTypeId(), e2.GetId());
    e3 = txn->GetEdgeById(e3.GetTypeId(), e3.GetId());
    e4 = txn->GetEdgeById(e4.GetTypeId(), e4.GetId());
    EXPECT_EQ(big_to_native(v1.GetId()), 1);
    EXPECT_EQ(big_to_native(v2.GetId()), 2);
    EXPECT_EQ(big_to_native(v3.GetId()), 3);
    EXPECT_EQ(big_to_native(v4.GetId()), 4);
    EXPECT_EQ(big_to_native(e1.GetId()), 1);
    EXPECT_EQ(big_to_native(e2.GetId()), 2);
    EXPECT_EQ(big_to_native(e3.GetId()), 3);
    EXPECT_EQ(big_to_native(e4.GetId()), 4);
    EXPECT_EQ(v1.GetAllProperty(), properties);
    EXPECT_EQ(v2.GetAllProperty(), properties);
    EXPECT_EQ(v3.GetAllProperty(), properties);
    EXPECT_EQ(v4.GetAllProperty(), properties);
    EXPECT_EQ(e1.GetAllProperty(), properties);
    EXPECT_EQ(e2.GetAllProperty(), properties);
    EXPECT_EQ(e3.GetAllProperty(), properties);
    EXPECT_EQ(e4.GetAllProperty(), properties);
    EXPECT_EQ(v1.GetLabels(), v1_labels);
    EXPECT_EQ(v2.GetLabels(), v2_labels);
    EXPECT_EQ(v3.GetLabels(), v3_labels);
    EXPECT_EQ(v4.GetLabels(), v4_labels);
    EXPECT_EQ(e1.GetType(), "edge_type12");
    EXPECT_EQ(e2.GetType(), "edge_type23");
    EXPECT_EQ(e3.GetType(), "edge_type34");
    EXPECT_EQ(e4.GetType(), "edge_type41");
    txn->Commit();
}

TEST(GraphDB, updateProperty) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->CreateEdge(v3, v4, "edge_type34", properties);
    txn->CreateEdge(v4, v1, "edge_type41", properties);
    v1.SetProperties({{"property1", Value::Bool(false)}});
    EXPECT_EQ(v1.GetProperty("property1"), Value::Bool(false));
    v1.SetProperties({{"property1", Value::String("str1")}});
    EXPECT_EQ(v1.GetProperty("property1"), Value::String("str1"));
    v1.RemoveProperty("property1");
    EXPECT_EQ(v1.GetAllProperty().size(), 7);
    v1.SetProperties({{"property9", Value::IntegerArray({10, 20, 30})}});
    EXPECT_EQ(v1.GetProperty("property9"), Value::IntegerArray({10, 20, 30}));
    EXPECT_EQ(v1.GetAllProperty().size(), 8);

    e1.SetProperties({{"property1", Value::Bool(false)}});
    EXPECT_EQ(e1.GetProperty("property1"), Value::Bool(false));
    e1.SetProperties({{"property1", Value::String("str1")}});
    EXPECT_EQ(e1.GetProperty("property1"), Value::String("str1"));
    e1.RemoveProperty("property1");
    EXPECT_EQ(e1.GetAllProperty().size(), 7);
    e1.SetProperties({{"property9", Value::IntegerArray({10, 20, 30})}});
    EXPECT_EQ(e1.GetProperty("property9"), Value::IntegerArray({10, 20, 30}));
    EXPECT_EQ(e1.GetAllProperty().size(), 8);
}

TEST(GraphDB, vertexIterator) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    txn->CreateEdge(v1, v2, "edge_type12", properties);
    txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->CreateEdge(v3, v4, "edge_type34", properties);
    txn->CreateEdge(v4, v1, "edge_type41", properties);
    int count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        count++;
        EXPECT_EQ(viter->GetVertex().GetAllProperty(), properties);
        if (count == 1) {
            EXPECT_EQ(viter->GetVertex().GetLabels(), v1_labels);
        } else if (count == 2) {
            EXPECT_EQ(viter->GetVertex().GetLabels(), v2_labels);
        } else if (count == 3) {
            EXPECT_EQ(viter->GetVertex().GetLabels(), v3_labels);
        } else if (count == 4) {
            EXPECT_EQ(viter->GetVertex().GetLabels(), v4_labels);
        }
    }
    EXPECT_EQ(count, 4);
    for (int i = 1; i <= 8; i++) {
        std::string label = "label" + std::to_string(i);
        count = 0;
        for (auto viter = txn->NewVertexIterator(label); viter->Valid(); viter->Next()) {
            count++;
        }
        EXPECT_EQ(count, 1);
    }
    for (int i = 1; i <= 8; i++) {
        std::string label = "label" + std::to_string(i);
        count = 0;
        for (auto viter = txn->NewVertexIterator(
                label, std::unordered_map<std::string, Value>{{"property3", Value::String("string")}}); viter->Valid(); viter->Next()) {
            count++;
        }
        EXPECT_EQ(count, 1);
    }
    for (int i = 1; i <= 8; i++) {
        std::string label = "label" + std::to_string(i);
        count = 0;
        for (auto viter = txn->NewVertexIterator(
                label, std::unordered_map<std::string, Value>{{"property3", Value::String("string")}}); viter->Valid(); viter->Next()) {
            count++;
        }
        EXPECT_EQ(count, 1);
    }
    for (int i = 1; i <= 8; i++) {
        std::string label = "label" + std::to_string(i);
        count = 0;
        for (auto viter = txn->NewVertexIterator(
                label, std::unordered_map<std::string, Value>{{"property3", Value::String("wrong_string")}}); viter->Valid(); viter->Next()) {
            count++;
        }
        EXPECT_EQ(count, 0);
    }
    for (int i = 1; i <= 8; i++) {
        std::string label = "label" + std::to_string(i);
        count = 0;
        for (auto viter = txn->NewVertexIterator(
                label, std::unordered_map<std::string, Value>{{"wrong_property", Value::String("string")}}); viter->Valid(); viter->Next()) {
            count++;
        }
        EXPECT_EQ(count, 0);
    }

    count = 0;
    for (auto viter = txn->NewVertexIterator("wrong_label"); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);

    txn->Commit();
}

TEST(GraphDB, edgeIterator) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v0 = txn->CreateVertex({},{});
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    auto e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    auto e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    int count = 0;

    for(auto eiter = v0.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for(auto eiter = v0.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);

    for(auto eiter = v1.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e1);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type12");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v1.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e4);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type41");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v2.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e2);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type23");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v2.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e1);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type12");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v3.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e3);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type34");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v3.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e2);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type23");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v4.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e4);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type41");
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v4.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        EXPECT_EQ(eiter->GetEdge(), e3);
        EXPECT_EQ(eiter->GetEdge().GetType(), "edge_type34");
    }
    EXPECT_EQ(count, 1);

    count = 0;
    for(auto eiter = v0.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for(auto eiter = v1.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for(auto eiter = v2.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for(auto eiter = v3.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for(auto eiter = v4.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for(auto eiter = v1.NewEdgeIterator(
            EdgeDirection::OUTGOING, {}, {{"property2", Value::Integer(100)}}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for(auto eiter = v1.NewEdgeIterator(
            EdgeDirection::BOTH, {}, {{"property2", Value::Integer(100)}}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for(auto eiter = v1.NewEdgeIterator(
            EdgeDirection::BOTH, {}, {{"wrong_property", Value::Integer(100)}}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for(auto eiter = v1.NewEdgeIterator(
            EdgeDirection::BOTH, {}, {{"property2", Value::Integer(1000)}}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);

    txn->Commit();
}

TEST(GraphDB, deleteVertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);

    v1.Delete();
    EXPECT_THROW_CODE(txn->GetVertexById(v1.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()), EdgeIdNotFound);
    int count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 3);
    count = 0;
    for (auto eiter = v2.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for (auto eiter = v2.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto eiter = v3.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for (auto eiter = v4.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for (auto eiter = v4.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    txn->Commit();
}

TEST(GraphDB, deleteEdge) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->CreateEdge(v3, v4, "edge_type34", properties);
    txn->CreateEdge(v4, v1, "edge_type41", properties);

    e1.Delete();
    EXPECT_THROW_CODE(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()), EdgeIdNotFound);
    int count = 0;
    for (auto viter = v1.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for (auto viter = v2.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for (auto viter = v2.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto viter = v3.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for (auto viter = v4.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    txn->Commit();
}

TEST(GraphDB, deleteAllVertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    auto e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    auto e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    int count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        viter->GetVertex().Delete();
        count++;
    }
    EXPECT_EQ(count, 4);
    EXPECT_THROW_CODE(txn->GetVertexById(v1.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetVertexById(v2.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetVertexById(v3.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetVertexById(v4.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e2.GetTypeId(), e2.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e3.GetTypeId(), e3.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()), EdgeIdNotFound);
    count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    txn->Commit();
}

TEST(GraphDB, scanAndUpdate) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    auto e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    auto e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        viter->GetVertex().SetProperties({{"property9", Value::Integer(100)}});
        EXPECT_EQ(viter->GetVertex().GetProperty("property9"), Value::Integer(100));
        EXPECT_EQ(viter->GetVertex().GetAllProperty().size(), 9);
    }
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertex().GetProperty("property9"), Value::Integer(100));
        EXPECT_EQ(viter->GetVertex().GetAllProperty().size(), 9);
    }
    int count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        for (auto eiter = viter->GetVertex().NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
            eiter->GetEdge().Delete();
            count++;
        }
    }
    EXPECT_EQ(count, 4);
    EXPECT_THROW_CODE(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e2.GetTypeId(), e2.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e3.GetTypeId(), e3.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()), EdgeIdNotFound);
    txn->Commit();
}

TEST(GraphDB, addDeleteLabel) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    v1.AddLabel("label1");
    v1.AddLabel("labelv1");
    {
        std::unordered_set<std::string> labels = {"label1", "label2", "labelv1"};
        EXPECT_EQ(v1.GetLabels(), labels);
    }
    int count = 0;
    for (auto viter = txn->NewVertexIterator("labelv1"); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertex(), v1);
        std::unordered_set<std::string> labels = {"label1", "label2", "labelv1"};
        EXPECT_EQ(viter->GetVertex().GetLabels(), labels);
        count++;
    }
    EXPECT_EQ(count, 1);
    v1.DeleteLabel("labelv1");
    {
        std::unordered_set<std::string> labels = {"label1", "label2"};
        EXPECT_EQ(v1.GetLabels(), labels);
    }
    v1.DeleteLabel("label_no_exist");
    {
        std::unordered_set<std::string> labels = {"label1", "label2"};
        EXPECT_EQ(v1.GetLabels(), labels);
    }
    count = 0;
    for (auto viter = txn->NewVertexIterator("labelv1"); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    txn->Commit();
}

TEST(GraphDB, expandEdge) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    txn->CreateEdge(v1, v2, "edge_type12", properties);
    txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->CreateEdge(v3, v4, "edge_type34", properties);
    txn->CreateEdge(v4, v1, "edge_type41", properties);
    bool found = false;
    for (auto viter = txn->NewVertexIterator("label1"); viter->Valid(); viter->Next()) {
        found = true;
        Vertex v = v1;
        int count = 0;
        for (auto eiter = viter->GetVertex().NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
            v = eiter->GetEdge().GetEnd();
            EXPECT_EQ(v, v2);
            count++;
            break;
        }
        EXPECT_EQ(count, 1);
        count = 0;
        for (auto eiter = v.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
            v = eiter->GetEdge().GetEnd();
            EXPECT_EQ(v, v3);
            count++;
            break;
        }
        EXPECT_EQ(count, 1);
        count = 0;
        for (auto eiter = v.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
            v = eiter->GetEdge().GetEnd();
            EXPECT_EQ(v, v4);
            count++;
            break;
        }
        EXPECT_EQ(count, 1);
        count = 0;
        for (auto eiter = v.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
            v = eiter->GetEdge().GetEnd();
            EXPECT_EQ(v, v1);
            count++;
            break;
        }
        EXPECT_EQ(count, 1);
        break;
    }
    EXPECT_EQ(found, true);
    txn->Commit();
}

TEST(GraphDB, graphTypes) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    txn->CreateEdge(v1, v2, "edge_type12", properties);
    txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->CreateEdge(v3, v4, "edge_type34", properties);
    txn->CreateEdge(v4, v1, "edge_type41", properties);
    std::unordered_set<std::string> vertex_labels = {"label1","label2","label3","label4","label5","label6","label7","label8"};
    std::unordered_set<std::string> edge_types = {"edge_type12","edge_type23", "edge_type34", "edge_type41"};
    std::unordered_set<std::string> properties_ = {"property1","property2","property3","property4","property5","property6","property7","property8"};
    EXPECT_EQ(txn->db()->id_generator().GetVertexLabels(), vertex_labels);
    EXPECT_EQ(txn->db()->id_generator().GetEdgeTypes(), edge_types);
    EXPECT_EQ(txn->db()->id_generator().GetProperties(), properties_);
    txn->Commit();
}

TEST(GraphDB, pointToSelf) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    auto v1 = txn->CreateVertex(v1_labels, properties);
    auto e1 = txn->CreateEdge(v1, v1, "edge_type1", properties);
    int count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge(), e1);
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge(), e1);
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge(), e1);
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 2);

    auto e2 = txn->CreateEdge(v1, v1, "edge_type1", properties);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 2);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 4);

    e2.Delete();
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::INCOMING, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto eiter = v1.NewEdgeIterator(EdgeDirection::BOTH, {}, {}); eiter->Valid(); eiter->Next()) {
        EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
        count++;
    }
    EXPECT_EQ(count, 2);

    v1.Delete();

    EXPECT_THROW_CODE(txn->GetVertexById(v1.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e2.GetTypeId(), e2.GetId()), EdgeIdNotFound);

    txn->Commit();
}
