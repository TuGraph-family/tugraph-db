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

#include <stdio.h>

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/string_formatter.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"

#include <boost/algorithm/string.hpp>
#include "import/import_v2.h"
#include "lgraph/lgraph.h"
#include "db/galaxy.h"

#include "./graph_factory.h"
#include "./test_tools.h"
using namespace fma_common;
using namespace lgraph;
using namespace import_v2;
using namespace lgraph_api;

class TestImportV2 : public TuGraphTest {};

static void check_import_db(std::string database, size_t num_vertex, size_t num_edge,
                     const IndexSpec* is) {
    lgraph_api::Galaxy galaxy(database,
                              lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS,
                              false, true);
    lgraph_api::GraphDB db = galaxy.OpenGraph("default");
    auto txn = db.CreateReadTxn();
    size_t num_vertices = 0;
    size_t num_edges = 0;
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        num_vertices += 1;
        num_edges += vit.GetNumOutEdges();
    }
    UT_LOG() << "scaned num_vertex :" << num_vertices << " scaned num_edge :" << num_edges;
    UT_EXPECT_EQ(num_vertex, num_vertices);
    UT_EXPECT_EQ(num_edge, num_edges);
    if (is) {
        auto indexes = txn.ListVertexIndexes();
        for (auto& i : indexes) {
            if (i.label == is->label && i.field == is->field && i.type == is->type) return;
        }
        UT_ASSERT(false);
    }
}

