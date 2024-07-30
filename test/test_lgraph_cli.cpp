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
#include "fma-common/fma_stream.h"
#include "gtest/gtest.h"
#include "./test_tools.h"

class TestLGraphCLI : public TuGraphTest {};


TEST_F(TestLGraphCLI, LGraphCLI) {
    using namespace lgraph;
    std::string file = "statements.txt";
    std::string statements = R"(
CALL db.createVertexLabel('person', 'int8',
'bool' ,'BOOL', false,
'int8' ,'INT8', false,
'int16' ,'INT16', false,
'int32' ,'INT32', false,
'int64' ,'INT64', false,
'float' ,'FLOAT', false,
'double' ,'DOUBLE', false,
'date' ,'DATE', false,
'datetime' ,'DATETIME', false,
'string' ,'STRING', false);

CALL db.createEdgeLabel('is_friend', '[["person","person"]]',
'message', 'STRING', false);

create
(n1:person{
    bool:true,
    int8:8,
    int16:16,
    int32:32,
    int64:64,
    float:1.11,
    double:100.98,
    date: '2017-05-03',
    datetime: '2017-05-01 10:00:00',
    string: 'foo bar'
}),
(n2:person{
    bool:true,
    int8:18,
    int16:116,
    int32:132,
    int64:164,
    float:11.11,
    double:1100.98,
    date: '2018-05-03',
    datetime: '2018-05-01 10:00:00',
    string: 'bar foo'
});

match (n1:person {int8:8}),
      (n2:person {int8:18})
create (n1)-[r:is_friend {message:'hi..'}]->(n2);

match (n)-[r]->(m) return n;

explain match (n)-[r]->(m) return n;

match (n)-[r]->(m) return r;

profile match (n)-[r]->(m) return r;

match p = (n)-[r]->(m) return p;
)";

    {
        lgraph::GlobalConfig conf;
        conf.db_dir = "./testdb";
        conf.http_port = 7774;
        conf.enable_rpc = false;
        conf.bolt_port = 7687;
        conf.bind_host = "127.0.0.1";
        AutoCleanDir cleaner(conf.db_dir);
        auto server = StartLGraphServer(conf);
        auto WriteFile = [](const std::string& name, const std::string& content) {
            fma_common::OutputFmaStream out(name);
            out.Write(content.data(), content.size());
        };
        std::string expected = R"(

<SUMMARY>
created 2 vertices, created 0 edges.
<SUMMARY>
created 0 vertices, created 1 edges.
n
(:person {int16:16,float:1.11,double:100.98,int8:8,string:"foo bar",int32:32,int64:64,bool:true,datetime:"2017-05-01 10:00:00",date:"2017-05-03"})
@plan
ReadOnly:1
Execution Plan:
Produce Results
    Project [n]
        Expand(All) [n --> m ]
            All Node Scan [n]

r
[:is_friend {message:"hi.."}]
@profile
Current Pattern Graph:
N[0] n: (MATCHED)
N[1] m: (MATCHED)
R[0 --> 1] r:{<0>: } (MATCHED)
Symbol: [n] type(NODE), scope(LOCAL), symbol_id(0)
Symbol: [m] type(NODE), scope(LOCAL), symbol_id(2)
Symbol: [r] type(RELATIONSHIP), scope(LOCAL), symbol_id(1)

p
(:person {int16:16,float:1.11,double:100.98,int8:8,string:"foo bar",int32:32,int64:64,bool:true,datetime:"2017-05-01 10:00:00",date:"2017-05-03"})-[:is_friend {message:"hi.."}]->(:person {int16:116,float:11.11,double:1100.98,int8:18,string:"bar foo",int32:132,int64:164,bool:true,datetime:"2018-05-01 10:00:00",date:"2018-05-03"})
)";

        WriteFile(file, statements);
        std::string lgraph_cli =
            "./lgraph_cli --ip 127.0.0.1 --port 7687 --graph default "
            "--user admin --password 73@TuGraph --format csv";
        lgraph::SubProcess cli(FMA_FMT("{} < {}", lgraph_cli, file));
        cli.Wait();
        UT_EXPECT_EQ(cli.Stdout(), expected);
        server->Kill();
        server->Wait();
    }

    {
        lgraph::GlobalConfig conf;
        conf.db_dir = "./testdb";
        conf.http_port = 7774;
        conf.enable_rpc = false;
        conf.bolt_port = 7687;
        conf.bind_host = "127.0.0.1";
        AutoCleanDir cleaner(conf.db_dir);
        auto server = StartLGraphServer(conf);
        auto WriteFile = [](const std::string& name, const std::string& content) {
            fma_common::OutputFmaStream out(name);
            out.Write(content.data(), content.size());
        };
        std::string expected = R"xx([""]
[""]
["<SUMMARY>"]
["created 2 vertices, created 0 edges."]
["<SUMMARY>"]
["created 0 vertices, created 1 edges."]
["n"]
["(:person {int16:16,float:1.11,double:100.98,int8:8,string:\"foo bar\",int32:32,int64:64,bool:true,datetime:\"2017-05-01 10:00:00\",date:\"2017-05-03\"})"]
["@plan"]
["ReadOnly:1\nExecution Plan:\nProduce Results\n    Project [n]\n        Expand(All) [n --> m ]\n            All Node Scan [n]\n"]
["r"]
["[:is_friend {message:\"hi..\"}]"]
["@profile"]
["Current Pattern Graph:\nN[0] n: (MATCHED)\nN[1] m: (MATCHED)\nR[0 --> 1] r:{<0>: } (MATCHED)\nSymbol: [n] type(NODE), scope(LOCAL), symbol_id(0)\nSymbol: [m] type(NODE), scope(LOCAL), symbol_id(2)\nSymbol: [r] type(RELATIONSHIP), scope(LOCAL), symbol_id(1)\n"]
["p"]
["(:person {int16:16,float:1.11,double:100.98,int8:8,string:\"foo bar\",int32:32,int64:64,bool:true,datetime:\"2017-05-01 10:00:00\",date:\"2017-05-03\"})-[:is_friend {message:\"hi..\"}]->(:person {int16:116,float:11.11,double:1100.98,int8:18,string:\"bar foo\",int32:132,int64:164,bool:true,datetime:\"2018-05-01 10:00:00\",date:\"2018-05-03\"})"]
)xx";

        WriteFile(file, statements);
        std::string lgraph_cli =
            "./lgraph_cli --ip 127.0.0.1 --port 7687 --graph default "
            "--user admin --password 73@TuGraph --format json";
        lgraph::SubProcess cli(FMA_FMT("{} < {}", lgraph_cli, file));
        cli.Wait();
        UT_EXPECT_EQ(cli.Stdout(), expected);
        server->Kill();
        server->Wait();
    }

    {
        lgraph::GlobalConfig conf;
        conf.db_dir = "./testdb";
        conf.http_port = 7774;
        conf.enable_rpc = false;
        conf.bolt_port = 7687;
        conf.bind_host = "127.0.0.1";
        AutoCleanDir cleaner(conf.db_dir);
        auto server = StartLGraphServer(conf);
        auto WriteFile = [](const std::string& name, const std::string& content) {
            fma_common::OutputFmaStream out(name);
            out.Write(content.data(), content.size());
        };
        std::string expected = R"xx(+--+

+--+

0 rows

+--+

+--+

0 rows

+--------------------------------------+
| <SUMMARY>                            |
+--------------------------------------+
| created 2 vertices, created 0 edges. |
+--------------------------------------+

1 rows

+--------------------------------------+
| <SUMMARY>                            |
+--------------------------------------+
| created 0 vertices, created 1 edges. |
+--------------------------------------+

1 rows

+----------------------------------------------------------------------------------------------------------------------------------------------------+
| n                                                                                                                                                  |
+----------------------------------------------------------------------------------------------------------------------------------------------------+
| (:person {int16:16,float:1.11,double:100.98,int8:8,string:"foo bar",int32:32,int64:64,bool:true,datetime:"2017-05-01 10:00:00",date:"2017-05-03"}) |
+----------------------------------------------------------------------------------------------------------------------------------------------------+

1 rows

+--------------------------------+
| @plan                          |
+--------------------------------+
| ReadOnly:1                     |
| Execution Plan:                |
| Produce Results                |
| Project [n]                    |
| Expand(All) [n --> m ]         |
| All Node Scan [n]              |
+--------------------------------+

1 rows

+-------------------------------+
| r                             |
+-------------------------------+
| [:is_friend {message:"hi.."}] |
+-------------------------------+

1 rows

+------------------------------------------------------------+
| @profile                                                   |
+------------------------------------------------------------+
| Current Pattern Graph:                                     |
| N[0] n: (MATCHED)                                          |
| N[1] m: (MATCHED)                                          |
| R[0 --> 1] r:{<0>: } (MATCHED)                             |
| Symbol: [n] type(NODE), scope(LOCAL), symbol_id(0)         |
| Symbol: [m] type(NODE), scope(LOCAL), symbol_id(2)         |
| Symbol: [r] type(RELATIONSHIP), scope(LOCAL), symbol_id(1) |
+------------------------------------------------------------+

1 rows

+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| p                                                                                                                                                                                                                                                                                                                                          |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| (:person {int16:16,float:1.11,double:100.98,int8:8,string:"foo bar",int32:32,int64:64,bool:true,datetime:"2017-05-01 10:00:00",date:"2017-05-03"})-[:is_friend {message:"hi.."}]->(:person {int16:116,float:11.11,double:1100.98,int8:18,string:"bar foo",int32:132,int64:164,bool:true,datetime:"2018-05-01 10:00:00",date:"2018-05-03"}) |
+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

1 rows

)xx";

        WriteFile(file, statements);
        std::string lgraph_cli =
            "./lgraph_cli --ip 127.0.0.1 --port 7687 --graph default "
            "--user admin --password 73@TuGraph";
        lgraph::SubProcess cli(FMA_FMT("{} < {}", lgraph_cli, file));
        cli.Wait();
        UT_EXPECT_EQ(cli.Stdout(), expected);
        server->Kill();
        server->Wait();
    }
}
