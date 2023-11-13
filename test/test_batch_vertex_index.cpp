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

#include "./ut_utils.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "db/galaxy.h"
#include "./graph_factory.h"

class TestBatchVertexIndex : public TuGraphTest {
 protected:
    void SetUp() {
        TuGraphTest::SetUp();
        usr = lgraph::_detail::DEFAULT_ADMIN_NAME;
        pad = lgraph::_detail::DEFAULT_ADMIN_PASS;
        db_path = "./testdb";
        graph = "default";
        indexes_str =
            "person:uid:1,person:name:0,person:age:0,person:age16:0,person:age32:"
            "0,"
            "person:agef:0,person:aged:0";
        n_dump_key = 10;
        dump_only = false;
        verbose = 1;
    }
    void TearDown() { TuGraphTest::TearDown(); }

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

TEST_F(TestBatchVertexIndex, BatchVertexIndex) {
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
    gf.create_snapshot();

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
    std::vector<lgraph::IndexSpec> idx_specs;
    idx_specs.reserve(indx.size());
    for (auto& str : indx) {
        // parse index specifier
        auto tokens = fma_common::Split(str, ":");
        if (tokens.size() != 3) {
            UT_ERR() << "Failed to parse index specifier: " << str;
        }
        lgraph::IndexSpec spec;
        spec.label = fma_common::Strip(tokens[0], "\t ");
        spec.field = fma_common::Strip(tokens[1], "\t ");
        int type;
        size_t r =
            fma_common::TextParserUtils::ParseT(fma_common::Strip(tokens[2], "\t "), type);
        spec.type = static_cast<lgraph::IndexType>(type);
        if (spec.label.empty() || spec.field.empty() || !r) {
            UT_ERR() << "Failed to parse index specifier: " << str;
        }
        idx_specs.emplace_back(std::move(spec));
    }
    UT_LOG() << "We will build the following indexes: ";
    for (auto& spec : idx_specs) {
        UT_LOG() << "\tlabel=" << spec.label << ", field=" << spec.field
                 << ", type=" << static_cast<int>(spec.type);
    }

    lgraph::Galaxy::Config conf;
    conf.dir = db_path;
    lgraph::Galaxy galaxy(conf, true, nullptr);
    if (galaxy.GetUserToken(user, password).empty()) throw AuthError("Bad user/password.");
    lgraph::AccessControlledDB ac_db = galaxy.OpenGraph(user, graph);
    LightningGraph* db = ac_db.GetLightningGraph();

    if (!dump_only) {
        UT_LOG() << "Creating batch index";
        UT_EXPECT_ANY_THROW(db->OfflineCreateBatchIndex(idx_specs, 1 << 30));
    }
    UT_LOG() << "Dumping index result";
    auto txn = db->CreateReadTxn();
    for (auto& spec : idx_specs) {
        UT_LOG() << "\n\nDumping index " << spec.label << ":" << spec.field << "\n";
        size_t nk = 0;
        for (auto it = txn.GetVertexIndexIterator(spec.label, spec.field, "", ""); it.IsValid();
             it.Next()) {
            if (nk >= n_dump_key) break;
            nk++;
            auto k = it.GetKey();
            if (k.Size() < 480) {
                UT_LOG() << it.GetKeyData().ToString() << " -> " << it.GetVid();
            } else {
                UT_LOG() << txn.GetVertexField(it.GetVid(), spec.field).ToString() << " -> "
                         << it.GetVid();
            }
        }
    }
}