// data must be a container of pair<std::string, std::string>, either map<string, string> or
// vector<pair<string, string>>...
template <typename T>
static void CreateCsvFiles(const T& data) {
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
static void ClearCsvFiles(const T& data) {
    fma_common::OutputFmaStream stream;
    for (auto& kv : data) {
        const std::string& file_name = kv.first;
        fma_common::file_system::RemoveDir(file_name.c_str());
        UT_LOG() << "  " << file_name << " deleted";
    }
}

// Test import
// data is a vector of pairs<file_name, file_content>
// the first pair must contain the config file name and content
static void TestImportOnData(const std::vector<std::pair<std::string, std::string>>& data,
                      const Importer::Config& basic_config, size_t num_vertex = 0,
                      size_t num_edge = 0, const IndexSpec* is = nullptr, bool clear = true) {
    CreateCsvFiles(data);
    Importer::Config config = basic_config;
    config.config_file = data[0].first;
    Importer import1(config);
    import1.DoImportOffline();
    if (num_vertex > 0) check_import_db(config.db_dir, num_vertex, num_edge, is);
    ClearCsvFiles(data);
    if (clear) {
        fma_common::file_system::RemoveDir(config.db_dir);
        fma_common::file_system::RemoveDir(config.intermediate_dir);
    }
}

static std::string Times(const std::string& s, unsigned int n) {
    std::stringstream out;
    while (n--) out << s;
    return out.str();
}

class TestImportV2Consistent : public TuGraphTest {
 public:
    std::shared_ptr<lgraph_api::Galaxy> GetImportDb(
        const std::vector<std::pair<std::string, std::string>>& data,
        const Importer::Config& cfg) {
        CreateCsvFiles(data);
        Importer::Config config = cfg;
        config.config_file = data[0].first;
        Importer ipt(config);
        ipt.DoImportOffline();
        ClearCsvFiles(data);
        std::shared_ptr<lgraph_api::Galaxy> galaxy = std::make_shared<lgraph_api::Galaxy>(
            config.db_dir, lgraph::_detail::DEFAULT_ADMIN_NAME,
            lgraph::_detail::DEFAULT_ADMIN_PASS, false, true);
        return galaxy;
    }
    size_t GetVertexNum(lgraph_api::GraphDB& db) {
        auto txn = db.CreateReadTxn();
        size_t num_vertices = 0;
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            num_vertices += 1;
        }
        return num_vertices;
    }
    size_t GetEdgeNum(lgraph_api::GraphDB& db) {
        auto txn = db.CreateReadTxn();
        size_t num_edges = 0;
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            num_edges += vit.GetNumOutEdges();
        }
        return num_edges;
    }
    bool HasVertexIndex(lgraph_api::GraphDB& db, const std::string& label, const std::string& field,
                        lgraph::IndexType type) {
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
    bool HasEdgeIndex(lgraph_api::GraphDB& db, const std::string& label, const std::string& field,
                      lgraph::IndexType type) {
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
    size_t GetVertexIndexValueNum(lgraph_api::GraphDB& db, const std::string& label,
                                  const std::string& field) {
        auto txn = db.CreateReadTxn();
        size_t count = 0;
        for (auto it = txn.GetVertexIndexIterator(label, field, "", ""); it.IsValid(); it.Next()) {
            ++count;
        }
        return count;
    }
    size_t GetEdgeIndexValueNum(lgraph_api::GraphDB& db, const std::string& label,
                                const std::string& field) {
        auto txn = db.CreateReadTxn();
        size_t count = 0;
        for (auto it = txn.GetEdgeIndexIterator(label, field, "", ""); it.IsValid(); it.Next())
            ++count;
        return count;
    }
};

struct V2DataSource {
    std::string import_string_ids = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id"]
        }
    ]
}
)";
    std::string import_conf_index = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "bool", "type":"BOOL", "index":true},
                {"name" : "int8", "type":"INT8", "index":true},
                {"name" : "int16", "type":"INT16", "index":true},
                {"name" : "int32", "type":"INT32", "index":true},
                {"name" : "int64", "type":"INT64", "index":true},
                {"name" : "float", "type":"FLOAT", "index":true},
                {"name" : "double", "type":"DOUBLE", "index":true},
                {"name" : "date", "type":"DATE", "index":true},
                {"name" : "datetime", "type":"DATETIME", "index":true}
            ]
        },
        {
            "label" : "like",
            "type" : "EDGE",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "bool", "type":"BOOL", "index":true},
                {"name" : "int8", "type":"INT8", "index":true},
                {"name" : "int16", "type":"INT16", "index":true},
                {"name" : "int32", "type":"INT32", "index":true},
                {"name" : "int64", "type":"INT64", "index":true},
                {"name" : "float", "type":"FLOAT", "index":true},
                {"name" : "double", "type":"DOUBLE", "index":true},
                {"name" : "date", "type":"DATE", "index":true},
                {"name" : "datetime", "type":"DATETIME", "index":true}
            ]
        }
    ]
}
                    )";
    std::string null_id_string = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )";
    std::string empty_string_field_ids = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )";
    std::string null_string_field = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )";
    std::string missing_uid = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        },
        {
            "path" : "edge.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
                    )";
    std::string missing_uid_skip = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name","SKIP", "SKIP"]
        },
        {
            "path" : "edge.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","SKIP", "DST_ID", "SKIP"]
        }
    ]
}
                    )";
} v2_data;
static const std::vector<std::pair<std::string, std::string>> data_import = {
    {"yago_copycat.conf",
     R"(
{
    "schema": [
        {
            "label" : "Person",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name" : "name", "type":"STRING"},
                {"name" : "birthyear", "type":"INT16", "optional":true},
                {"name" : "phone", "type":"INT16","unique":false, "index":true}
            ]
        },
        {
            "label" : "City",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "Film",
            "type" : "VERTEX",
            "primary" : "title",
            "properties" : [
                {"name" : "title", "type":"STRING"}
            ]
        },
        {"label" : "HAS_CHILD", "type" : "EDGE"},
        {"label" : "MARRIED", "type" : "EDGE"},
        {
            "label" : "BORN_IN",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT", "optional":true}
            ]
        },
        {"label" : "DIRECTED", "type" : "EDGE"},
        {"label" : "WROTE_MUSIC_FOR", "type" : "EDGE"},
        {
            "label" : "ACTED_IN",
            "type" : "EDGE",
            "properties" : [
                {"name" : "charactername", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "person.csv",
            "format" : "CSV",
            "label" : "Person",
            "columns" : ["name","birthyear","phone"]
        },
        {
            "path" : "city.csv",
            "format" : "CSV",
            "header" : 1,
            "label" : "City",
            "columns" : ["name"]
        },
        {
            "path" : "film.csv",
            "format" : "CSV",
            "label" : "Film",
            "columns" : ["title"]
        },
        {
            "path" : "has_child.csv",
            "format" : "CSV",
            "label" : "HAS_CHILD",
            "SRC_ID" : "Person",
            "DST_ID" : "Person",
            "columns" : ["SRC_ID","DST_ID"]
        },
        {
            "path" : "married.csv",
            "format" : "CSV",
            "label" : "MARRIED",
            "SRC_ID" : "Person",
            "DST_ID" : "Person",
            "columns" : ["SRC_ID","DST_ID"]
        },
        {
            "path" : "born_in.csv",
            "format" : "CSV",
            "label" : "BORN_IN",
            "SRC_ID" : "Person",
            "DST_ID" : "City",
            "columns" : ["SRC_ID","DST_ID","weight"]
        },
        {
            "path" : "directed.csv",
            "format" : "CSV",
            "label" : "DIRECTED",
            "SRC_ID" : "Person",
            "DST_ID" : "Film",
            "columns" : ["SRC_ID","DST_ID"]
        },
        {
            "path" : "wrote.csv",
            "format" : "CSV",
            "label" : "WROTE_MUSIC_FOR",
            "SRC_ID" : "Person",
            "DST_ID" : "Film",
            "columns" : ["SRC_ID","DST_ID"]
        },
        {
            "path" : "acted_in.csv",
            "format" : "CSV",
            "label" : "ACTED_IN",
            "SRC_ID" : "Person",
            "DST_ID" : "Film",
            "columns" : ["SRC_ID","DST_ID","charactername"]
        }
    ]
}
                )"},
    {"person.csv",
     R"(Rachel Kempson,1910,10086
Michael Redgrave,1908,10087
Vanessa Redgrave,1937,10088
Corin Redgrave,1939,10089
Liam Neeson,1952,10090
Natasha Richardson,1963,10091
Richard Harris,1930,10092
Dennis Quaid,1954,10093
Lindsay Lohan,1986,10094
Jemma Redgrave,1965,10095
Roy Redgrave,1873,10096
John Williams,1932,10097
Christopher Nolan,1970,10098
)"},
    {"city.csv",
     R"(New York
London
Houston
)"},
    {"film.csv",
     R"("Goodbye, Mr. Chips"
Batman Begins
Harry Potter and the Sorcerer's Stone
The Parent Trap
Camelot
)"},
    {"has_child.csv",
     R"(Rachel Kempson,Vanessa Redgrave
Rachel Kempson,Corin Redgrave
Michael Redgrave,Vanessa Redgrave
Michael Redgrave,Corin Redgrave
Corin Redgrave,Jemma Redgrave
Vanessa Redgrave,Natasha Richardson
Roy Redgrave,Michael Redgrave
)"},
    {"married.csv",
     R"(Rachel Kempson,Michael Redgrave
Michael Redgrave,Rachel Kempson
Natasha Richardson,Liam Neeson
Liam Neeson,Natasha Richardson
)"},
    {"born_in.csv",
     R"(Vanessa Redgrave,London,20.21
Natasha Richardson,London,20.18
Christopher Nolan,London,19.93
Dennis Quaid,Houston,19.11
Lindsay Lohan,New York,20.62
John Williams,New York,20.55
)"},
    {"directed.csv",
     R"(Christopher Nolan,Batman Begins
)"},
    {"wrote.csv",
     R"(John Williams,Harry Potter and the Sorcerer's Stone
John Williams,"Goodbye, Mr. Chips"
)"},
    {"acted_in.csv",
     R"(Michael Redgrave,"Goodbye, Mr. Chips",The Headmaster
Vanessa Redgrave,Camelot,Guenevere
Richard Harris,Camelot,King Arthur
Richard Harris,Harry Potter and the Sorcerer's Stone,Albus Dumbledore
Natasha Richardson,The Parent Trap,Liz James
Dennis Quaid,The Parent Trap,Nick Parker
Lindsay Lohan,The Parent Trap,Halle/Annie
Liam Neeson,Batman Begins,Henri Ducard
)"}};

static const std::vector<std::pair<std::string, std::string>> data_mock_snb = {
    {"mock_snb.conf",
     R"(
{
    "schema": [
        {
            "label" : "Comment",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT64"},
                {"name" : "creationDate", "type":"INT64"}
            ]
        },
        {
            "label" : "Place",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT64"},
                {"name" : "name", "type" : "STRING"}
            ]
        },
        {
            "label" : "Post",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT64"},
                {"name" : "creationDate", "type":"INT64"}
            ]
        },
        {"label" : "commentIsLocatedIn", "type" : "EDGE"},
        {
            "label" : "replyOf",
            "type" : "EDGE",
            "properties" : [
                {"name" : "creationDate", "type":"INT64"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "comment.csv",
            "format" : "CSV",
            "label" : "Comment",
            "columns" : ["id","creationDate"]
        },
        {
            "path" : "comment_isLocatedIn_place.csv",
            "format" : "CSV",
            "header" : 0,
            "label" : "commentIsLocatedIn",
            "SRC_ID" : "Comment",
            "DST_ID" : "Place",
            "columns" : ["SRC_ID","SKIP","SKIP","DST_ID","SKIP"]
        },
        {
            "path" : "comment_replyOf_comment.csv",
            "format" : "CSV",
            "label" : "replyOf",
            "SRC_ID" : "Comment",
            "DST_ID" : "Comment",
            "columns" : ["SRC_ID","DST_ID","creationDate"]
        },
        {
            "path" : "comment_replyOf_post.csv",
            "format" : "CSV",
            "label" : "replyOf",
            "SRC_ID" : "Comment",
            "DST_ID" : "Post",
            "columns" : ["SRC_ID","DST_ID","creationDate"]
        },
        {
            "path" : "place.csv",
            "format" : "CSV",
            "label" : "Place",
            "columns" : ["id","name"]
        },
        {
            "path" : "post.csv",
            "format" : "CSV",
            "label" : "Post",
            "columns" : ["id","creationDate"]
        }
    ]
}
)"},
    {"comment.csv",
     R"(100001,20200103
100002,20200214
100003,20200401
)"},
    {"place.csv",
     R"(200001,Shanghai
200002,Beijing
200003,Guangzhou
200004,Hong Kong
)"},
    {"post.csv",
     R"(300001,20200102
300002,20200213
300003,20200401
)"},
    {"comment_isLocatedIn_place.csv",
     R"(100001,bala,bala,200004,bala
100003,bala,bala,200001,bala
)"},
    {"comment_replyOf_comment.csv",
     R"(100003,100001,20200401
100002,100001,20200214
)"},
    {"comment_replyOf_post.csv",
     R"(100001,300002,20200102
100002,300002,20200214
)"}};

TEST_F(TestImportV2, ImportV2) {
    // testing import with large string id
    {
        UT_LOG() << "Parsing large string ids";
        const std::string dbdir = "./testdb";
        const std::string srcdir = "./import_data/";
        AutoCleanDir db(dbdir);
        AutoCleanDir src(srcdir);
        Importer::Config config;
        config.delete_if_exists = true;
        CreateCsvFiles(std::map<std::string, std::string>(
            {{"import.conf", v2_data.import_string_ids},
             {"node.csv",
              UT_FMT("\"{}\"\n", std::string(lgraph::_detail::MAX_KEY_SIZE + 1, '1'))}}));
        SubProcess p(UT_FMT("./lgraph_import -c import.conf --continue_on_error 0 -d {}", dbdir));
        p.ExpectOutput("too long");
        p.Wait();
        UT_EXPECT_NE(p.GetExitCode(), 0);
    }
    {
        UT_LOG() << "Test empty field";
        Importer::Config config;
        config.delete_if_exists = true;
        UT_EXPECT_ANY_THROW(TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "user",
            "type" : "VERTEX",
            "primary" : "uid",
            "properties" : [
                {"name":"uid", "type":"STRING"}
            ]
        },
        {
            "label" : "friends",
            "type" : "EDGE",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        }
    ]
    "files" : [
        {
            "path" : "edge.csv",
            "format" : "CSV",
            "label" : "friends",
            "SRC_ID" : "user",
            "DST_ID" : "user",
            "columns" : ["SRC_ID","id","DST_ID"]
        }
    ]
}
                    )"},
                                                             {"edge.csv",
                                                              "wangtao,12,wangtao\n"
                                                              "wangtao,14,wangtao1\n"
                                                              "chengyi,15,wangtao1\n"
                                                              "chuntao,16,chuntao3\n"}},
            config));
    }
    {
        UT_LOG() << "Test index";

        Importer::Config config;
        config.delete_if_exists = true;
        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{
                {"import.conf", v2_data.import_conf_index}},
            config);

        UT_EXPECT_ANY_THROW(
            TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"schema.config", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"BLOB"}
            ]
        }
    ]
}
                    )"}},
                             config));
    }
    {
        UT_LOG() << "Test null ID string";
        Importer::Config config;
        config.delete_if_exists = true;
        UT_EXPECT_ANY_THROW(TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{
                {"import.conf", v2_data.null_id_string},
                {"node.csv",
                 "id001,name001\n"
                 ",name002\n"}},
            config));
    }
    {
        UT_LOG() << "Test empty string field";
        Importer::Config config;
        config.delete_if_exists = true;
        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{
                {"import.conf", v2_data.empty_string_field_ids},
                {"node.csv",
                 "\"id001\",\"name001\"\n"
                 "\"id002\",\"\"\n"}},
            config, 2, 0);
    }
    {
        UT_LOG() << "Test null string field";
        Importer::Config config;
        config.delete_if_exists = true;
        UT_EXPECT_ANY_THROW(TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{
                {"import.conf", v2_data.null_string_field},
                {"node.csv",
                 "\"id001\",\"name001\"\n"
                 "\"id002\",\n"}},
            config));
    }

    {
        UT_LOG() << "Test missing uid";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf",
                                                                           v2_data.missing_uid},
                                                                          {"node.csv",
                                                                           "\"id001\",\"name001\"\n"
                                                                           "\"id002\",\"name002\"\n"
                                                                           "\"\",\"name003\"\n"},
                                                                          {"edge.csv",
                                                                           "\"id001\",\"id002\"\n"
                                                                           "\"id002\",\"\"\n"}},
                         config, 2, 1);
    }
    {
        UT_LOG() << "Test missing uid with skip";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{
                {"import.conf", v2_data.missing_uid_skip},
                {"node.csv",
                 "1,\"name001\",2,\"name002\"\n"
                 "1,\"name001\",1,\"name001\"\n"},
                {"edge.csv",
                 "1,\"name001\",2,\"name002\"\n"
                 "1,\"name001\",1,\"name001\"\n"}},
            config, 1, 1);
    }
    {
        UT_LOG() << "Test overwrite graph db";
        Importer::Config config;
        GraphFactory::create_yago("db");
        config.config_file = "yago.conf";
        config.db_dir = "db";
        {
            // overwrite succ
            config.delete_if_exists = true;
            Importer import(config);
            import.DoImportOffline();
            check_import_db(config.db_dir, 21, 28, nullptr);
        }
        {
            // overwrite fail
            config.delete_if_exists = false;
            Importer import(config);
            UT_EXPECT_ANY_THROW(import.DoImportOffline());
        }
    }
    {
        UT_LOG() << "Test import with config_file (SCHEME-1)";
        Importer::Config config;
        config.config_file = "yago_copycat.conf";
        config.continue_on_error = true;
        config.delete_if_exists = true;

        IndexSpec is_check;
        is_check.label = "Person";
        is_check.field = "phone";
        is_check.type = lgraph::IndexType::NonuniqueIndex;
        TestImportOnData(data_import, config, 20, 26, &is_check);
    }
    {
        // test content: 1. SKIP 2. all-SKIP 3. duplicated edge labels
        UT_LOG() << "Test import mock-SNB with config_file (SCHEME-1)";
        Importer::Config config;
        config.config_file = "mock_snb.conf";
        config.continue_on_error = true;
        config.delete_if_exists = true;
        TestImportOnData(data_mock_snb, config, 10, 6);
    }

    {
        UT_LOG() << "Test one label multi csv";
        Importer::Config config;
        config.delete_if_exists = true;
        std::vector<std::pair<std::string, std::string>> data = {{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        },
        {
            "path" : "node2.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        },
        {
            "path" : "edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID"]
        },
        {
            "path" : "edge2.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
                    )"},
                                                                 {"node1.csv",
                                                                  "id001,name001\n"
                                                                  "id002,name002\n"},
                                                                 {"node2.csv",
                                                                  "id003,name003\n"
                                                                  "id004,name004\n"},
                                                                 {"edge1.csv",
                                                                  "id001,id003\n"
                                                                  "id001,id003\n"},
                                                                 {"edge2.csv", "id001,id002\n"}};
        TestImportOnData(data, config, 4, 3);
    }

    {
        // dup id
        UT_LOG() << "Test duplicated id";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        std::vector<std::pair<std::string, std::string>> data = {{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                                 {"node1.csv",
                                                                  "id001,name001\n"
                                                                  "id001,name002\n"
                                                                  "id001,name002\n"}};
        TestImportOnData(data, config, 1, 0);
    }
    {
        // skip
        UT_LOG() << "Test @SKIP in csv";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        std::vector<std::pair<std::string, std::string>> data = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type" : "FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["SKIP","id","SKIP","name","SKIP"]
        },
        {
            "path" : "edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SKIP","SRC_ID","weight","SKIP","DST_ID"]
        }
    ]
}
                    )"},
            {"node1.csv",
             "skip,001,skip,name001,skip\n"
             "skip,002,skip,name002,skip\n"},
            {"edge1.csv", "skip,001,1.1,skip,002\n"}};
        TestImportOnData(data, config, 2, 1);
    }
    {
        // src_id not exists
        UT_LOG() << "Test src_id not exists in vertex";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        std::vector<std::pair<std::string, std::string>> data = {{"schema.config", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
             "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"]
        },
        {
            "path" : "edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
                    )"},
                                                                 {"node1.csv",
                                                                  "id001,name001\n"
                                                                  "id002,name002\n"},
                                                                 {"edge1.csv",
                                                                  "id001,id002\n"
                                                                  "id003,id002\n"
                                                                  "id003,id005\n"}};
        TestImportOnData(data, config, 2, 1);
    }
    {
        // many fields
        UT_LOG() << "Test many fields";
        Importer::Config config;
        config.delete_if_exists = true;

        std::string conf = R"(
    {
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE"
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","name"@replace]
        },
        {
            "path" : "edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID"@replace]
        }
    ]
}
            )";
        boost::replace_all(conf, "@replace", Times(",\"SKIP\"", 1000));

        std::vector<std::pair<std::string, std::string>> data = {
            {"schema.config", conf},
            {"node1.csv", "id001,name001" + Times(",", 1000) +
                              "\n"
                              "id002,name002" +
                              Times(",", 1000) + "\n"},
            {"edge1.csv", "id001,id002" + Times(",", 1000) +
                              "\n"
                              "id001,id002" +
                              Times(",", 1000) + "\n"}};
        TestImportOnData(data, config, 2, 2);
    }
    {
        UT_LOG() << "Testing with large records.";
        std::string db_dir = "./lgraph_db";
        AutoCleanDir db_dir_cleaner(db_dir);
        auto ImportWithRecordOfSize = [](size_t n_strs, size_t str_size, bool expect_ret_zero,
                                         const std::string& expect_msg) {
            std::string db_dir = "./lgraph_db";
            std::string dir = "./import_data";
            // std::string import_cfg = dir + "/import.config";
            // std::string schema_cfg = dir + "/schema.config";
            std::string import_conf = dir + "/import.conf";
            std::string csv = dir + "/node.csv";

            AutoCleanDir cleaner(dir);

            nlohmann::json conf;
            conf = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        }
    ],
    "file" : []
}
                )"_json;

            // conf["schema"][0]["type"] = "VERTEX";
            // conf["schema"][0]["name"] = "node";
            // conf["schema"][0]["properties"]["id"] = R"({"type" : "INT32","id":true})"_json;
            for (size_t i = 0; i < n_strs; i++) {
                nlohmann::json c = R"({"type" : "STRING"})"_json;
                c["name"] = "s" + std::to_string(i);
                conf["schema"][0]["properties"].push_back(c);
            }

            nlohmann::json file;
            file["path"] = csv;
            file["format"] = "CSV";
            file["label"] = "node";
            file["columns"].push_back("id");
            for (size_t i = 0; i < n_strs; i++) file["columns"].push_back("s" + std::to_string(i));
            conf["files"].push_back(file);
            std::map<std::string, std::string> data;
            data[import_conf] = conf.dump();

            data[csv].append("1");
            for (size_t i = 0; i < n_strs; i++)
                data[csv].append("," + std::string(str_size, '0' + (char)i));
            CreateCsvFiles(data);
            std::string cmd = UT_FMT(
                "./lgraph_import --online false -c {} "
                "--overwrite true --continue_on_error false -d {}",
                import_conf, db_dir);
            UT_LOG() << cmd;
            SubProcess proc(cmd);
            proc.Wait();
            UT_LOG() << "expect: " << expect_msg;
            UT_EXPECT_TRUE(proc.ExpectOutput(expect_msg));
            if (expect_ret_zero) UT_EXPECT_EQ(proc.GetExitCode(), 0);
        };
        // this should be ok
        ImportWithRecordOfSize(1, std::min<size_t>(lgraph::_detail::MAX_STRING_SIZE, 64<<20),
                               true,
                               "Import finished");
        // this should fail due to string too large
        ImportWithRecordOfSize(1, lgraph::_detail::MAX_STRING_SIZE + 1, false, "Data size");
        // this should fail due to record too large
        ImportWithRecordOfSize(
            lgraph::_detail::MAX_PROP_SIZE / lgraph::_detail::MAX_STRING_SIZE + 1,
            lgraph::_detail::MAX_STRING_SIZE, false, "Record size");
    }
    {
        UT_LOG() << "Testing with BLOBS";
        std::string dir = "./import_data";
        std::string db_dir = "./lgraph_db";
        AutoCleanDir cleaner(dir);
        AutoCleanDir db_cleaner(db_dir);

        // std::string schema_cfg = dir + "/schema.config";
        // std::string import_cfg = dir + "/import.config";
        std::string import_conf = dir + "/import.conf";
        std::string v1 = dir + "/node1.csv";
        std::string v2 = dir + "/node2.csv";
        std::string e1 = dir + "/edge1.csv";
        std::string e2 = dir + "/edge2.csv";

        auto WriteFile = [](const std::string& name, const std::string& content) {
            OutputFmaStream out(name);
            out.Write(content.data(), content.size());
        };
        auto TryImport = [&](const std::string& expect_output, int expect_ec) {
            SubProcess proc(
                UT_FMT("./lgraph_import --online false -c {} "
                        "--overwrite true --continue_on_error false --v3 false",
                        import_conf));
            UT_EXPECT_TRUE(proc.ExpectOutput(expect_output, 100 * 1000));
            proc.Wait();
            UT_EXPECT_EQ(proc.GetExitCode(), expect_ec);
        };

        auto TryImportV3 = [&](const std::string& expect_output, int expect_ec) {
            SubProcess proc(
                UT_FMT("./lgraph_import --online false -c {} "
                       "--overwrite true --continue_on_error false --v3 true",
                       import_conf));
            UT_EXPECT_TRUE(proc.ExpectOutput(expect_output, 100 * 1000));
            proc.Wait();
            UT_EXPECT_EQ(proc.GetExitCode(), expect_ec);
        };

        auto ValidateGraph = [&](const std::function<void(lgraph_api::GraphDB&)>& validate) {
            lgraph_api::Galaxy galaxy(db_dir);
            galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                  lgraph::_detail::DEFAULT_ADMIN_PASS);
            lgraph_api::GraphDB graph = galaxy.OpenGraph("default");
            validate(graph);
        };
        std::string conf = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "img1", "type":"BLOB","optional":true},
                {"name" : "img2", "type":"BLOB","optional":true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "img", "type":"BLOB","optional":true},
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "@replace",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id","img1"]
        }
    ]
}
            )";
        boost::replace_all(conf, "@replace", v1);
        WriteFile(import_conf, conf);
        // not valid base64
        WriteFile(v1, UT_FMT("1,{}\n2,{}", std::string(52, 'a'), std::string(8191, 'b')));
        db_cleaner.Clean();
        TryImport("Value is not a valid BASE64 string", 1);

        // ok
        WriteFile(v1, UT_FMT("1,{}\n2,{}", std::string(8192, 'b'), std::string(52, 'a')));
        db_cleaner.Clean();
        UT_LOG() << "<<<<<" << __LINE__;
        TryImport("Import finished", 0);
        ValidateGraph([](lgraph_api::GraphDB& g) {
            auto txn = g.CreateReadTxn();
            auto it1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
            UT_EXPECT_EQ(it1.GetField("img1").AsBase64Blob(), std::string(8192, 'b'));
            auto it2 = txn.GetVertexByUniqueIndex("node", "id", FieldData(2));
            UT_EXPECT_EQ(it2.GetField("img1").AsBase64Blob(), std::string(52, 'a'));
        });

        std::string conf1 = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "img1", "type":"BLOB","optional":true},
                {"name" : "img2", "type":"BLOB","optional":true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "img", "type":"BLOB","optional":true},
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "@replace",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["img1", "id","img2"]
        }
    ]
}
            )";
        boost::replace_all(conf1, "@replace", v1);
        // test with really large blobs
        WriteFile(import_conf, conf1);
        WriteFile(v1,
                  UT_FMT("{},1,{}\n{},2,{}", std::string(1 << 24, 'a'), std::string(1 << 24, 'b'),
                          std::string(1 << 24, 'c'), std::string(1 << 24, 'd')));
        db_cleaner.Clean();
        TryImport("Import finished", 0);
        ValidateGraph([](lgraph_api::GraphDB& g) {
            auto txn = g.CreateReadTxn();
            auto it1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
            UT_EXPECT_EQ(it1.GetField("img1").AsBase64Blob(), std::string(1 << 24, 'a'));
            UT_EXPECT_EQ(it1.GetField("img2").AsBase64Blob(), std::string(1 << 24, 'b'));
            auto it2 = txn.GetVertexByUniqueIndex("node", "id", FieldData(2));
            UT_EXPECT_EQ(it2.GetField("img1").AsBase64Blob(), std::string(1 << 24, 'c'));
            UT_EXPECT_EQ(it2.GetField("img2").AsBase64Blob(), std::string(1 << 24, 'd'));
        });

        std::string conf2 = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "img1", "type":"BLOB","optional":true},
                {"name" : "img2", "type":"BLOB","optional":true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "img", "type":"BLOB","optional":true},
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "@replace1",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["img1","id","img2"]
        },
        {
            "path" : "@replace2",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["img1","id","img2"]
        },
        {
            "path" : "@replace3",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","img","weight"]
        },
        {
            "path" : "@replace4",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","img","weight"]
        }
    ]
}
            )";
        boost::replace_all(conf2, "@replace1", v1);
        boost::replace_all(conf2, "@replace2", v2);
        boost::replace_all(conf2, "@replace3", e1);
        boost::replace_all(conf2, "@replace4", e2);
        // test with edges
        WriteFile(import_conf, conf2);
        WriteFile(v1, UT_FMT("{},1,{}\n{},2,{}", std::string(8196, 'a'), std::string(8192, 'a'),
                              std::string(52, 'b'), std::string(56, 'b')));
        WriteFile(v2, UT_FMT("{},3,{}\n{},4,{}", "", std::string(1024, 'a'), "",
                              std::string(2048, 'b')));
        WriteFile(e1,
                  UT_FMT("1,2,{},1.0\n2,3,{},2.0", std::string(100, 'e'), std::string(204, 'f')));
        WriteFile(
            e2, UT_FMT("3,4,{},3.0\n4,1,{},4.0", std::string(1048, 'e'), std::string(2096, 'f')));
        db_cleaner.Clean();
        TryImport("Import finished", 0);
        ValidateGraph([](lgraph_api::GraphDB& g) {
            auto txn = g.CreateReadTxn();
            auto it1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
            UT_EXPECT_EQ(it1.GetField("img1").AsBase64Blob(), std::string(8196, 'a'));
            auto eit12 = it1.GetOutEdgeIterator();
            UT_EXPECT_EQ(txn.GetVertexIterator(eit12.GetDst()).GetField("id").AsInt32(), 2);
            UT_EXPECT_EQ(eit12.GetField("img").AsBase64Blob(), std::string(100, 'e'));
            auto it4 = txn.GetVertexByUniqueIndex("node", "id", FieldData(4));
            UT_EXPECT_TRUE(it4.GetField("img1").IsNull());
            auto eit34 = it4.GetInEdgeIterator();
            UT_EXPECT_EQ(txn.GetVertexIterator(eit34.GetSrc()).GetField("id").AsInt32(), 3);
            UT_EXPECT_EQ(eit34.GetField("img").AsBase64Blob(), std::string(1048, 'e'));
        });
        UT_LOG() << "-----------------------------";
        std::string conf3 = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "temporal" : "ts",
            "properties" : [
                {"name" : "ts", "type" : "INT64"},
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "@replace1",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id"]
        },
        {
            "path" : "@replace2",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","ts","weight"]
        }
    ]
}
            )";
        boost::replace_all(conf3, "@replace1", v1);
        boost::replace_all(conf3, "@replace2", e1);
        // test with really large blobs
        WriteFile(import_conf, conf3);
        WriteFile(v1, UT_FMT("1\n2\n3\n4"));
        WriteFile(e1, UT_FMT("1,2,100,1.1\n1,4,50,4.4\n1,3,70,5.5\n2,3,200,2.2\n3,4,300,3.3"));
        db_cleaner.Clean();
        TryImport("Import finished", 0);
        ValidateGraph([](lgraph_api::GraphDB& g) {
            auto txn = g.CreateReadTxn();
            auto it1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
            auto eit12 = it1.GetOutEdgeIterator();
            UT_EXPECT_EQ(eit12.GetField("ts").AsInt64(), 50);
            eit12.Next();
            UT_EXPECT_EQ(eit12.GetField("ts").AsInt64(), 70);
            eit12.Next();
            UT_EXPECT_EQ(eit12.GetField("ts").AsInt64(), 100);
        });

        // V3 tid test
        db_cleaner.Clean();
        TryImportV3("Import finished", 0);
        ValidateGraph([](lgraph_api::GraphDB& g) {
            auto txn = g.CreateReadTxn();
            auto it1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
            auto eit12 = it1.GetOutEdgeIterator();
            UT_EXPECT_EQ(eit12.GetField("ts").AsInt64(), 50);
            eit12.Next();
            UT_EXPECT_EQ(eit12.GetField("ts").AsInt64(), 70);
            eit12.Next();
            UT_EXPECT_EQ(eit12.GetField("ts").AsInt64(), 100);
        });
    }
}

