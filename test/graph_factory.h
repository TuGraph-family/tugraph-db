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

#pragma once
#include <map>

#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "./ut_utils.h"
#include "gtest/gtest.h"
#include "core/lightning_graph.h"
#include "import/import_v3.h"

#include "lgraph/lgraph.h"

class GraphFactory {
 public:
    static void create_snapshot(const std::string& dir = "./testdb") {
        fma_common::FileSystem::GetFileSystem(dir).RemoveDir(dir);
        lgraph_api::Galaxy galaxy(dir, lgraph::_detail::DEFAULT_ADMIN_NAME,
                                  lgraph::_detail::DEFAULT_ADMIN_PASS, false, true);
        lgraph_api::GraphDB db = galaxy.OpenGraph("default");
        // add schemas
        std::string v_label_1("person");
        std::vector<lgraph::FieldSpec> v_fds_1 = {
            {"uid", lgraph::FieldType::INT64, false},  {"name", lgraph::FieldType::STRING, false},
            {"age", lgraph::FieldType::INT8, false},   {"age16", lgraph::FieldType::INT16, true},
            {"age32", lgraph::FieldType::INT32, true}, {"agef", lgraph::FieldType::FLOAT, true},
            {"aged", lgraph::FieldType::DOUBLE, true}};
        std::string v_label_2("software");
        std::vector<lgraph::FieldSpec> v_fds_2 = {{"uid", lgraph::FieldType::INT64, false},
                                                  {"name", lgraph::FieldType::STRING, false},
                                                  {"lang", lgraph::FieldType::STRING, false}};
        std::string e_label_1("knows");
        std::vector<lgraph::FieldSpec> e_fds_1 = {{"weight", lgraph::FieldType::FLOAT, false},
                                                  {"since", lgraph::FieldType::INT32, true}};
        std::string e_label_2("created");
        std::vector<lgraph::FieldSpec> e_fds_2 = {{"weight", lgraph::FieldType::FLOAT, false}};
        UT_ASSERT(db.AddVertexLabel(v_label_1, v_fds_1, "uid"));
        UT_ASSERT(db.AddVertexLabel(v_label_2, v_fds_2, "uid"));
        UT_ASSERT(db.AddEdgeLabel(e_label_1, e_fds_1, {}));
        UT_ASSERT(db.AddEdgeLabel(e_label_2, e_fds_2, {}));

        auto txn = db.CreateWriteTxn();
        // add vertex
        lgraph::VertexId vid[6];
        vid[0] = txn.AddVertex(
            v_label_1,
            std::vector<std::string>{"uid", "name", "age", "age16", "age32", "agef", "aged"},
            std::vector<std::string>{"1", "marko", "29", "23", "22", "5.0", "6.0"});
        vid[1] = txn.AddVertex(
            v_label_1,
            std::vector<std::string>{"uid", "name", "age", "age16", "age32", "agef", "aged"},
            std::vector<std::string>{"2", "vadas", "27", "23", "34", "23", "232.9"});
        vid[2] = txn.AddVertex(v_label_2, std::vector<std::string>{"uid", "name", "lang"},
                               std::vector<std::string>{"3", "lop", "java"});
        vid[3] = txn.AddVertex(v_label_1, std::vector<std::string>{"uid", "name", "age"},
                               std::vector<std::string>{"4", "josh", "32"});
        vid[4] = txn.AddVertex(v_label_2, std::vector<std::string>{"uid", "name", "lang"},
                               std::vector<std::string>{"5", "ripple", "java"});
        vid[5] = txn.AddVertex(v_label_1, std::vector<std::string>{"uid", "name", "age"},
                               std::vector<std::string>{"6", "peter", "35"});
        // add edge
        txn.Commit();
    }

