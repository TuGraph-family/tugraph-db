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
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "core/lightning_graph.h"
#include "db/galaxy.h"
#include "./graph_factory.h"
#include "core/data_type.h"
class TestBatchEdgeIndex : public TuGraphTest {
 protected:
    void SetUp() {
        TuGraphTest::SetUp();
        usr = "admin";
        pad = "73@TuGraph";
        db_path = "./testdb";
        graph = "default";
        indexes_str = "Person:birthyear:0:true,ACTED_IN:charactername:0:false";
        n_dump_key = 10;
        dump_only = false;
        verbose = 1;
    }
    void TearDown() { TuGraphTest::TearDown(); }

    bool HasVertexIndex(lgraph::AccessControlledDB& db, const std::string& label,
                        const std::string& field, lgraph::IndexType type) {
        auto txn = db.CreateReadTxn();
        auto indexes = txn.ListVertexIndexes();
        for (auto& i : indexes) {
            if (i.label == label && i.field == field &&
                i.type == type) {
                return true;
            }
        }
        return false;
    }
    bool HasEdgeIndex(lgraph::AccessControlledDB& db, const std::string& label,
                      const std::string& field, lgraph::IndexType type) {
        auto txn = db.CreateReadTxn();
        auto indexes = txn.ListEdgeIndexes();
        for (auto& i : indexes) {
            if (i.label == label && i.field == field &&
                i.type == type) {
                return true;
            }
        }
        return false;
    }
    size_t GetVertexIndexValueNum(lgraph::AccessControlledDB& db, const std::string& label,
                                  const std::string& field) {
        auto txn = db.CreateReadTxn();
        size_t count = 0;
        for (auto it = txn.GetVertexIndexIterator(label, field, "", ""); it.IsValid(); it.Next()) {
            ++count;
        }
        return count;
    }
    size_t GetEdgeIndexValueNum(lgraph::AccessControlledDB& db, const std::string& label,
                                const std::string& field) {
        auto txn = db.CreateReadTxn();
        size_t count = 0;
        for (auto it = txn.GetEdgeIndexIterator(label, field, "", ""); it.IsValid(); it.Next()) {
            ++count;
        }
        return count;
    }
    bool HasVertexIndexKey(lgraph::AccessControlledDB& db, const std::string& label,
                           const std::string& field, const lgraph::FieldData& key) {
        auto txn = db.CreateReadTxn();
        for (auto it =
                 txn.GetVertexIndexIterator(label, field, key, key); it.IsValid(); it.Next()) {
            if (it.GetKeyData() == key) return true;
        }
        return false;
    }
    bool HasEdgeIndexKey(lgraph::AccessControlledDB& db, const std::string& label,
                         const std::string& field, const lgraph::FieldData& key) {
        auto txn = db.CreateReadTxn();
        for (auto it = txn.GetEdgeIndexIterator(label, field, key, key); it.IsValid(); it.Next()) {
            if (it.GetKeyData() == key) return true;
        }
        return false;
    }

    std::string usr;
    std::string pad;
    std::string db_path;
    std::string graph;
    std::string indexes_str;
    size_t n_dump_key;
    bool dump_only;
    int verbose;
    std::string log;
    std::string user, password;
};

