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

#if LGRAPH_ENABLE_FULLTEXT_INDEX

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"
#include "core/lightning_graph.h"
#include "core/full_text_index.h"
#include "./ut_utils.h"

using namespace lgraph;
using namespace fma_common;

class TestFullTextIndex : public TuGraphTestWithParam<bool> {};

INSTANTIATE_TEST_CASE_P(TestFullTextIndex, TestFullTextIndex, testing::Values(false, true));

TEST_P(TestFullTextIndex, basic) {
    {
        DBConfig config;
        config.dir = "./testdb";
        config.ft_index_options.enable_fulltext_index = true;
        config.ft_index_options.fulltext_analyzer = "StandardAnalyzer";
        config.ft_index_options.fulltext_refresh_interval = 1;
        LightningGraph db(config);
        db.DropAllData();
        VertexOptions vo;
        vo.primary_field = "name";
        vo.detach_property = GetParam();
        EdgeOptions eo;
        eo.detach_property = GetParam();
        std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                        {"title", FieldType::STRING, false},
                                        {"description", FieldType::STRING, true},
                                        {"type", FieldType::INT8, false}};
        std::vector<FieldSpec> e_fds = {{"name", FieldType::STRING, false},
                                        {"comments", FieldType::STRING, true},
                                        {"weight", FieldType::FLOAT, false}};
        UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, vo));
        UT_EXPECT_TRUE(db.AddLabel("e1", e_fds, false, eo));
        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "name"));
        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "title"));
        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "description"));
        UT_EXPECT_TRUE(db.AddFullTextIndex(false, "e1", "name"));
        UT_EXPECT_TRUE(db.AddFullTextIndex(false, "e1", "comments"));

        // add vertex
        Transaction txn = db.CreateWriteTxn();
        std::vector<std::string> v1_properties = {"name", "title", "description", "type"};
        VertexId v_id1 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name1", "title title1 title2", "desc1 desc2", "1"});
        VertexId v_id2 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name2", "title title3 title4", "desc3 desc4", "1"});
        VertexId v_id3 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name3", "title title5 title6", "desc5 desc6", "1"});
        VertexId v_id4 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name4", "title title7 title8", "desc7 desc8", "1"});
        txn.Commit();
        db.FullTextIndexRefresh();
        auto vids = db.QueryVertexByFullTextIndex("v1", "title:title1", 10);
        UT_EXPECT_EQ(vids.size(), 1);
        UT_EXPECT_TRUE(vids[0].first == v_id1);
        vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_EQ(vids.size(), 4);

        // update vertex
        txn = db.CreateWriteTxn();
        txn.SetVertexProperty(
            v_id4, v1_properties,
            std::vector<std::string>{"name4", "title title7 title8", "desc77 desc88", "1"});
        txn.Commit();
        db.FullTextIndexRefresh();
        vids = db.QueryVertexByFullTextIndex("v1", "description:desc7", 10);
        UT_EXPECT_TRUE(vids.empty());
        vids = db.QueryVertexByFullTextIndex("v1", "description:desc77", 10);
        UT_EXPECT_EQ(vids.size(), 1);

        // add edge
        txn = db.CreateWriteTxn();
        std::vector<std::string> e1_properties = {"name", "comments", "weight"};
        txn.AddEdge(v_id1, v_id2, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name1", "comments comments1 comments2", "10"});
        txn.AddEdge(v_id1, v_id2, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name2", "comments comments3 comments4", "11"});
        EdgeUid e_id3 =
            txn.AddEdge(v_id2, v_id1, std::string("e1"), e1_properties,
                        std::vector<std::string>{"name3", "comments comments5 comments6", "12"});
        txn.AddEdge(v_id2, v_id1, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name4", "comments comments7 comments8", "13"});
        EdgeUid e_id5 =
            txn.AddEdge(v_id3, v_id4, std::string("e1"), e1_properties,
                        std::vector<std::string>{"name5", "comments comments7 comments8", "14"});
        EdgeUid e_id6 =
            txn.AddEdge(v_id4, v_id3, std::string("e1"), e1_properties,
                        std::vector<std::string>{"name6", "comments comments9 comments10", "15"});
        txn.Commit();
        db.FullTextIndexRefresh();
        auto eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments5", 10);
        UT_EXPECT_EQ(eids.size(), 1);
        UT_EXPECT_TRUE(eids[0].first == e_id3);
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments", 10);
        UT_EXPECT_EQ(eids.size(), 6);

        // update edge
        txn = db.CreateWriteTxn();
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments100", 10);
        UT_EXPECT_TRUE(eids.empty());
        txn.SetEdgeProperty(
            e_id6, e1_properties,
            std::vector<std::string>{"name6", "comments comments9 comments10 comments100", "15"});
        txn.Commit();
        db.FullTextIndexRefresh();
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments100", 10);
        UT_EXPECT_EQ(eids.size(), 1);

        // delete vertex
        txn = db.CreateWriteTxn();
        size_t n_in, n_out;
        txn.DeleteVertex(v_id1, &n_in, &n_out);
        UT_EXPECT_EQ(n_in, 2);
        UT_EXPECT_EQ(n_out, 2);
        txn.Commit();
        db.FullTextIndexRefresh();
        vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_EQ(vids.size(), 3);
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments", 10);
        UT_EXPECT_EQ(eids.size(), 2);
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments7", 10);
        UT_EXPECT_EQ(eids.size(), 1);
        UT_EXPECT_TRUE(eids[0].first == e_id5);
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments9", 10);
        UT_EXPECT_EQ(eids.size(), 1);
        UT_EXPECT_TRUE(eids[0].first == e_id6);

        // delete edge
        txn = db.CreateWriteTxn();
        UT_EXPECT_TRUE(txn.DeleteEdge(e_id6));
        txn.Commit();
        db.FullTextIndexRefresh();
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments", 10);
        UT_EXPECT_EQ(eids.size(), 1);
        UT_EXPECT_TRUE(eids[0].first == e_id5);
        vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_EQ(vids.size(), 3);

        // test exception
        vids.clear();
        UT_EXPECT_THROW_MSG(vids = db.QueryVertexByFullTextIndex("v1", "", 10), "ParseException");
        UT_EXPECT_TRUE(vids.empty());

        std::mutex mutex;
        // muti-thread jni write
        std::vector<std::thread> threads;
        auto writer = [&](int num) {
            std::lock_guard<std::mutex> guard(mutex);
            Transaction txn = db.CreateWriteTxn();
            std::vector<std::string> v1_properties = {"name", "title", "description", "type"};
            txn.AddVertex(std::string("v1"), v1_properties,
                          std::vector<std::string>{"thread_name" + std::to_string(num),
                                                   "thread thread1 thread2", "desc1 desc2", "1"});
            txn.Commit();
        };
        for (int i = 0; i < 10; i++) {
            threads.emplace_back(writer, 100 + i);
        }
        for (auto& t : threads) {
            t.join();
        }
        db.FullTextIndexRefresh();
        vids = db.QueryVertexByFullTextIndex("v1", "title:thread", 20);
        UT_EXPECT_EQ(vids.size(), 10);

        // muti-thread jni read
        threads.clear();
        auto reader = [&](int num) {
            std::lock_guard<std::mutex> guard(mutex);
            db.FullTextIndexRefresh();
            auto vids =
                db.QueryVertexByFullTextIndex("v1", "name:thread_name" + std::to_string(num), 10);
            UT_EXPECT_EQ(vids.size(), 1);
        };
        for (int i = 0; i < 10; i++) {
            threads.emplace_back(reader, 100 + i);
        }
        for (auto& t : threads) {
            t.join();
        }

        // backup
        std::string backup_dir = "./testdb_backup";
        fma_common::FileSystem& fs = fma_common::FileSystem::GetFileSystem(backup_dir);
        fma_common::file_system::RemoveDir(backup_dir);
        fs.Mkdir(backup_dir);
        db.Backup(backup_dir);
        db.Close();
        config.dir = backup_dir;
        config.ft_index_options.enable_fulltext_index = true;
        config.ft_index_options.fulltext_analyzer = "StandardAnalyzer";
        config.ft_index_options.fulltext_refresh_interval = 1;
        LightningGraph another_db(config);
        another_db.FullTextIndexRefresh();
        vids = another_db.QueryVertexByFullTextIndex("v1", "title:thread", 20);
        UT_EXPECT_EQ(vids.size(), 10);
    }

    {
        DBConfig config;
        config.dir = "./testdb";
        config.ft_index_options.enable_fulltext_index = true;
        config.ft_index_options.fulltext_analyzer = "StandardAnalyzer";
        config.ft_index_options.fulltext_refresh_interval = 1;
        LightningGraph db(config);
        db.DropAllData();
        VertexOptions vo;
        vo.primary_field = "name";
        vo.detach_property = GetParam();
        EdgeOptions eo;
        eo.detach_property = GetParam();
        std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                        {"title", FieldType::STRING, false},
                                        {"description", FieldType::STRING, true},
                                        {"type", FieldType::INT8, false}};
        std::vector<FieldSpec> e_fds = {{"name", FieldType::STRING, false},
                                        {"comments", FieldType::STRING, true},
                                        {"weight", FieldType::FLOAT, false}};
        UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, vo));
        UT_EXPECT_TRUE(db.AddLabel("e1", e_fds, false, eo));
        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "name"));
        UT_EXPECT_TRUE(db.AddFullTextIndex(false, "e1", "name"));

        Transaction txn = db.CreateWriteTxn();
        std::vector<std::string> v1_properties = {"name", "title", "description", "type"};
        VertexId v_id1 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name name1", "title title1 title2", "desc1 desc2", "1"});
        VertexId v_id2 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name name2", "title title3 title4", "desc3 desc4", "1"});
        VertexId v_id3 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name name3", "title title5 title6", "desc5 desc6", "1"});
        VertexId v_id4 = txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name name4", "title title7 title8", "desc7 desc8", "1"});

        std::vector<std::string> e1_properties = {"name", "comments", "weight"};
        txn.AddEdge(v_id1, v_id2, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name name1", "comments comments1 comments2", "10"});
        txn.AddEdge(v_id2, v_id1, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name name2", "comments comments3 comments4", "11"});
        txn.AddEdge(v_id3, v_id4, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name name3", "comments comments5 comments6", "10"});
        EdgeUid e_id4 = txn.AddEdge(
            v_id4, v_id3, std::string("e1"), e1_properties,
            std::vector<std::string>{"name name4", "comments comments7 comments8", "11"});
        txn.Commit();

        db.FullTextIndexRefresh();
        auto vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_TRUE(vids.empty());
        vids = db.QueryVertexByFullTextIndex("v1", "name:name", 10);
        UT_EXPECT_EQ(vids.size(), 4);
        auto eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments", 10);
        UT_EXPECT_TRUE(eids.empty());
        eids = db.QueryEdgeByFullTextIndex("e1", "name:name", 10);
        UT_EXPECT_EQ(eids.size(), 4);

        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "title"));
        db.RebuildFullTextIndex({"v1"}, {});
        db.FullTextIndexRefresh();
        vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_EQ(vids.size(), 4);
        vids = db.QueryVertexByFullTextIndex("v1", "title:title7", 10);
        UT_EXPECT_EQ(vids.size(), 1);
        UT_EXPECT_TRUE(vids[0].first == v_id4);
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments", 10);
        UT_EXPECT_TRUE(eids.empty());
        eids = db.QueryEdgeByFullTextIndex("e1", "name:name", 10);
        UT_EXPECT_EQ(eids.size(), 4);

        UT_EXPECT_TRUE(db.AddFullTextIndex(false, "e1", "comments"));
        db.RebuildFullTextIndex({}, {"e1"});
        db.FullTextIndexRefresh();
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments", 10);
        UT_EXPECT_EQ(eids.size(), 4);
        eids = db.QueryEdgeByFullTextIndex("e1", "comments:comments7", 10);
        UT_EXPECT_EQ(eids.size(), 1);
        UT_EXPECT_TRUE(eids[0].first == e_id4);
        vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_EQ(vids.size(), 4);
        vids = db.QueryVertexByFullTextIndex("v1", "name:name", 10);
        UT_EXPECT_EQ(vids.size(), 4);

        UT_EXPECT_ANY_THROW(db.AlterLabelModFields(
            "v1", std::vector<FieldSpec>({FieldSpec("title", FieldType::INT64, true)}), true,
            nullptr));

        db.AlterLabelDelFields("v1", {"title"}, true, nullptr);
        db.RebuildFullTextIndex({"v1"}, {});
        db.FullTextIndexRefresh();
        vids = db.QueryVertexByFullTextIndex("v1", "title:title", 10);
        UT_EXPECT_TRUE(vids.empty());
        vids = db.QueryVertexByFullTextIndex("v1", "name:name", 10);
        UT_EXPECT_EQ(vids.size(), 4);
    }

    {
        DBConfig config;
        config.dir = "./testdb";
        config.ft_index_options.enable_fulltext_index = true;
        config.ft_index_options.fulltext_analyzer = "SmartChineseAnalyzer";
        config.ft_index_options.fulltext_refresh_interval = 1;
        LightningGraph db(config);
        db.DropAllData();
        VertexOptions vo;
        vo.primary_field = "name";
        vo.detach_property = GetParam();
        std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                        {"description", FieldType::STRING, true},
                                        {"type", FieldType::INT8, false}};
        UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, vo));
        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "description"));
        Transaction txn = db.CreateWriteTxn();
        std::vector<std::string> v1_properties = {"name", "description", "type"};
        txn.AddVertex(std::string("v1"), v1_properties,
                      std::vector<std::string>{
                          "name1",
                          "党的十八大提出，倡导富强、民主、文明、和谐，倡导自由、平等、公正、法"
                          "治，倡导爱国、敬业、诚信、友善，积极培育和践行社会主义核心价值观。",
                          "1"});
        txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{
                "name2", "“富强、民主、文明、和谐”，是我国社会主义现代化国家的建设目标", "1"});
        txn.AddVertex(std::string("v1"), v1_properties,
                      std::vector<std::string>{
                          "name3", "“自由、平等、公正、法治”，是对美好社会的生动表述", "1"});
        txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{"name4", "“爱国、敬业、诚信、友善”，是公民基本道德规范", "1"});
        txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<std::string>{
                "name5", "Apache Lucene is an open source project available for free download",
                "1"});
        txn.AddVertex(std::string("v1"), v1_properties,
                      std::vector<std::string>{
                          "name6",
                          "HUAWEI/华为 P50 "
                          "HarmonyOS2原色双影像单元新款华为智能手机新款华为官方旗舰店p50pro",
                          "1"});
        txn.Commit();
        db.FullTextIndexRefresh();
        auto vids = db.QueryVertexByFullTextIndex("v1", "description:富强", 10);
        UT_EXPECT_EQ(vids.size(), 2);
        vids = db.QueryVertexByFullTextIndex("v1", "description:道德规范", 10);
        UT_EXPECT_EQ(vids.size(), 1);
        vids = db.QueryVertexByFullTextIndex("v1", "description:HUAWEI", 10);
        UT_EXPECT_EQ(vids.size(), 1);
        vids = db.QueryVertexByFullTextIndex("v1", "description:Lucene", 10);
        UT_EXPECT_EQ(vids.size(), 1);
    }
    {
        DBConfig config;
        config.dir = "./testdb";
        config.ft_index_options.enable_fulltext_index = true;
        config.ft_index_options.fulltext_analyzer = "SmartChineseAnalyzer";
        config.ft_index_options.fulltext_commit_interval = 0;
        config.ft_index_options.fulltext_refresh_interval = 0;
        LightningGraph db(config);
        db.DropAllData();
        VertexOptions vo;
        vo.primary_field = "name";
        vo.detach_property = GetParam();
        std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                        {"description", FieldType::STRING, true},
                                        {"type", FieldType::INT8, false}};
        UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, vo));
        UT_EXPECT_TRUE(db.AddFullTextIndex(true, "v1", "description"));
        Transaction txn = db.CreateWriteTxn();
        std::vector<std::string> v1_properties = {"name", "description", "type"};
        txn.AddVertex(std::string("v1"), v1_properties,
                      std::vector<std::string>{
                          "name1",
                          "HUAWEI/华为 P50 "
                          "HarmonyOS2原色双影像单元新款华为智能手机新款华为官方旗舰店p50pro",
                          "1"});
        txn.Commit();
        auto vids = db.QueryVertexByFullTextIndex("v1", "description:HUAWEI", 10);
        UT_EXPECT_EQ(vids.size(), 1);
        UT_EXPECT_TRUE(db.DeleteFullTextIndex(true, "v1", "description"));
    }
}
#endif