TEST_F(TestImportV2, ImportJson) {
    {
        UT_LOG() << "Test import json, with extra space at the end";
        Importer::Config config;
        config.delete_if_exists = true;
        TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.json",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                                          {"node.json", R"(
["1","name1"]
["2","name2"]
["3","name3"]
                    )"}},
                         config, 3, 0);
    }

    {
        UT_LOG() << "Test import json, with extra newline";
        Importer::Config config;
        config.delete_if_exists = true;
        TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.json",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                                          {"node.json",
                                                                           "[\"1\",\"name1\"]\n"
                                                                           "[\"2\",\"name2\"]\n"
                                                                           "[\"3\",\"name3\"]\n"}},
                         config, 3, 0);

        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.json",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                             {"node.json",
                                                              "\n\n[\"1\",\"name1\"]\n\n\n\n\n"
                                                              "\n\n\n [\"2\",  \"name2\"] \n"
                                                              "[\"3\",\"name3\"]\n\n\n\n\n\n"}},
            config, 3, 0);
    }

    {
        UT_LOG() << "Test import json, error json line";
        Importer::Config config;
        config.delete_if_exists = true;
        std::string conf = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.json",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
            )";
        UT_EXPECT_ANY_THROW(
            TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", conf},
                                                                              {"node.json", R"(
["1","name1"
["2","name2"]
["3","name3"]
                    )"}},
                             config, 3, 0));

        UT_EXPECT_ANY_THROW(
            TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", conf},
                                                                              {"node.json", R"(
["1"]
["2","name2"]
["3","name3"]
                    )"}},
                             config, 3, 0));

        UT_EXPECT_ANY_THROW(
            TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", conf},
                                                                              {"node.json", R"(
[1,"name1"]
["2","name2"]
["3","name3"]
                    )"}},
                             config, 3, 0));
    }

    {
        UT_LOG() << "Test null json string field";
        Importer::Config config;
        config.delete_if_exists = true;
        UT_EXPECT_ANY_THROW(TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                             {"node.csv",
                                                              "[\"id001\",\"name001\"]\n"
                                                              "[\"id002\"]\n"}},
            config));
    }

    {
        UT_LOG() << "Test SKIP in json";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        std::vector<std::pair<std::string, std::string>> data = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type" : "FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["SKIP","id","SKIP","name","SKIP"]
        },
        {
            "path" : "edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SKIP","SRC_ID","weight","SKIP","DST_ID"]
        }
    ]
}
                    )"},
            {"node1.csv",
             "[\"skip\",1,\"skip\",\"name001\",\"skip\"]\n"
             "[\"skip\",2,\"skip\",\"name002\",\"skip\"]\n"},
            {"edge1.csv", "[\"skip\",1,1.1,\"skip\",2]\n"}};
        TestImportOnData(data, config, 2, 1);
    }

    {
        UT_LOG() << "Test detaching property";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        std::vector<std::pair<std::string, std::string>> data = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING"}
            ],
            "detach_property" : true
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type" : "FLOAT"}
            ],
            "detach_property" : true
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["SKIP","id","SKIP","name","SKIP"]
        },
        {
            "path" : "edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SKIP","SRC_ID","weight","SKIP","DST_ID"]
        }
    ]
}
                    )"},
            {"node1.csv",
             "[\"skip\",1,\"skip\",\"name001\",\"skip\"]\n"
             "[\"skip\",2,\"skip\",\"name002\",\"skip\"]\n"},
            {"edge1.csv", "[\"skip\",1,1.1,\"skip\",2]\n"
             "[\"skip\",1,2.2,\"skip\",2]\n"}};
        TestImportOnData(data, config, 2, 2);
    }

    {
        UT_LOG() << "Test edge constraints";
        Importer::Config config;
        config.delete_if_exists = true;
        UT_EXPECT_ANY_THROW(TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node1",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "node2",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "constraints" : [["node1", "node1"]]
        }
    ],
    "files" : [
        {
            "path" : "edge.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node1",
            "DST_ID" : "node2",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
                    )"},
                                                             {"node1.csv",
                                                              "[\"id001\",\"name001\"]\n"
                                                              "[\"id002\",\"name002\"]\n"},
                                                             {"node2.csv",
                                                              "[\"id001\",\"name001\"]\n"
                                                              "[\"id002\",\"name002\"]\n"},
                                                             {"edge.csv", R"(["id001","id002"])"}},
            config));

        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node1",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "node2",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "constraints" : [["node1", "node2"]]
        }
    ],
    "files" : [
        {
            "path" : "node1.json",
            "format" : "JSON",
            "label" : "node1",
            "columns" : ["id","name"]
        },
        {
            "path" : "node2.json",
            "format" : "JSON",
            "label" : "node2",
            "columns" : ["id","name"]
        },
        {
            "path" : "edge.json",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node1",
            "DST_ID" : "node2",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
                    )"},
                                                             {"node1.json",
                                                              "[\"id001\",\"name001\"]\n"
                                                              "[\"id002\",\"name002\"]\n"},
                                                             {"node2.json",
                                                              "[\"id003\",\"name001\"]\n"
                                                              "[\"id004\",\"name002\"]\n"},
                                                             {"edge.json",
                                                              R"(["id001","id003"]
["id001","id004"]
["id002","id003"]
["id001","id004"])"}},
            config, 4, 4);
    }

    {
        UT_LOG() << "Test parsing jsonline data with extra space";
        Importer::Config config;
        config.delete_if_exists = true;
        TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.jsonl",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                                          {"node.jsonl",
                                                                           R"(
    ["1","name1"]
            
["2","name2"]

   ["3","name3"]
["4","name4"]
               
                    )"}},
                         config, 4);
    }

    {
        UT_LOG() << "Test continue_on_error with jsonline data";
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        TestImportOnData(std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node.jsonl",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                    )"},
                                                                          {"node.jsonl",
                                                                           R"(
["1","name1"]
["2","name2"]
[sdacdsc]
["3","name3"]
["4"]
["4","name4"]
12345
["5","name5"]
                    )"}},
                         config, 5);
    }