TEST_F(TestBatchEdgeIndex, BatchEdgeIndex) {
    using namespace fma_common;
    using namespace lgraph;
    user = usr;
    password = pad;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(db_path, "dir", true).Comment("DB data dir");
    config.Add(graph, "g,graph", true).Comment("Graph name");
    config.Add(user, "u,user", true).Comment("User name");
    config.Add(password, "p,password", true).Comment("Password");
    config.Add(indexes_str, "indexes", true)
        .Comment("Indexes to build, in the form of [label:field:unique],[l:f:u]");
    config.Add(n_dump_key, "n_dump", true)
        .Comment("Number of keys to dump for the specified index");
    config.Add(dump_only, "dump_only", true).Comment("Do not build index, only dump the index");
    config.Add(verbose, "verbose", true).Comment("Verbose level");
    config.Add(log, "log", true).Comment("Log location");
    config.ExitAfterHelp();
    config.ParseAndFinalize(argc, argv);
    GraphFactory gf;
    gf.create_yago(db_path);

    fma_common::LogLevel level;
    if (verbose == 0)
        level = fma_common::LL_WARNING;
    else if (verbose == 1)
        level = fma_common::LL_INFO;
    else
        level = fma_common::LL_DEBUG;
    fma_common::Logger::Get().SetLevel(level);
    if (!log.empty()) {
        fma_common::Logger::Get().SetDevice(
            std::shared_ptr<fma_common::LogDevice>(new fma_common::FileLogDevice(log)));
    }
    fma_common::Logger::Get().SetFormatter(
        std::shared_ptr<fma_common::LogFormatter>(new fma_common::TimedModuleLogFormatter()));

    if (indexes_str.empty()) {
        UT_ERR() << "Empty index.";
    }
    std::vector<std::string> indx = fma_common::Split(indexes_str, ",");
    std::vector<lgraph::IndexSpec> vertex_idxs, edge_idxs;
    vertex_idxs.reserve(indx.size());
    edge_idxs.reserve(indx.size());
    for (auto& str : indx) {
        // parse index specifier
        auto tokens = fma_common::Split(str, ":");
        if (tokens.size() != 4) {
            UT_ERR() << "Failed to parse index specifier: " << str;
        }
        lgraph::IndexSpec spec;
        spec.label = fma_common::Strip(tokens[0], "\t ");
        spec.field = fma_common::Strip(tokens[1], "\t ");
        int type;
        size_t r =
            fma_common::TextParserUtils::ParseT(fma_common::Strip(tokens[2], "\t "), type);
        spec.type = static_cast<lgraph::IndexType>(type);
        bool is_vertex = false;
        size_t v =
            fma_common::TextParserUtils::ParseT(fma_common::Strip(tokens[3], "\t "), is_vertex);
        if (spec.label.empty() || spec.field.empty() || !r || !v) {
            UT_ERR() << "Failed to parse index specifier: " << str;
        }
        if (is_vertex) {
            vertex_idxs.emplace_back(std::move(spec));
        } else {
            edge_idxs.emplace_back(std::move(spec));
        }
    }
    UT_LOG() << "We will build the following vertex indexes: ";
    for (auto& spec : vertex_idxs) {
        UT_LOG() << "\tlabel =" << spec.label << ", field =" << spec.field
                 << ", type =" << static_cast<int>(spec.type);
    }
    UT_LOG() << "We will build the following edge indexes: ";
    for (auto& spec : edge_idxs) {
        UT_LOG() << "\tlabel =" << spec.label << ", field =" << spec.field
                 << ", type =" << static_cast<int>(spec.type);
    }

    lgraph::Galaxy::Config conf;
    conf.dir = db_path;
    lgraph::Galaxy galaxy(conf, true, nullptr);
    if (galaxy.GetUserToken(user, password).empty()) throw AuthError("Bad user/password.");
    lgraph::AccessControlledDB ac_db = galaxy.OpenGraph(user, graph);
    LightningGraph* db = ac_db.GetLightningGraph();

    db->OfflineCreateBatchIndex(vertex_idxs, 1 << 30, true);
    db->OfflineCreateBatchIndex(edge_idxs, 1 << 30, false);

    UT_EXPECT_TRUE(HasVertexIndex(ac_db, "Person", "name", lgraph::IndexType::GlobalUniqueIndex));
    UT_EXPECT_TRUE(HasVertexIndex(ac_db, "Person", "birthyear", lgraph::IndexType::NonuniqueIndex));
    UT_EXPECT_TRUE(
        HasEdgeIndex(ac_db, "ACTED_IN", "charactername", lgraph::IndexType::NonuniqueIndex));

    UT_EXPECT_EQ(GetVertexIndexValueNum(ac_db, "Person", "name"), 13);
    UT_EXPECT_EQ(GetVertexIndexValueNum(ac_db, "Person", "birthyear"), 13);
    UT_EXPECT_EQ(GetEdgeIndexValueNum(ac_db, "ACTED_IN", "charactername"), 8);

    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1910)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1908)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1937)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1939)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1952)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1963)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1930)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1954)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1986)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1965)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1873)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1932)));
    UT_EXPECT_TRUE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1970)));
    UT_EXPECT_FALSE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)1800)));
    UT_EXPECT_FALSE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)2000)));
    UT_EXPECT_FALSE(
        HasVertexIndexKey(ac_db, "Person", "birthyear", lgraph::FieldData((int16_t)2100)));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("The Headmaster")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("Guenevere")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("King Arthur")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("Albus Dumbledore")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("Liz James")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("Nick Parker")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("Halle/Annie")));
    UT_EXPECT_TRUE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("Henri Ducard")));
    UT_EXPECT_FALSE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("test1")));
    UT_EXPECT_FALSE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("test2")));
    UT_EXPECT_FALSE(
        HasEdgeIndexKey(ac_db, "ACTED_IN", "charactername", lgraph::FieldData("test3")));


    fma_common::file_system::RemoveDir(db_path);
}
