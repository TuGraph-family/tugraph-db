
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

#include "fma-common/fma_stream.h"
#include "gtest/gtest.h"
#include "./test_tools.h"
#include "lgraph/lgraph_rpc_client.h"

class TestUpsert : public TuGraphTest {};

TEST_F(TestUpsert, upsert) {
    lgraph::GlobalConfig conf;
    conf.db_dir = "./testdb";
    conf.http_port = 7774;
    conf.enable_rpc = true;
    conf.use_pthread = true;
    conf.rpc_port = 19999;
    conf.bind_host = "127.0.0.1";
    lgraph::AutoCleanDir cleaner(conf.db_dir);
    auto server = StartLGraphServer(conf);

    lgraph::RpcClient client("127.0.0.1:19999", "admin", "73@TuGraph");
    std::string str;

    bool ret = client.CallCypher(str, "CALL db.createVertexLabel('node1', 'id' , "
                            "'id' ,'INT32', false, "
                            "'name' ,'STRING', false, "
                            "'num', 'INT32', false, 'desc', 'STRING', true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.createVertexLabel('node2', 'id' , "
                            "'id' ,'INT32', false, "
                            "'name' ,'STRING', false, "
                            "'num', 'INT32', false, 'desc', 'STRING', true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            R"(CALL db.createEdgeLabel('edge1','[["node1","node2"]]',
                'id' ,'INT32', false,
                'name' ,'STRING', false,
                'num', 'INT32', false, 'desc', 'STRING', true))");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.addIndex('node1', 'name', false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addIndex('node1', 'num', true)");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.addIndex('node2', 'name', false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addIndex('node2', 'num', true)");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.addEdgeIndex('edge1', 'name', false, false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addEdgeIndex('edge1', 'num', true, false)");
    UT_EXPECT_TRUE(ret);

    // node1
    for (int i = 1; i <= 10; i++) {
        auto prop = FMA_FMT("id:{}, name:'{}', num:{}", i, "name" + std::to_string(i), i);
        prop = "{ " + prop + " }";
        ret = client.CallCypher(str, FMA_FMT("create (n:node1 {} )", prop));
        UT_EXPECT_TRUE(ret);
    }
    nlohmann::json array = nlohmann::json::array();
    for (int i = 1; i <= 20; i++) {
        nlohmann::json prop;
        prop["id"] = i;
        prop["not_exist"] = i;
        prop["name"] = "new name " + std::to_string(i);
        if (i == 15) {
            prop["num"] = 5;  // unique index conflict
        } else if (i == 17) {
            prop["num"] = 7;  // unique index conflict
        } else {
            prop["num"] = i;
        }
        array.push_back(prop);
    }
    std::string cypher = FMA_FMT("CALL db.upsertVertexByJson('node1', '{}')", array.dump());
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"([{"data_error":0,"index_conflict":2,"insert":8,"total":20,"update":10}])");

    // node2
    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = "desc " + std::to_string(i);
            array.push_back(prop);
        }
        cypher = FMA_FMT("CALL db.upsertVertexByJson('node2', '{}')", array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":0,"index_conflict":0,"insert":20,"total":20,"update":0}])");
    }

    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = nullptr;
            array.push_back(prop);
        }
        cypher = FMA_FMT("CALL db.upsertVertexByJson('node2', '{}')", array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":0,"index_conflict":0,"insert":0,"total":20,"update":20}])");
    }
    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = nullptr;
            array.push_back(prop);
        }
        cypher = FMA_FMT("CALL db.upsertVertexByJson('node2', '{}')", array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":20,"index_conflict":0,"insert":0,"total":20,"update":0}])");
    }

    // edge1
    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["node1_id"] = i;
            prop["node2_id"] = i;

            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = "desc " + std::to_string(i);
            array.push_back(prop);
        }
        nlohmann::json start;
        start["type"] = "node1";
        start["key"] = "node1_id";
        nlohmann::json end;
        end["type"] = "node2";
        end["key"] = "node2_id";
        cypher = FMA_FMT("CALL db.upsertEdgeByJson('edge1','{}', '{}', '{}')", start.dump(),
                         end.dump(), array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":2,"index_conflict":0,"insert":18,"total":20,"update":0}])");
    }

    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["node1_id"] = i;
            prop["node2_id"] = i;

            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = nullptr;
            array.push_back(prop);
        }
        nlohmann::json start;
        start["type"] = "node1";
        start["key"] = "node1_id";
        nlohmann::json end;
        end["type"] = "node2";
        end["key"] = "node2_id";
        cypher = FMA_FMT("CALL db.upsertEdgeByJson('edge1','{}', '{}', '{}')", start.dump(),
                         end.dump(), array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":2,"index_conflict":0,"insert":0,"total":20,"update":18}])");
    }

    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["node1_id"] = i;
            prop["node2_id"] = i;

            prop["id"] = i;
            prop["name"] = "new name" + std::to_string(i);
            array.push_back(prop);
        }
        nlohmann::json start;
        start["type"] = "node1";
        start["key"] = "node1_id";
        nlohmann::json end;
        end["type"] = "node2";
        end["key"] = "node2_id";
        cypher = FMA_FMT("CALL db.upsertEdgeByJson('edge1','{}', '{}', '{}')", start.dump(),
                         end.dump(), array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":2,"index_conflict":0,"insert":0,"total":20,"update":18}])");
    }
    cypher = "match(n:node1) return count(n)";
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"#([{"count(n)":18}])#");
    cypher = "match(n:node2) return count(n)";
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"#([{"count(n)":20}])#");

    cypher = "match(n)-[r]->(m) return count(r)";
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"#([{"count(r)":18}])#");
}