#ifdef LGRAPH_ENABLE_FULLTEXT_INDEX
    // fulltext index
    {
        UT_LOG() << "Test importing with fulltext index enabled";
        Importer::Config config;
        config.delete_if_exists = true;
        config.enable_fulltext_index = true;
        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "node1",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING", "fulltext":true}
            ]
        },
        {
            "label" : "node2",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "desc", "type":"STRING", "fulltext":true}
            ],
            "constraints" : [["node1", "node2"]]
        }
    ],
    "files" : [
        {
            "path" : "node1.json",
            "format" : "JSON",
            "label" : "node1",
            "columns" : ["id","name"]
        },
        {
            "path" : "node2.json",
            "format" : "JSON",
            "label" : "node2",
            "columns" : ["id","name"]
        },
        {
            "path" : "edge.json",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node1",
            "DST_ID" : "node2",
            "columns" : ["SRC_ID","DST_ID", "desc"]
        }
    ]
}
                    )"},
                                                             {"node1.json",
                                                              R"(["id001","name name001"]
["id002","name name002"])"},
                                                             {"node2.json",
                                                              R"(["id003","name001"]
["id004","name002"])"},
                                                             {"edge.json",
                                                              R"(["id001","id003", "desc desc1"]
["id001","id004", "desc desc2"]
["id002","id003", "desc desc3"]
["id001","id004", "desc desc4"])"}},
            config, 4, 4, nullptr, false);
        {
            lgraph::Galaxy::Config conf;
            conf.dir = config.db_dir;
            conf.durable = false;
            conf.optimistic_txn = false;
            conf.load_plugins = false;
            std::shared_ptr<GlobalConfig> gc(new lgraph::GlobalConfig);
            gc->ft_index_options.enable_fulltext_index = true;
            lgraph::Galaxy galaxy(conf, false, gc);
            auto db = galaxy.OpenGraph("admin", "default");
            auto vids = db.QueryVertexByFullTextIndex("node1", "name:name", 10);
            UT_EXPECT_TRUE(vids.size() == 2);
            auto eids = db.QueryEdgeByFullTextIndex("edge", "desc:desc", 10);
            UT_EXPECT_TRUE(eids.size() == 4);
        }
        fma_common::file_system::RemoveDir(config.db_dir);
        fma_common::file_system::RemoveDir(config.intermediate_dir);
    }