    static void create_modern(const std::string& dir = "./testdb") {
        fma_common::FileSystem::GetFileSystem(dir).RemoveDir(dir);
        lgraph_api::Galaxy galaxy(dir, lgraph::_detail::DEFAULT_ADMIN_NAME,
                                  lgraph::_detail::DEFAULT_ADMIN_PASS, false, true);
        lgraph_api::GraphDB db = galaxy.OpenGraph("default");
        // add schemas
        std::string v_label_1("person");
        std::vector<lgraph::FieldSpec> v_fds_1 = {{"uid", lgraph::FieldType::INT64, false},
                                                  {"name", lgraph::FieldType::STRING, false},
                                                  {"age", lgraph::FieldType::INT8, false}};
        std::string v_label_2("software");
        std::vector<lgraph::FieldSpec> v_fds_2 = {{"uid", lgraph::FieldType::INT64, false},
                                                  {"name", lgraph::FieldType::STRING, false},
                                                  {"lang", lgraph::FieldType::STRING, false}};
        std::string e_label_1("knows");
        std::vector<lgraph::FieldSpec> e_fds_1 = {{"weight", lgraph::FieldType::FLOAT, false},
                                                  {"since", lgraph::FieldType::INT32, true}};
        std::string e_label_2("created");
        std::vector<lgraph::FieldSpec> e_fds_2 = {{"weight", lgraph::FieldType::FLOAT, false}};
        UT_ASSERT(db.AddVertexLabel(v_label_1, v_fds_1, "uid"));
        UT_ASSERT(db.AddVertexLabel(v_label_2, v_fds_2, "uid"));
        UT_ASSERT(db.AddEdgeLabel(e_label_1, e_fds_1, {}));
        UT_ASSERT(db.AddEdgeLabel(e_label_2, e_fds_2, {}));

        auto txn = db.CreateWriteTxn();
        // add vertex
        lgraph::VertexId vid[6];
        vid[0] = txn.AddVertex(v_label_1, std::vector<std::string>{"uid", "name", "age"},
                               std::vector<std::string>{"1", "marko", "29"});
        vid[1] = txn.AddVertex(v_label_1, std::vector<std::string>{"uid", "name", "age"},
                               std::vector<std::string>{"2", "vadas", "27"});
        vid[2] = txn.AddVertex(v_label_2, std::vector<std::string>{"uid", "name", "lang"},
                               std::vector<std::string>{"3", "lop", "java"});
        vid[3] = txn.AddVertex(v_label_1, std::vector<std::string>{"uid", "name", "age"},
                               std::vector<std::string>{"4", "josh", "32"});
        vid[4] = txn.AddVertex(v_label_2, std::vector<std::string>{"uid", "name", "lang"},
                               std::vector<std::string>{"5", "ripple", "java"});
        vid[5] = txn.AddVertex(v_label_1, std::vector<std::string>{"uid", "name", "age"},
                               std::vector<std::string>{"6", "peter", "35"});
        // add edge
        // lgraph::EdgeId eid[6];
        txn.AddEdge(vid[0], vid[1], e_label_1, std::vector<std::string>{"weight", "since"},
                    std::vector<std::string>{"0.5", "2018"});
        txn.AddEdge(vid[0], vid[3], e_label_1, std::vector<std::string>{"weight", "since"},
                    std::vector<std::string>{"1.0", "1998"});
        txn.AddEdge(vid[0], vid[2], e_label_2, std::vector<std::string>{"weight"},
                    std::vector<std::string>{"0.4"});
        txn.AddEdge(vid[3], vid[4], e_label_2, std::vector<std::string>{"weight"},
                    std::vector<std::string>{"1.0"});
        txn.AddEdge(vid[3], vid[2], e_label_2, std::vector<std::string>{"weight"},
                    std::vector<std::string>{"0.4"});
        txn.AddEdge(vid[5], vid[2], e_label_2, std::vector<std::string>{"weight"},
                    std::vector<std::string>{"0.2"});
        txn.Commit();
    }

    static void CreateCsvFiles(const std::map<std::string, std::string>& data) {
        fma_common::OutputFmaStream stream;
        for (auto& kv : data) {
            const std::string& file_name = kv.first;
            const std::string& data = kv.second;
            stream.Open(file_name);
            stream.Write(data.data(), data.size());
            stream.Close();
            UT_LOG() << file_name << " created";
        }
    }

    static void WriteYagoFiles() {
        static const std::map<std::string, std::string> data = {
            {"yago.conf", R"(
{
    "schema": [
        {
            "label" : "Person",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name" : "name", "type":"STRING"},
                {"name" : "birthyear", "type":"INT16", "optional":true}
            ]
        },
        {
            "label" : "City",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name": "name", "type":"STRING"}
            ]
        },
        {
            "label" : "Film",
            "primary": "title",
            "type" : "VERTEX",
            "properties" : [
                {"name": "title", "type":"STRING"}
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
            "columns" : ["name","birthyear"]
        },
        {
            "path" : "city.csv",
            "format" : "CSV",
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
             R"(Rachel Kempson,1910
Michael Redgrave,1908
Vanessa Redgrave,1937
Corin Redgrave,1939
Liam Neeson,1952
Natasha Richardson,1963
Richard Harris,1930
Dennis Quaid,1954
Lindsay Lohan,1986
Jemma Redgrave,1965
Roy Redgrave,1873
John Williams,1932
Christopher Nolan,1970
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

        CreateCsvFiles(data);
    }

    /**
     * The openCypher YAGO graph, or oc-yago-graph, is distributed as a standalone
     * openCypher artifact and as a built-in graph in the openCypher TCK.
     * see https://github.com/opencypher/openCypher/tree/master/tck/graphs/yago
     */
    static void create_yago(const std::string& dir = "./lgraph_db") {
        using namespace lgraph;
        WriteYagoFiles();
        import_v3::Importer::Config config;
        config.config_file = "./yago.conf";  // the config file specifying which files to import
        config.db_dir = dir;                 // db data dir to use
        config.delete_if_exists = true;
        config.graph = "default";

        config.parse_block_threads = 1;
        config.parse_file_threads = 1;
        config.generate_sst_threads = 1;

        import_v3::Importer importer(config);
        importer.DoImportOffline();
    }
};