TEST_F(TestUpsert, addLabelWithJson) {
    lgraph::GlobalConfig conf;
    conf.db_dir = "./testdb";
    conf.http_port = 7774;
    conf.enable_rpc = true;
    conf.use_pthread = true;
    conf.rpc_port = 19999;
    conf.bind_host = "127.0.0.1";
    lgraph::AutoCleanDir cleaner(conf.db_dir);
    auto server = StartLGraphServer(conf);

    lgraph::RpcClient client("127.0.0.1:19999", "admin", "73@TuGraph");
    std::string str;

    std::string node1_schema = R"!({
"label": "node1",
"primary": "id",
"type": "VERTEX",
"detach_property": true,
"properties": [{
  "name": "id",
  "type": "INT32",
  "optional":false
}, {
  "name": "name",
  "type": "STRING",
  "optional": false,
  "index":true
},{
  "name": "num",
  "type": "INT32",
  "optional": false,
  "index":true,
  "unique":true
}, {
  "name": "desc",
  "type": "STRING",
  "optional": true
}]})!";

    std::string node2_schema = R"!({
"label": "node2",
"primary": "id",
"type": "VERTEX",
"detach_property": true,
"properties": [{
  "name": "id",
  "type": "INT32",
  "optional":false
}, {
  "name": "name",
  "type": "STRING",
  "optional": false,
  "index":true
},{
  "name": "num",
  "type": "INT32",
  "optional": false,
  "index":true,
  "unique":true
}, {
  "name": "desc",
  "type": "STRING",
  "optional": true
}]})!";

    std::string edge1_schema = R"!({