#endif

    {
        UT_LOG() << "Test indexes with various numeric types";
        Importer::Config config;
        config.delete_if_exists = true;
        TestImportOnData(
            std::vector<std::pair<std::string, std::string>>{{"import.conf", R"(
{
    "schema": [
        {
            "label" : "string",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"}
            ]
        },
        {
            "label" : "int8",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT8"}
            ]
        },
        {
            "label" : "int16",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT16"}
            ]
        },
        {
            "label" : "int32",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "int64",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT64"}
            ]
        },
        {
            "label" : "float",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"FLOAT"}
            ]
        },
        {
            "label" : "double",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"DOUBLE"}
            ]
        },
        {
            "label" : "date",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"DATE"}
            ]
        },
        {
            "label" : "datetime",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"DATETIME"}
            ]
        },
        {
            "label" : "bool",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"BOOL"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "string.json",
            "format" : "JSON",
            "label" : "string",
            "columns" : ["id"]
        },
        {
            "path" : "int8.json",
            "format" : "JSON",
            "label" : "int8",
            "columns" : ["id"]
        },
        {
            "path" : "int16.json",
            "format" : "JSON",
            "label" : "int16",
            "columns" : ["id"]
        },
        {
            "path" : "int32.json",
            "format" : "JSON",
            "label" : "int32",
            "columns" : ["id"]
        },
        {
            "path" : "int64.json",
            "format" : "JSON",
            "label" : "int64",
            "columns" : ["id"]
        },
        {
            "path" : "float.json",
            "format" : "JSON",
            "label" : "float",
            "columns" : ["id"]
        },
        {
            "path" : "double.json",
            "format" : "JSON",
            "label" : "double",
            "columns" : ["id"]
        },
        {
            "path" : "date.json",
            "format" : "JSON",
            "label" : "date",
            "columns" : ["id"]
        },
        {
            "path" : "datetime.json",
            "format" : "JSON",
            "label" : "datetime",
            "columns" : ["id"]
        },
        {
            "path" : "bool.json",
            "format" : "JSON",
            "label" : "bool",
            "columns" : ["id"]
        }
    ]
}
                    )"},

