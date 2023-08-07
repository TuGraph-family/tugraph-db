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
#include "fma-common/utils.h"
#include "lgraph/olap_base.h"
#include "import/import_v2.h"
#include "import/import_v3.h"
#include "lgraph/lgraph.h"

const std::vector<std::pair<std::string, std::string>> snb_comment = {
    {"snb_comment.conf",
     R"(
{
    "schema": [
        {
            "label" : "Comment",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT64"}
            ]
        },
        {
            "label" : "replyOf",
            "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "comment.csv",
            "format" : "CSV",
            "label" : "Comment",
            "columns" : ["id","SKIP"]
        },
        {
            "path" : "comment.csv",
            "format" : "CSV",
            "label" : "replyOf",
            "SRC_ID" : "Comment",
            "DST_ID" : "Comment",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
)"},
    {"comment.csv",
     R"(100000,100001
100001,100002
100002,100003
100003,100004
100004,100005
100005,100006
100006,100007
100007,100008
100008,100009
100009,100010
100010,100011
100011,100012
100012,100013
100013,100014
100014,100015
100015,100016
100016,100017
100017,100018
100018,100019
100019,100020
100020,100021
100021,100022
100022,100023
100023,100024
100024,100025
100025,100026
100026,100027
100027,100028
100028,100029
100029,100030
100030,100031
100031,100032
100032,100033
100033,100034
100034,100035
100035,100036
100036,100037
100037,100038
100038,100039
100039,100040
100040,100041
100041,100042
100042,100043
100043,100044
100044,100045
100045,100046
100046,100047
100047,100048
100048,100049
100049,100050
100050,100051
100051,100052
100052,100053
100053,100054
100054,100055
100055,100056
100056,100057
100057,100058
100058,100059
100059,100060
100060,100061
100061,100062
100062,100063
100063,100064
100064,100065
100065,100066
100066,100067
100067,100068
100068,100069
100069,100070
100070,100071
100071,100072
100072,100073
100073,100074
100074,100075
100075,100076
100076,100077
100077,100078
100078,100079
100079,100080
)"}};


// data must be a container of pair<std::string, std::string>, either map<string, string> or
// vector<pair<string, string>>...
template <typename T>
void CreateCsvFiles(const T& data) {
    fma_common::OutputFmaStream stream;
    for (auto& kv : data) {
        const std::string& file_name = kv.first;
        const std::string& data = kv.second;
        if (file_name == "") continue;
        stream.Open(file_name);
        stream.Write(data.data(), data.size());
        stream.Close();
        UT_LOG() << "  " << file_name << " created";
    }
}

// data must be a container of pair<std::string, std::string>, either map<string, string> or
// vector<pair<string, string>>...
template <typename T>
void ClearCsvFiles(const T& data) {
    fma_common::OutputFmaStream stream;
    for (auto& kv : data) {
        const std::string& file_name = kv.first;
        fma_common::file_system::RemoveDir(file_name.c_str());
        UT_LOG() << "  " << file_name << " deleted";
    }
}

void ProcessComments(lgraph_api::Transaction& t, lgraph_api::VertexIterator& commentIt,
                        std::vector<int64_t>& comment_neighbor) {
    for (auto eit = commentIt.GetOutEdgeIterator(); eit.IsValid();
         eit.Next()) {
        auto iit = t.GetVertexIterator();
        iit.Goto(eit.GetDst());
        comment_neighbor.push_back(iit.GetField("id").AsInt64());
    }
}

size_t ProcessCommentCount(lgraph_api::Transaction& t, lgraph_api::VertexIterator& commentIt,
                     size_t vtx) {
    std::vector<int64_t> ans;
    for (auto eit = commentIt.GetOutEdgeIterator(); eit.IsValid();
         eit.Next()) {
        auto iit = t.GetVertexIterator();
        iit.Goto(eit.GetDst());
        ans.push_back(iit.GetField("id").AsInt64());
    }
    return ans.size();
}

class TestOlapVertexTraversal : public TuGraphTest {};

TEST_F(TestOlapVertexTraversal, OlapVertexTraversal) {
    using namespace lgraph_api;
    CreateCsvFiles(snb_comment);
    lgraph::import_v3::Importer::Config config;
    config.delete_if_exists = true;
    config.continue_on_error = true;
    config.db_dir = "./lgraph_db.olap";
    config.config_file = snb_comment[0].first;
    lgraph::import_v3::Importer import1(config);
    import1.DoImportOffline();

    std::string db_path(config.db_dir);

    Galaxy galaxy(db_path, "admin", "73@TuGraph", true, false);
    GraphDB db = galaxy.OpenGraph("default");
    using result_type = std::vector<int64_t>;
    static std::vector<Worker> workers(4);
    auto txn = db.CreateReadTxn();
    std::vector<int64_t> comments;
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        comments.push_back(vit.GetId());
    }
    auto comment_neighbors = ForEachVertex<result_type>(
        db, txn, workers, comments,
        [&](Transaction& t, VertexIterator& vit, result_type& local) {
            ProcessComments(t, vit, local);
        },
        [&](const result_type& local, result_type& res) {
            for (auto item : local) {
                res.push_back(item);
            }
        });
    std::sort(comment_neighbors.begin(), comment_neighbors.end());
    for (int64_t i = 100001; i < 100080; ++i) {
        UT_EXPECT_EQ(comment_neighbors[i-100001], i);
    }

    auto comment_neighbor_count = ForEachVertex<size_t>(
        db, txn, workers, comments,
        [&](Transaction& t, VertexIterator& vit, size_t vtx) {
            return ProcessCommentCount(t, vit, vtx);
        });
    size_t ans = 0;
    for (auto item : comment_neighbor_count) {
        ans += item;
    }
    UT_EXPECT_EQ(ans, 79);

    ClearCsvFiles(snb_comment);
    fma_common::file_system::RemoveDir(config.db_dir);
    fma_common::file_system::RemoveDir(config.intermediate_dir);
}