"label": "edge1",
"type": "EDGE",
"detach_property": true,
"properties": [{
  "name": "id",
  "type": "INT32",
  "optional":false
}, {
  "name": "name",
  "type": "STRING",
  "optional": false,
  "index":true
},{
  "name": "num",
  "type": "INT32",
  "optional": false,
  "index":true,
  "unique":true
}, {
  "name": "desc",
  "type": "STRING",
  "optional": true
}]})!";

    auto ret = client.CallCypher(str,
                                 FMA_FMT("CALL db.createVertexLabelByJson('{}')", node1_schema));
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            FMA_FMT("CALL db.createVertexLabelByJson('{}')", node2_schema));
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            FMA_FMT("CALL db.createEdgeLabelByJson('{}')", edge1_schema));
    UT_EXPECT_TRUE(ret);

    // node1
    for (int i = 1; i <= 10; i++) {
        auto prop = FMA_FMT("id:{}, name:'{}', num:{}", i, "name" + std::to_string(i), i);
        prop = "{ " + prop + " }";
        ret = client.CallCypher(str, FMA_FMT("create (n:node1 {} )", prop));
        UT_EXPECT_TRUE(ret);
    }
    nlohmann::json array = nlohmann::json::array();
    for (int i = 1; i <= 20; i++) {
        nlohmann::json prop;
        prop["id"] = i;
        prop["not_exist"] = i;
        prop["name"] = "new name " + std::to_string(i);
        if (i == 15) {
            prop["num"] = 5;  // unique index conflict
        } else if (i == 17) {
            prop["num"] = 7;  // unique index conflict
        } else {
            prop["num"] = i;
        }
        array.push_back(prop);
    }
    std::string cypher = FMA_FMT("CALL db.upsertVertexByJson('node1', '{}')", array.dump());
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"([{"data_error":0,"index_conflict":2,"insert":8,"total":20,"update":10}])");

    // node2
    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = "desc " + std::to_string(i);
            array.push_back(prop);
        }
        cypher = FMA_FMT("CALL db.upsertVertexByJson('node2', '{}')", array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":0,"index_conflict":0,"insert":20,"total":20,"update":0}])");
    }

    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = nullptr;
            array.push_back(prop);
        }
        cypher = FMA_FMT("CALL db.upsertVertexByJson('node2', '{}')", array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":0,"index_conflict":0,"insert":0,"total":20,"update":20}])");
    }
    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = nullptr;
            array.push_back(prop);
        }
        cypher = FMA_FMT("CALL db.upsertVertexByJson('node2', '{}')", array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":20,"index_conflict":0,"insert":0,"total":20,"update":0}])");
    }

    // edge1
    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["node1_id"] = i;
            prop["node2_id"] = i;

            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = "desc " + std::to_string(i);
            array.push_back(prop);
        }
        nlohmann::json start;
        start["type"] = "node1";
        start["key"] = "node1_id";
        nlohmann::json end;
        end["type"] = "node2";
        end["key"] = "node2_id";
        cypher = FMA_FMT("CALL db.upsertEdgeByJson('edge1','{}', '{}', '{}')", start.dump(),
                         end.dump(), array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":2,"index_conflict":0,"insert":18,"total":20,"update":0}])");
    }

    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["node1_id"] = i;
            prop["node2_id"] = i;

            prop["id"] = i;
            prop["name"] = "name" + std::to_string(i);
            prop["num"] = i;
            prop["desc"] = nullptr;
            array.push_back(prop);
        }
        nlohmann::json start;
        start["type"] = "node1";
        start["key"] = "node1_id";
        nlohmann::json end;
        end["type"] = "node2";
        end["key"] = "node2_id";
        cypher = FMA_FMT("CALL db.upsertEdgeByJson('edge1','{}', '{}', '{}')", start.dump(),
                         end.dump(), array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":2,"index_conflict":0,"insert":0,"total":20,"update":18}])");
    }

    {
        array.clear();
        for (int i = 1; i <= 20; i++) {
            nlohmann::json prop;
            prop["node1_id"] = i;
            prop["node2_id"] = i;

            prop["id"] = i;
            prop["name"] = "new name" + std::to_string(i);
            array.push_back(prop);
        }
        nlohmann::json start;
        start["type"] = "node1";
        start["key"] = "node1_id";
        nlohmann::json end;
        end["type"] = "node2";
        end["key"] = "node2_id";
        cypher = FMA_FMT("CALL db.upsertEdgeByJson('edge1','{}', '{}', '{}')", start.dump(),
                         end.dump(), array.dump());
        ret = client.CallCypher(str, cypher);
        UT_EXPECT_TRUE(ret);
        UT_EXPECT_EQ(str, R"([{"data_error":2,"index_conflict":0,"insert":0,"total":20,"update":18}])");
    }
    cypher = "match(n:node1) return count(n)";
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"#([{"count(n)":18}])#");
    cypher = "match(n:node2) return count(n)";
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"#([{"count(n)":20}])#");

    cypher = "match(n)-[r]->(m) return count(r)";
    ret = client.CallCypher(str, cypher);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(str, R"#([{"count(r)":18}])#");
}