{"string.json",
R"(["a1"]
["b2"])"},

{"int8.json",
R"([11]
[0]
[-1]
[-100]
[1])"},

{"int16.json",
R"([-32760]
[3276]
[-1]
[-3276]
[32760])"},

{"int32.json",
R"([-2147483640]
[214748364]
[-1]
[-214748364]
[2147483640])"},

{"int64.json",
R"([-9223372036854775800]
[92233720368547758]
[-1]
[-92233720368547758]
[9223372036854775800])"},

{"date.json",
R"(["2020-01-20"]
["2021-02-20"]
["2022-03-20"]
["2023-01-20"])"},

{"datetime.json",
R"(["2020-01-20 01:59:59"]
["2021-02-20 02:59:59"]
["2022-03-20 03:59:59"]
["2023-01-20 05:59:59"])"},

{"bool.json",
R"([false]
[true])"},

{"float.json",
R"([-3.40282e+30]
[3.40282e+20]
[-1]
[-3.40282e+20]
[3.40282e+30])"},

{"double.json",
R"([-1.79769e+300]
[1.79769e+200]
[-1]
[-1.79769e+200]
[1.79769e+300])"}},
            config, 42, 0, nullptr, false);
    }
}

TEST_F(TestImportV2, dirtyData) {
    {
        UT_LOG() << "Test dirty data";
        std::vector<std::pair<std::string, std::string>> data = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "addr", "type":"STRING", "index": true},
                {"name" : "name", "type":"STRING", "index": true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "score", "type" : "INT32", "index": true},
                {"name" : "weight", "type" : "FLOAT", "index": true}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name", "addr"]
        },
        {
            "path" : "edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID", "weight", "score"]
        }
    ]
}
                    )"},
            {"node1.csv",
             R"([1,"name1","beijing"]
[2,"name2","beijing"]
[3,"name3","shanghai"]
[3,"name33","shanghai"]
[4,"name4","hangzhou"]
[11,"name4","hangzhou"]
[5,"name5","shenzhen"]
[6,"name6","beijing"]
[7,"name7","beijing"]
[8,"name8","shanghai"]
[9,"name9","beijing"]
[10,"name10","hangzhou"])"},
            {"edge1.csv", R"([1,2,1.1,10]
[2,3,2.1,1]
[3,4,3.1,2]
[4,5,4.1,3]
[5,6,5.1,4]
[5,6,5.1,44]
[6,7,6.1,5]
[7,8,7.1,6]
[11,8,7.1,6]
[9,10,8.1,7]
[10,1,9.1,8]
[1,2,10.1,8])"}};
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        TestImportOnData(data, config, 11, 12);
    }
}

TEST_F(TestImportV2, emptySST) {
    {
        UT_LOG() << "Test empty sst";
        std::vector<std::pair<std::string, std::string>> data = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "addr", "type":"STRING", "index": true},
                {"name" : "name", "type":"STRING", "index": true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "score", "type" : "INT32", "index": true},
                {"name" : "weight", "type" : "FLOAT", "index": true}
            ]
        }
    ],
    "files" : [
        {
            "path" : "node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name", "addr"]
        },
        {
            "path" : "node_invalid.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name", "addr"]
        },
        {
            "path" : "edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID", "weight", "score"]
        },
        {
            "path" : "edge_invalid.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID", "weight", "score"]
        }
    ]
}
                    )"},
            {"node1.csv",
             R"([1,"name1","beijing"]
[2,"name2","beijing"]
[3,"name3","shanghai"]
[3,"name33","shanghai"]
[4,"name4","hangzhou"]
[11,"name4","hangzhou"]
[5,"name5","shenzhen"]
[6,"name6","beijing"]
[7,"name7","beijing"]
[8,"name8","shanghai"]
[9,"name9","beijing"]
[10,"name10","hangzhou"])"},
            {"node_invalid.csv",
             R"([1,"name1","beijing"]
[2,"name2","beijing"]
[3,"name3","shanghai"]
[3,"name33","shanghai"]
[4,"name4","hangzhou"]
[11,"name4","hangzhou"]
[5,"name5","shenzhen"]
[6,"name6","beijing"]
[7,"name7","beijing"]
[8,"name8","shanghai"]
[9,"name9","beijing"]
[10,"name10","hangzhou"])"},
            {"edge1.csv", R"([1,2,1.1,10]
[2,3,2.1,1]
[3,4,3.1,2]
[4,5,4.1,3]
[5,6,5.1,4]
[5,6,5.1,44]
[6,7,6.1,5]
[7,8,7.1,6]
[11,8,7.1,6]
[9,10,8.1,7]
[10,1,9.1,8]
[1,2,10.1,8])"},
            {"edge_invalid.csv", R"([100,2,110.1,10]
[200,3,20.1,1]
[3,400,30.1,2]
[400,5,40.1,3]
[500,6,50.1,4]
[500,6,50.1,44]
[600,7,60.1,5]
[7,800,70.1,6]
[110,8,70.1,6]
[900,10,80.1,7]
[100,1,90.1,8]
[100,2,110.1,8])"}
        };
        Importer::Config config;
        config.delete_if_exists = true;
        config.continue_on_error = true;
        TestImportOnData(data, config, 11, 12);
    }
}

TEST_F(TestImportV2Consistent, DataConsistent) {
    Importer::Config config;
    config.delete_if_exists = true;
    config.continue_on_error = true;
    {
        UT_LOG() << "Test vertex primary";
        std::vector<std::pair<std::string, std::string>> data_files = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "user",
            "type" : "VERTEX",
            "primary" : "uid",
            "properties" : [
                {"name":"uid", "type":"STRING"},
                {"name":"name", "type":"STRING", "index" : true, "unique" : false},
                {"name":"phone", "type":"STRING", "index" : true, "unique" : false}
            ]
        }
    ],
    "files" : [
        {
            "path" : "user.csv",
            "format" : "CSV",
            "label" : "user",
            "columns" : ["uid", "name", "phone"]
        }
    ]
}
                    )"},
            {"user.csv",
             R"(user0001,aaa,1111
            user0001,bbb,2222
            user0001,ccc,3333
            user0002,ddd,4444
            user0002,eee,5555
            user0001,fff,6666
            user0002,ggg,7777
            user0003,hhh,8888
            user0003,lll,9999
            user0004,mmm,0000)"}};

        auto galaxy = GetImportDb(data_files, config);
        lgraph_api::GraphDB db = galaxy->OpenGraph("default");
        UT_EXPECT_EQ(GetVertexNum(db), 4);
        UT_EXPECT_TRUE(HasVertexIndex(db, "user", "uid", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "user", "uid"), 4);
        UT_EXPECT_FALSE(HasVertexIndex(db, "user", "name", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_TRUE(HasVertexIndex(db, "user", "phone", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "user", "phone"), 4);
    }

    {
        UT_LOG() << "Test vertex index";
        std::vector<std::pair<std::string, std::string>> data_files = {
            {"import.conf", R"(
{
    "schema": [
        {
            "label" : "user",
            "type" : "VERTEX",
            "primary" : "uid",
            "properties" : [
                {"name":"uid", "type":"STRING"},
                {"name":"name", "type":"STRING", "index" : true, "unique" : false},
                {"name":"phone", "type":"STRING", "index" : true, "unique" : false}
            ]
        }
    ],
    "files" : [
        {
            "path" : "user.csv",
            "format" : "CSV",
            "label" : "user",
            "columns" : ["uid", "name", "phone"]
        }
    ]
}
                    )"},
            {"user.csv",
             R"(user0001,aaa,1111
            user0002,bbb,2222
            user0003,ccc,3333
            user0004,ddd,4444
            user0005,eee,5555
            user0006,fff,6666
            user0007,ggg,7777
            user0008,hhh,8888
            user0009,lll,9999
            user0010,mmm,0000
            user0011,bbb,2222
            user0012,ccc,3333
            user0013,ddd,4444
            user0014,eee,5555
            user0015,fff,6666
            user0016,ggg,7777
            user0017,hhh,8888
            user0018,lll,9999
            user0019,bbb,2222
            user0020,ccc,3333
            user0021,ddd,4444
            user0022,eee,5555
            user0023,fff,6666
            user0024,ggg,7777
            user0025,hhh,8888
            user0026,lll,9999)"}};

        auto galaxy = GetImportDb(data_files, config);
        lgraph_api::GraphDB db = galaxy->OpenGraph("default");
        UT_EXPECT_EQ(GetVertexNum(db), 26);
        UT_EXPECT_TRUE(HasVertexIndex(db, "user", "uid", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "user", "uid"), 26);
        UT_EXPECT_FALSE(HasVertexIndex(db, "user", "name", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_TRUE(HasVertexIndex(db, "user", "phone", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "user", "phone"), 26);
    }

    {
        UT_LOG() << "Test edge unique index";
        std::vector<std::pair<std::string, std::string>> data_files = {
            {"import.conf", R"(
{
	"schema": [{
			"label": "node1",
			"type": "VERTEX",
			"primary": "uid",
			"properties": [{
					"name": "uid",
					"type": "STRING"
				},
				{
					"name": "name",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "phone",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "id",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "address",
					"type": "STRING",
					"index": true,
					"unique": false
				}
			]
		}, {
			"label": "node2",
			"type": "VERTEX",
			"primary": "uid",
			"properties": [{
					"name": "uid",
					"type": "STRING"
				},
				{
					"name": "name",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "phone",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "id",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "address",
					"type": "STRING",
					"index": true,
					"unique": false
				}
			]
		},
		{
			"label": "edge",
			"type": "EDGE",
            "temporal" : "uid",
			"properties": [{
					"name": "uid",
					"type": "INT64"
				},
				{
					"name": "name_g",
					"type": "STRING",
					"index": true,
					"unique": false
				},
                {
                    "name": "address_g",
                    "type": "STRING",
                    "index": true,
                    "unique": false
                },
                {
                    "name": "name",
                    "type": "STRING",
                    "index": true,
                    "unique": false
                },
				{
					"name": "address",
					"type": "STRING",
					"index": true,
					"unique": false
				},
				{
					"name": "phone_g",
					"type": "STRING",
					"index": true,
					"unique": false
				},
                {
                    "name": "id_g",
                    "type": "STRING",
                    "index": true,
                    "unique": false
                },
                {
                    "name": "phone",
                    "type": "STRING",
                    "index": true,
                    "pair_unique": true
                },
                {
                    "name": "id",
                    "type": "STRING",
                    "index": true,
                    "pair_unique": false
                }
			]
		}
	],
	"files": [{
			"path": "node1.csv",
			"format": "CSV",
			"label": "node1",
			"columns": ["uid", "name", "phone", "id", "address"]
		},
		{
			"path": "node2.csv",
			"format": "CSV",
			"label": "node2",
			"columns": ["uid", "name", "phone", "id", "address"]
		},
		{
			"path": "edge.csv",
			"format": "CSV",
			"label": "edge",
			"SRC_ID": "node1",
			"DST_ID": "node2",
			"columns": ["SRC_ID", "DST_ID", "uid", "name_g",  "address_g", "name", "address", "phone_g", "id_g", "phone", "id"]
		}
	]
}
                    )"},
            {"node1.csv",
             R"(node0001,node1_a,00,111,111
                    node0003,node1_b,01,111,222
                    node0005,node1_c,02,222,333
                    node0007,node1_d,03,333,444
                    node0009,node1_e,04,444,555
                    node0011,node1_f,05,555,666
                    node0013,node1_g,06,666,777
                    node0015,node1_h,07,777,888
                    node0017,node1_i,08,888,999
                    node0019,node1_g,09,999,000)"},
            {"node2.csv",
             R"(node0002,node2_a,00,111,111
                    node0004,node2_b,01,111,222
                    node0006,node2_c,02,222,333
                    node0008,node2_d,03,333,444
                    node0010,node2_e,04,444,555
                    node0012,node2_f,05,555,666
                    node0014,node2_g,06,666,777
                    node0016,node2_h,07,777,888
                    node0018,node2_i,08,888,999
                    node0020,node2_g,09,999,000)"},
            {"edge.csv",
             R"(node0001,node0002,1,00,00,00,00,00,00,100,200
                    node0003,node0004,2,01,01,01,01,01,01,101,201
                    node0005,node0006,3,02,02,02,02,02,02,102,202
                    node0007,node0008,4,03,00,03,00,03,00,103,203
                    node0009,node0010,5,04,01,04,01,04,01,104,204
                    node0011,node0012,6,05,02,05,02,05,02,105,205
                    node0013,node0014,7,06,00,06,00,06,00,106,206
                    node0015,node0016,8,07,01,07,01,07,01,107,207
                    node0017,node0018,9,08,02,08,02,08,02,108,208
                    node0019,node0020,10,09,00,09,00,09,00,109,209
                    node0001,node0002,11,00,01,00,01,10,01,109,200
                    node0003,node0004,12,01,02,01,02,11,02,108,201
                    node0005,node0006,13,02,00,02,00,12,00,107,202
                    node0007,node0008,14,03,01,03,01,13,01,106,203
                    node0009,node0010,15,04,02,04,02,14,02,105,204
                    node0011,node0012,16,05,00,05,00,15,00,104,205
                    node0013,node0014,17,06,01,06,01,16,01,103,206
                    node0015,node0016,18,07,02,07,02,17,02,102,207
                    node0017,node0018,19,08,00,08,00,18,00,101,208
                    node0019,node0020,20,09,01,09,01,19,01,100,209)"},
        };

        auto galaxy = GetImportDb(data_files, config);
        lgraph_api::GraphDB db = galaxy->OpenGraph("default");
        UT_EXPECT_EQ(GetVertexNum(db), 20);
        UT_EXPECT_EQ(GetEdgeNum(db), 20);
        UT_EXPECT_TRUE(HasVertexIndex(db, "node1", "uid", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "node1", "uid"), 10);
        UT_EXPECT_TRUE(HasVertexIndex(db, "node1", "name", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "node1", "name"), 10);
        UT_EXPECT_FALSE(HasVertexIndex(db, "node1", "phone", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_FALSE(HasVertexIndex(db, "node1", "id", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_TRUE(HasVertexIndex(db, "node1", "address", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "node1", "address"), 10);

        UT_EXPECT_TRUE(HasVertexIndex(db, "node2", "uid", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "node2", "uid"), 10);
        UT_EXPECT_TRUE(HasVertexIndex(db, "node2", "name", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "node2", "name"), 10);
        UT_EXPECT_FALSE(HasVertexIndex(db, "node2", "phone", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_FALSE(HasVertexIndex(db, "node2", "id", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_TRUE(HasVertexIndex(db, "node2", "address", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetVertexIndexValueNum(db, "node2", "address"), 10);

        UT_EXPECT_TRUE(HasEdgeIndex(db, "edge", "name_g", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "edge", "name_g"), 20);
        UT_EXPECT_TRUE(HasEdgeIndex(db, "edge", "address_g", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "edge", "address_g"), 20);
        UT_EXPECT_TRUE(HasEdgeIndex(db, "edge", "name", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "edge", "name"), 20);
        UT_EXPECT_TRUE(HasEdgeIndex(db, "edge", "address", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "edge", "address"), 20);

        UT_EXPECT_FALSE(HasEdgeIndex(db, "edge", "phone_g", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_FALSE(HasEdgeIndex(db, "edge", "id_g", lgraph::IndexType::GlobalUniqueIndex));
        UT_EXPECT_TRUE(HasEdgeIndex(db, "edge", "phone", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "edge", "phone"), 20);
        UT_EXPECT_FALSE(HasEdgeIndex(db, "edge", "id", lgraph::IndexType::PairUniqueIndex));
        UT_EXPECT_TRUE(HasEdgeIndex(db, "edge", "id", lgraph::IndexType::NonuniqueIndex));
        UT_EXPECT_EQ(GetEdgeIndexValueNum(db, "edge", "id"), 20);
    }
}
