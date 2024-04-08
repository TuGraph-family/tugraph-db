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

#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <boost/lexical_cast.hpp>
#include "./ut_utils.h"
#include "fma-common/configuration.h"
#include "restful/server/stdafx.h"
#include "restful/server/rest_server.h"
#include "./graph_factory.h"
#include "fma-common/string_formatter.h"

#include "brpc/closure_guard.h"
#include "brpc/server.h"
#include "lgraph/lgraph_rpc_client.h"
#include "./test_tools.h"

using namespace utility;               // Common utilities like string conversions
using namespace concurrency::streams;  // Asynchronous streams

std::mutex lock_rpc;
std::condition_variable cond;
int stage_3 = 0;
lgraph::StateMachine::Config sm_config;
std::shared_ptr<lgraph::GlobalConfig> gconfig = std::make_shared<lgraph::GlobalConfig>();
lgraph::StateMachine* ptr_state_machine;
extern void build_so();

brpc::Server rpc_server_;
struct Config : public lgraph::StateMachine::Config, public lgraph::RestServer::Config {
    int verbose = 1;
    std::string log_file;
    bool enable_ha = false;
    bool enable_rpc = true;
    int thread_limit = 0;
};
class RPCService : public lgraph::LGraphRPCService {
 public:
    explicit RPCService(lgraph::StateMachine* sm) : sm_(sm) {}

    void HandleRequest(::google::protobuf::RpcController* controller,
                       const ::lgraph::LGraphRequest* request, ::lgraph::LGraphResponse* response,
                       ::google::protobuf::Closure* done) {
        sm_->HandleRequest(controller, request, response, done);
    }

 private:
    lgraph::StateMachine* sm_;
};
RPCService* ptr_rpc_service;

void on_initialize_rpc_server() {
    using namespace fma_common;
    using namespace lgraph;
    bool enable_ssl = true;
    std::string host = "127.0.0.1";
    uint16_t port = 6464;
    std::string db_name = "default";
    {
        Configuration config;
        config.Add(enable_ssl, "ssl", true).Comment("Enable SSL");
        config.Add(host, "host", true).Comment("Host address");
        config.Add(port, "port", true).Comment("HTTP port");
        config.Add(gconfig->enable_ip_check, "enable_ip_check", true).Comment("Enable IP check.");
    }

    { GraphFactory::create_modern(); }
    // Build listener's URI from the configured address and the hard-coded path "RestServer/Action"
    sm_config.db_dir = "./testdb";
    sm_config.rpc_port = 19099;

    gconfig->ft_index_options.enable_fulltext_index = true;
    ptr_state_machine = new lgraph::StateMachine(sm_config, gconfig);
    ptr_rpc_service = new RPCService(ptr_state_machine);

    rpc_server_.AddService(ptr_rpc_service, brpc::SERVER_DOESNT_OWN_SERVICE);
    rpc_server_.Start(sm_config.rpc_port, NULL);

    ptr_state_machine->Start();
    return;
}

void on_shutdown_rpc_server() {
    try {
        ptr_state_machine->Stop();
    } catch (std::exception& e) {
        LOG_ERROR() << "Rest server shutdown failed: " << e.what();
    }
    rpc_server_.Stop(0);
    return;
}

void* test_rpc_server(void*) {
    std::unique_lock<std::mutex> l(lock_rpc);
    if (stage_3 == 0) {
        on_initialize_rpc_server();
        UT_LOG() << "rpc server is running";
        stage_3++;
        cond.notify_one();
    }
    if (stage_3 != 2) cond.wait(l);
    on_shutdown_rpc_server();
    UT_LOG() << __func__ << " thread exit";
    delete ptr_state_machine;
    delete ptr_rpc_service;
    return nullptr;
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

static void WritePythonProcedure() {
    fma_common::OutputFmaStream stream;
    std::string code_sleep = R"(
import time
def Process(db, input):
    t = 1
    try:
        t = int(input)
    except:
        return (False, 'input must be a time duration')
    print('sleeping for {} seconds'.format(t))
    time.sleep(t)
    return (True, '')
)";
    stream.Open("code_sleep.py");
    stream.Write(code_sleep.data(), code_sleep.size());
    stream.Close();

    std::string code_read = R"(
from liblgraph_python_api import *
def Process(db, input):
    n = 0
    try:
        n = int(input)
    except:
        pass
    if n == 0:
        n = 1000000
    txn = db.CreateReadTxn()
    it = txn.GetVertexIterator()
    nv = 0
    while it.IsValid() and nv < n:
        nv = nv + 1
        it.Next()
    return (True, str(nv))
)";
    stream.Open("code_read.py");
    stream.Write(code_read.data(), code_read.size());
    stream.Close();
}

static std::map<std::string, std::string> sImportContent = {
    {"schema",
     R"(
{"schema" : [
        {
            "label" : "Person",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name" : "name", "type":"STRING"},
                {"name" : "birthyear", "type":"INT16", "optional":true},
                {"name" : "phone", "type":"INT16","unique":true, "index":true}
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
        },
    {
	    "label": "PLAY_IN",
	    "type": "EDGE",
	    "properties": [{
		    "name": "role",
		    "type": "STRING",
		    "optional": true
	    }],
	    "constraints": [
		    ["Person", "Film"]
	    ]
    }
    ]})"},
    {"person_desc",
     R"(
{"files": [
    {
        "columns": [
            "name",
            "birthyear",
            "phone"
        ],
        "format": "CSV",
        "header": 0,
        "label": "Person"
        }
    ]
})"},
    {"person",
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
    {"city_desc",
     R"(
{
    "files": [
        {
            "columns": [
                "name"
            ],
            "format": "CSV",
            "header": 1,
            "label": "City"
        }
    ]
})"},
    {"city",
     R"(Header Line
New York
London
Houston
)"},
    {"film_desc",
     R"(
{
    "files": [
        {
            "columns": [
                "title"
            ],
            "format": "CSV",
            "header": 0,
            "label": "Film"
        }
    ]
})"},
    {"film",
     R"("Goodbye, Mr. Chips"
Batman Begins
Harry Potter and the Sorcerer's Stone
The Parent Trap
Camelot
)"},
    {"has_child_desc",
     R"(
{
    "files": [
        {
            "DST_ID": "Person",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "label": "HAS_CHILD"
        }
    ]
})"},
    {"has_child",
     R"(Rachel Kempson,Vanessa Redgrave
Rachel Kempson,Corin Redgrave
Michael Redgrave,Vanessa Redgrave
Michael Redgrave,Corin Redgrave
Corin Redgrave,Jemma Redgrave
Vanessa Redgrave,Natasha Richardson
Roy Redgrave,Michael Redgrave
)"},
    {"married_desc",
     R"(
{
    "files": [
        {
            "DST_ID": "Person",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "label": "MARRIED"
        }
    ]
})"},
    {"married",
     R"(Rachel Kempson,Michael Redgrave
Michael Redgrave,Rachel Kempson
Natasha Richardson,Liam Neeson
Liam Neeson,Natasha Richardson
)"},
    {"born_in_desc",
     R"(
{
    "files": [
        {
            "DST_ID": "City",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID",
                "weight"
            ],
            "format": "CSV",
            "header": 0,
            "label": "BORN_IN"
        }
    ]
})"},
    {"born_in",
     R"(Vanessa Redgrave,London,20.21
Natasha Richardson,London,20.18
Christopher Nolan,London,19.93
Dennis Quaid,Houston,19.11
Lindsay Lohan,New York,20.59
John Williams,New York,20.55
)"},
    {"directed_desc",
     R"(
{
    "files": [
        {
            "DST_ID": "Film",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID"
            ],
            "format": "CSV",
            "header": 0,
            "label": "DIRECTED"
        }
    ]
})"},
    {"directed",
     R"(Christopher Nolan,Batman Begins
)"},
    {"wrote_desc",
     R"(
{
"files": [
    {
        "DST_ID": "Film",
        "SRC_ID": "Person",
        "columns": [
            "SRC_ID",
            "DST_ID"
        ],
        "format": "CSV",
        "header": 0,
        "label": "WROTE_MUSIC_FOR"
    }
]
})"},
    {"wrote",
     R"(John Williams,Harry Potter and the Sorcerer's Stone
John Williams,"Goodbye, Mr. Chips"
)"},
    {"acted_in_desc",
     R"(
{
    "files": [
        {
            "DST_ID": "Film",
            "SRC_ID": "Person",
            "columns": [
                "SRC_ID",
                "DST_ID",
                "charactername"
            ],
            "format": "CSV",
            "header": 0,
            "label": "ACTED_IN"
        }
    ]
})"},
    {"acted_in",
     R"(Michael Redgrave,"Goodbye, Mr. Chips",The Headmaster
Vanessa Redgrave,Camelot,Guenevere
Richard Harris,Camelot,King Arthur
Richard Harris,Harry Potter and the Sorcerer's Stone,Albus Dumbledore
Natasha Richardson,The Parent Trap,Liz James
Dennis Quaid,The Parent Trap,Nick Ducard
Lindsay Lohan,The Parent Trap,Halle/Annie
Liam Neeson,Batman Begins,Henri Ducard
)"}};

static void WriteYagoFiles() {
    static const std::map<std::string, std::string> data = {
        {"yago.conf",
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
                {"name" : "phone", "type":"INT16","unique":true, "index":true}
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
         R"(Header Line
New York
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

int ElementCount(const web::json::value& val, const std::string& value, const std::string& field) {
    int count = 0;
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(field)) {
                if (val.at(i).at(field).as_string() == value) {
                    ++count;
                }
            }
        }
    } else if (val.is_object()) {
        if (val.has_field(field)) {
            if (val.at(field).as_string() == value) {
                ++count;
            }
        }
    }
    return count;
}

bool HasElement(const web::json::value& val, const std::string& value, const std::string& field) {
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(field)) {
                if (val.at(i).at(field).as_string() == value) {
                    return true;
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.has_field(field)) return false;
        if (val.at(field).as_string() == value) {
            return true;
        }
    }
    return false;
}

static const int INTEGER = 1;
static const int BOOL = 2;
static const int DOUBLE = 3;
static const int STRING = 4;
std::unordered_map<std::string, int> TypeMap{
    {"INTEGER", 1},
    {"BOOL", 2},
    {"DOUBLE", 3},
    {"STRING", 4},
};

bool Equal(const web::json::value& lval, const std::string& rval, const std::string& type) {
    auto it = TypeMap.find(type);
    if (it == TypeMap.end()) return false;
    try {
        switch (it->second) {
        case INTEGER:
            {
                int val = boost::lexical_cast<int>(rval);
                return lval.as_integer() == val;
            }
        case BOOL:
            {
                bool val = boost::lexical_cast<bool>(rval);
                return lval.as_bool() == val;
            }
        case DOUBLE:
            {
                double val1 = boost::lexical_cast<double>(rval);
                double val2 = lval.as_double();
                if ((val1 - val2 > -0.000001) && (val1 - val2 < 0.000001)) {
                    return true;
                }
                return false;
            }
        case STRING:
            {
                return lval.as_string() == rval;
            }
        }
    } catch (boost::bad_lexical_cast& e) {
        return false;
    }
    return false;
}

bool CheckElementEqual(const web::json::value& val, const std::string& val1,
                       const std::string& val2, const std::string& field1,
                       const std::string& field2, const std::string& type1,
                       const std::string& type2) {
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(field1)) {
                if (Equal(val.at(i).at(field1), val1, type1)) {
                    if (Equal(val.at(i).at(field2), val2, type2)) {
                        return true;
                    }
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.has_field(field1)) return false;
        if (Equal(val.at(field1), val1, type1)) {
            if (Equal(val.at(field2), val2, type2)) {
                return true;
            }
        }
    }
    return false;
}

bool CheckObjectElementEqual(const web::json::value& val, const std::string& obj,
                             const std::string& field, const std::string& value,
                             const std::string& type) {
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(obj)) {
                if (Equal(val.at(i).at(obj).at(field), value, type)) {
                    return true;
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.has_field(obj)) return false;
        if (Equal(val.at(obj).at(field), value, type)) {
            return true;
        }
    }
    return false;
}

bool CheckInnerObjectElementEqual(const web::json::value& val, const std::string& obj,
                                  const std::string& inner, const std::string& field,
                                  const std::string& value, const std::string& type) {
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(obj)) {
                if (val.at(i).at(obj).has_field(inner)) {
                    if (Equal(val.at(i).at(obj).at(inner).at(field), value, type)) {
                        return true;
                    }
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.has_field(obj)) return false;
        if (!val.at(obj).has_field(inner)) return false;
        if (Equal(val.at(obj).at(inner).at(field), value, type)) {
            return true;
        }
    }
    return false;
}

bool ObjectHasListValue(const web::json::value& val, const std::string& obj,
                        const std::string& list, const std::string value) {
    if (val.is_array()) {
        for (int i = 0; i < val.size(); ++i) {
            if (val.at(i).has_field(obj)) {
                if (val.at(i).at(obj).has_field(list)) {
                    auto arr = val.at(i).at(obj).at(list).as_array();
                    for (auto v : arr) {
                        if (v.as_string() == value) {
                            return true;
                        }
                    }
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.has_field((obj))) return false;
        if (!val.at(obj).has_field(list)) return false;
        auto arr = val.at(obj).at(list).as_array();
        for (auto v : arr) {
            if (v.as_string() == value) {
                return true;
            }
        }
    }
    return false;
}

void test_label(lgraph::RpcClient& client) {
    UT_LOG() << "test createVertexLabel , deleteLabel , vertexLabels";
    std::string str;
    // db.addLabel test
    bool ret = client.CallCypher(
        str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(
        str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(
        str,
        "CALL db.createVertexLabel('dirctor', 'name', 'name', string, false, 'age', int8, true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(HasElement(json_val, "actor", "label"));
    UT_EXPECT_TRUE(HasElement(json_val, "dirctor", "label"));
    UT_EXPECT_FALSE(HasElement(json_val, "actoddr", "label"));

    ret = client.CallCypher(str, "CALL db.deleteLabel('vertex', 'actor')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.deleteLabel('vertex', 'dirctor')");
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_FALSE(client.CallCypher(str, "CALL db.deleteLabel('vertex', 'dirctor')"));
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_FALSE(HasElement(json_val, "actor", "label"));
    UT_EXPECT_FALSE(HasElement(json_val, "dirctor", "label"));
    ret = client.CallCypher(
        str,
        "CALL db.createVertexLabel('actor', 'name', 'name', string, false, 'age', int8, true)");
    UT_EXPECT_TRUE(ret);
}

void test_relationshipTypes(lgraph::RpcClient& client) {
    UT_LOG() << "test createEdgeLabel , deleteLabel , edgeLabels";
    std::string str;
    bool ret = client.CallCypher(str,
                                 "CALL db.createEdgeLabel('followed', '[]', 'address', string, "
                                 "false, 'date', int32, false)");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str,
                            "CALL db.createEdgeLabel('followed', '[]', 'address', string, false, "
                            "'date', int32, false)");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(
        str,
        "CALL db.createEdgeLabel('followed', '[[\"df\",[\"wq\"]]]', 'address', string, false, "
        "'date', int32, false)");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str,
                            "CALL db.createLabel('edge', 'name', '[\"df\"]', ['zv', string, "
                            "true], ['zz', int8, false])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(
        str,
        "CALL db.createLabel('edge', 'name', '[[\"df\",[\"wq\"]]]', ['zv', string, "
        "true], ['zz', int8, false])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(
        str,
        "CALL db.createLabel('edge', 'followed', '[[\"df\",\"wq\"]]', ['zv', string, "
        "true], ['zz', int8, false])");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(
        str,
        "CALL db.createEdgeLabel('married', '[]', 'address', string, false, 'date', int32, false)");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.edgeLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "followed", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "married", "label"), true);

    ret = client.CallCypher(str, "CALL db.deleteLabel('edge', 'followed')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.deleteLabel('edge', 'married')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.edgeLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "followed", "label"), false);
    UT_EXPECT_EQ(HasElement(json_val, "married", "label"), false);
    ret = client.CallCypher(
        str,
        "CALL db.createEdgeLabel('married', '[]', 'address', string, false, 'date', int32, false)");
    UT_EXPECT_TRUE(ret);
}

void test_index(lgraph::RpcClient& client) {
    UT_LOG() << "test addIndex , deleteIndex , indexes";
    std::string str;
//    std::string test_str;
    bool ret = client.CallCypher(str, "CALL db.addIndex('actor', 'age', false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(
        str, "CALL db.createEdgeLabel('index_edge', '[]', 'index_value', string, false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addIndex(true, 'actor', 'age')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.addFullTextIndex(true, 'actor', 'age')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addFullTextIndex(true, 'actor', 'age')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.deleteFullTextIndex(true, 'actor', 'age')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.rebuildFullTextIndex('\"actor\"', '\"age\"')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.rebuildFullTextIndex('[\"actor\"]', '\"age\"')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.addFullTextIndex(true, 'actor', 'age')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.rebuildFullTextIndex('[\"actor\"]', '[]')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.fullTextIndexes()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addEdgeIndex("
                            "'index_edge','index_value',true,true)");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.addEdgeIndex("
                            "'index_edge','index_value',false,false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.addEdgeIndex("
                            "'index_edge','index_value',false,false)");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.indexes()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(ElementCount(json_val, "actor", "label"), 2);

    ret = client.CallCypher(
        str, "CALL db.createEdgeLabel('pair_edge', '[]', 'index_value', string, false)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.addEdgeIndex('pair_edge','index_value',false,true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.listLabelIndexes('pair_edge', 'edge')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(ElementCount(json_val, "pair_edge", "label"), 1);
    // delete index
    ret = client.CallCypher(str, "CALL db.deleteIndex('actor', 'age')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.deleteIndex('actor', 'age')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL db.deleteIndex('actor', 'name')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL db.indexes()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "actor", "label"), 1);

    ret = client.CallCypher(str, "CALL db.deleteEdgeIndex('index_edge', 'index_value')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.deleteEdgeIndex('index_edge', 'index_value')");
    UT_EXPECT_FALSE(ret);
}

void test_warmup(lgraph::RpcClient& client) {
    UT_LOG() << "test warmup";
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.warmup()");
    UT_EXPECT_TRUE(ret);
}

void test_info(lgraph::RpcClient& client) {
    UT_LOG() << "test info";
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.monitor.serverInfo()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.monitor.tuGraphInfo()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.task.terminateTask('12')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.takeSnapshot('snapsfiles')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.takeSnapshot()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.ha.clusterInfo()");
    UT_EXPECT_FALSE(ret);
}

void test_createlabel(lgraph::RpcClient& client) {
    UT_LOG() << "test createLabel , labels , getLabelSchema";
    std::string str;
    std::string test_str2;
    bool ret = client.CallCypher(str,
                                 "CALL db.createLabel('vertex', 'animal', 'sleep', ['eat', string, "
                                 "true], ['sleep', int8, false])");

    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(test_str2, "CALL db.createLabel('animal')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "animal", "label"), true);

    ret = client.CallCypher(str, "CALL db.getLabelSchema('vertex', 'animal')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(test_str2, "CALL db.getLabelSchema('animdal')");
    UT_EXPECT_FALSE(ret);

    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "sleep", "name"), true);
    UT_EXPECT_EQ(HasElement(json_val, "eat", "name"), true);
}

void test_label_field(lgraph::RpcClient& client) {
    UT_LOG()
        << "test alterLabelAddFields , alterLabelModFields , alterLabelDelFields , getLabelSchema";
    std::string str;
    bool ret = client.CallCypher(str,
                                 "CALL db.alterLabelAddFields('vertex', 'animal',"
                                 "['run', string, '',true], ['jeep', int8, 10,false])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.alterLabelAddFields('vertex', 'animal_not_exist',"
                            "['run', string, '',true], ['jeep', int8, 10,false])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL db.alterLabelAddFields('vertex', 'animal',"
                            "['run', string, '',true], ['jeep', int12, 10,false])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.alterLabelAddFields('vertex', 'animal')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "db.propertyKeys()");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL db.getLabelSchema('vertex', 'animal')");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "run", "name"), true);
    UT_EXPECT_EQ(HasElement(json_val, "jeep", "name"), true);

    ret = client.CallCypher(str,
                            "CALL db.alterLabelModFields('vertex', 'animal',"
                            "['run', int8, false], ['jeep', int32, true])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL db.alterLabelModFields('vertex', 'animal_not_exist',['run', "
                            "int8, false], ['jeep', int32, true])");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL db.alterLabelModFields('vertex', 'animal')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL db.getLabelSchema('vertex', 'animal')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(CheckElementEqual(json_val, "run", "INT8", "name", "type", "STRING", "STRING"),
                 true);
    UT_EXPECT_EQ(CheckElementEqual(json_val, "jeep", "INT32", "name", "type", "STRING", "STRING"),
                 true);

    ret = client.CallCypher(
        str, "CALL db.alterLabelDelFields('vertex', 'animal_not_exist', ['run', 'jeep'])");
    UT_EXPECT_FALSE(ret);
    ret =
        client.CallCypher(str, "CALL db.alterLabelDelFields('vertex', 'animal', ['run', 'jeep'])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.alterLabelDelFields('vertex', 'animal')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.getLabelSchema('vertex', 'animal')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "run", "name"), false);
    UT_EXPECT_EQ(HasElement(json_val, "jeep", "name"), false);
    ret = client.CallCypher(str,
                            "CALL db.alterLabelAddFields('vertex', 'animal', "
                            "['null_string', string, null, true], "
                            "['null_int8', int8, null, true])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.alterLabelDelFields('vertex', 'animal', "
                            "['null_string', 'null_int8'])");
    UT_EXPECT_TRUE(ret);
}

void test_procedure(lgraph::RpcClient& client) {
    UT_LOG() << "test procedures";
    std::string str;
    bool ret = client.CallCypher(str, "CALL dbms.procedures()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.is_array(), true);
}

void test_graph(lgraph::RpcClient& client) {
    UT_LOG() << "test createGraph , listGraphs , modGraph , deleteGraph";
    std::string str;
    bool ret = client.CallCypher(
        str, "CALL dbms.graph.createGraph('test_graph1', 'this is a test graph1', 20)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(
        str, "CALL dbms.graph.createGraph('test_graph2', 'this is a test graph2', 100)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(
        str, "CALL dbms.graph.createGraph('test_graph2', 'this is a test graph2', 100)");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.graph.listGraphs()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "test_graph1", "graph_name"), true);
    UT_EXPECT_EQ(HasElement(json_val, "test_graph2", "graph_name"), true);

    ret = client.CallCypher(str,
                            "CALL dbms.graph.modGraph('test_graph1',"
                            " {max_size_GB:200, description:'modify graph1 desc'})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.graph.modGraph('test_graph1',"
                            " {max_size_GBs:200, description:'modify graph1 desc'})");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.graph.modGraph('test_graph1',"
                            " {max_size_GB:2.3, description:'modify graph1 desc'})");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.graph.modGraph('test_graph_not_exist',"
                            " {max_size_GB:200, description:'modify graph1 desc'})");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.graph.modGraph('test_graph1',"
                            " {max_size_GB:200, description:['modify graph1 desc']})");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str,
                            "CALL dbms.graph.modGraph('test_graph2',"
                            " {max_size_GB:500, description:'modify graph2 desc'})");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL dbms.graph.listGraphs()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "configuration", "description",
                                         "modify graph1 desc", "STRING"),
                 true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "configuration", "description",
                                         "modify graph2 desc", "STRING"),
                 true);

    ret = client.CallCypher(str, "CALL dbms.graph.deleteGraph('test_graph1')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.graph.deleteGraph('test_graph2')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.graph.deleteGraph('test_graph_not_exist')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.graph.listGraphs()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "test_graph1", "graph_name"), false);
    UT_EXPECT_EQ(HasElement(json_val, "test_graph2", "graph_name"), false);

    ret = client.CallCypher(str,
                            "CALL dbms.graph.createGraph('test_graph1',"
                            " 'this is a test graph1', 20)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.graph.createGraph('test_graph2',"
                            " 'this is a test graph2', 100)");
    UT_EXPECT_TRUE(ret);
}

void test_allow_host(lgraph::RpcClient& client) {
    UT_LOG() << "test addAllowedHosts , listAllowedHosts , deleteAllowedHosts";
    std::string str;
    bool ret =
        client.CallCypher(str, "CALL dbms.security.addAllowedHosts('172.172.1.1', '127.0.0.1')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listAllowedHosts()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "172.172.1.1", "host"), true);
    UT_EXPECT_EQ(HasElement(json_val, "127.0.0.1", "host"), true);

    ret =
        client.CallCypher(str, "CALL dbms.security.deleteAllowedHosts('172.172.1.1', '127.0.0.1')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listAllowedHosts()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "172.172.1.1", "host"), false);
    UT_EXPECT_EQ(HasElement(json_val, "127.0.0.1", "host"), false);
}

void test_configration(lgraph::RpcClient& client) {
    UT_LOG() << "test config.list , config.update";
    std::string str;
    bool ret = client.CallCypher(str, "CALL dbms.config.list()");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str,
                            "CALL dbms.config.update({durable:false,"
                            " enable_audit_log:true, enable_ip_check:true})");
    UT_EXPECT_TRUE(ret);
}

void test_configration_valid(lgraph::RpcClient& client) {
    UT_LOG() << "test config.list , config.update is valid";
    std::string str;
    bool ret = client.CallCypher(str, "CALL dbms.config.list()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ((CheckElementEqual(json_val, "durable", "0", "name", "value", "STRING", "BOOL")),
                 true);
    UT_EXPECT_EQ(
        (CheckElementEqual(json_val, "enable_audit_log", "1", "name", "value", "STRING", "BOOL")),
        true);
    UT_EXPECT_EQ(
        (CheckElementEqual(json_val, "enable_ip_check", "1", "name", "value", "STRING", "BOOL")),
        true);
}

void test_procedure_privilege(lgraph::RpcClient& client) {
    std::string str;
    bool ret =
        client.CallCypher(str, "CALL dbms.security.createRole('role_full', 'this is test role')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.modRoleAccessLevel('role_full',"
                            " {default:'FULL'})");
    ret = client.CallCypher(str,
                                 "CALL dbms.security.createUser('user_full', 'user_full')");
    ret = client.CallCypher(str,
                            "CALL dbms.security.addUserRoles('user_full', ['role_full'])");
    lgraph::RpcClient client_non_admin("0.0.0.0:19099", "user_full", "user_full");
    std::string code_cpp_path = "../../test/test_procedures/sortstr.cpp";
    // non-admin is not allowed to upload procedure
    ret = client_non_admin.LoadProcedure(str, code_cpp_path, "CPP", "test_procedure_1", "CPP",
                            "this is a test procedure", true, "v1");
    UT_EXPECT_FALSE(ret);
    ret = client.LoadProcedure(str, code_cpp_path, "CPP", "test_procedure_1", "CPP",
                             "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);
    // non-admin is not allowed to delete procedure
    ret = client_non_admin.CallCypher(str,
                                      "CALL db.plugin.deletePlugin('CPP', 'test_procedure_1')");
    UT_EXPECT_FALSE(ret);
    ret = client.DeleteProcedure(str, "CPP", "test_procedure_1");
    UT_EXPECT_TRUE(ret);
}

void test_role(lgraph::RpcClient& client) {
    UT_LOG() << "test createRole , listRoles , modRoleDesc , disableRole ,"
                " getRoleInfo , rebuildRoleAccessLevel , modSpecifiedAccessLevel , deleteRole";
    std::string str;
    bool ret =
        client.CallCypher(str, "CALL dbms.security.createRole('test_role1', 'this is test role1')");
    UT_EXPECT_TRUE(ret);
    ret =
        client.CallCypher(str, "CALL dbms.security.createRole('test_role2', 'this is test role2')");
    UT_EXPECT_TRUE(ret);
    ret =
        client.CallCypher(str, "CALL dbms.security.createRole('test_role2', 'this is test role2')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listRoles()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "test_role1", "role_name"), true);
    UT_EXPECT_EQ(HasElement(json_val, "test_role2", "role_name"), true);

    ret = client.CallCypher(str, "CALL dbms.security.modRoleDesc('test_role2', 'modified role2')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(
        str, "CALL dbms.security.modRoleDesc('test_role_not_exist', 'modified role2')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.disableRole('test_role2', true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.disableRole('test_role_not_exist', true)");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.getRoleInfo('test_role2')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "role_info", "description", "modified role2", "STRING"),
        true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "role_info", "disabled", "1", "BOOL"), true);

    ret = client.CallCypher(str,
                            "CALL dbms.security.rebuildRoleAccessLevel('test_role1',"
                            " {test_graph1:'READ', test_graph2:'WRITE'})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.rebuildRoleAccessLevel('test_role_not_exist',"
                            " {test_graph1:'READ'})");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.getRoleInfo('test_role1')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(CheckInnerObjectElementEqual(json_val, "role_info", "permissions", "test_graph1",
                                              "READ", "STRING"),
                 true);
    UT_EXPECT_EQ(CheckInnerObjectElementEqual(json_val, "role_info", "permissions", "test_graph2",
                                              "WRITE", "STRING"),
                 true);

    ret = client.CallCypher(str,
                            "CALL dbms.security.modRoleAccessLevel('test_role1',"
                            " {test_graph1:'FULL', test_graph2:'NONE'})");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.modRoleAccessLevel('test_role_not_exist',"
                            " {test_graph1:'FULL', test_graph2:'NONE'})");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str,
                            "CALL "
                            "dbms.security.modRoleFieldAccessLevel('test_role1','default','animal',"
                            "'run','vertex','WRITE')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL "
                            "dbms.security.modRoleFieldAccessLevel('test_role1','default','animal',"
                            "'run','edge','WRITE')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL "
                            "dbms.security.modRoleFieldAccessLevel('test_role1','default','animal',"
                            "'run','vertex_edge','WRITE')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(
        str,
        "CALL "
        "dbms.security.modRoleFieldAccessLevel('test_role_not_exist','default','animal',"
        "'run','vertex','WRITE')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.getRoleInfo('test_role1')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(CheckInnerObjectElementEqual(json_val, "role_info", "permissions", "test_graph1",
                                              "FULL", "STRING"),
                 true);

    ret = client.CallCypher(str, "CALL dbms.security.deleteRole('test_role1')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.deleteRole('test_role2')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.deleteRole('test_role2')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listRoles()");
    UT_EXPECT_TRUE(ret);
    UT_EXPECT_EQ(HasElement(json_val, "role_name", "test_role1"), false);
    UT_EXPECT_EQ(HasElement(json_val, "role_name", "test_role2"), false);

    ret = client.CallCypher(str,
                            "CALL dbms.security.createRole('test_role1', "
                            " 'this is test role1')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.createRole('test_role2', "
                            "'this is test role2')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.createRole('test_role3', "
                            " 'this is test role3')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.createRole('test_role4', "
                            "'this is test role4')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.createRole('test_role5', "
                            "'this is test role5')");
    UT_EXPECT_TRUE(ret);
}

void test_user(lgraph::RpcClient& client) {
    UT_LOG() << "test createUser , setCurrentDesc , listUsers , showCurrentUser ,"
                " setUserDesc , disableUser , getUserInfo , addUserRoles ,"
                " rebuildUserRoles , deleteUserRoles , deleteUserRoles";
    std::string str;
    bool ret = client.CallCypher(str,
                                 "CALL dbms.security.createUser('test_user1', "
                                 "'this is test user1')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.createUser('test_user2', "
                            "'this is test user2')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str,
                            "CALL dbms.security.createUser('test_user2', "
                            "'this is test user2')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.setCurrentDesc('modified user desc')");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listUsers()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "test_user1", "user_name"), true);
    UT_EXPECT_EQ(HasElement(json_val, "test_user2", "user_name"), true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "user_info", "description", "modified user desc",
                                         "STRING"),
                 true);

    ret = client.CallCypher(str, "CALL dbms.security.showCurrentUser()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "admin", "current_user"), true);

    ret = client.CallCypher(str,
                            "CALL dbms.security.setUserDesc('test_user2',"
                            " 'modified user2')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.setUserDesc('test_user_not_exist',"
                            " 'modified user2')");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.disableUser('test_user2', true)");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.disableUser('test_user_not_exist', true)");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.getUserInfo('test_user2')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "user_info", "description", "modified user2", "STRING"),
        true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "user_info", "disabled", "1", "BOOL"), true);

    ret = client.CallCypher(str,
                            "CALL dbms.security.addUserRoles('test_user1', ['test_role1',"
                            " 'test_role2', 'test_role3', 'test_role4', 'test_role5'])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.addUserRoles('test_user_not_exist', ['test_role1',"
                            " 'test_role2', 'test_role3', 'test_role4', 'test_role5'])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.addUserRoles('test_user1', [['test_role1'],"
                            " 'test_role2', 'test_role3', 'test_role4', 'test_role5'])");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listUsers()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role1"), true);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role2"), true);

    ret = client.CallCypher(str,
                            "CALL dbms.security.rebuildUserRoles('test_user1',"
                            " ['test_role3', 'test_role4', 'test_role5'])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.rebuildUserRoles('test_user1',"
                            " [['test_role3'], 'test_role4', 'test_role5'])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.rebuildUserRoles('test_user_not_exist',"
                            " ['test_role3', 'test_role4', 'test_role5'])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.listUsers()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role1"), false);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role2"), false);

    ret = client.CallCypher(str,
                            "CALL dbms.security.deleteUserRoles('test_user1',"
                            " ['test_role3', 'test_role4'])");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.deleteUserRoles('test_user1',"
                            " [['test_role3'], 'test_role4'])");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str,
                            "CALL dbms.security.deleteUserRoles('test_user_not_exist',"
                            " ['test_role3', 'test_role4'])");
    UT_EXPECT_FALSE(ret);

    ret = client.CallCypher(str, "CALL dbms.security.listUsers()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role3"), false);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role4"), false);
    UT_EXPECT_EQ(ObjectHasListValue(json_val, "user_info", "roles", "test_role5"), true);

    ret = client.CallCypher(str, "CALL dbms.security.deleteUser('test_user1')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.deleteUser('test_user2')");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.deleteUser('test_user2')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL dbms.security.listUsers()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "test_user1", "user_name"), false);
    UT_EXPECT_EQ(HasElement(json_val, "test_user2", "user_name"), false);
}

void test_flushDb(lgraph::RpcClient& client) {
    UT_LOG() << "test flushDB";
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.flushDB()");
    UT_EXPECT_TRUE(ret);
    client.CallCypher(str, "CALL dbms.listBackupFiles()");
}
void test_password(lgraph::RpcClient& client) {
    UT_LOG() << "test changeUserPassword , changePassword";
    std::string str;
    bool ret = client.CallCypher(str,
                                 "CALL dbms.security.createUser('test_pw_user',"
                                 " '13579@TuGraph')");
    UT_EXPECT_TRUE(ret);
    {
        lgraph::RpcClient new_client("0.0.0.0:19099", "test_pw_user", "13579@TuGraph");
        ret = new_client.CallCypher(str, "CALL dbms.security.showCurrentUser()", "");
        UT_EXPECT_TRUE(ret);
        web::json::value json_val = web::json::value::parse(str);
        UT_EXPECT_EQ(HasElement(json_val, "test_pw_user", "current_user"), true);
        ret = client.CallCypher(str,
                                "CALL dbms.security.changeUserPassword('test_not_exist_user',"
                                " '24680@TuGraph')");
        UT_EXPECT_FALSE(ret);
        ret = client.CallCypher(str,
                                "CALL dbms.security.changeUserPassword('test_pw_user',"
                                " '24680@TuGraph')");
        UT_EXPECT_TRUE(ret);
    }

    {
        lgraph::RpcClient new_client("0.0.0.0:19099", "test_pw_user", "24680@TuGraph");
        ret = new_client.CallCypher(str, "CALL dbms.security.showCurrentUser()", "");
        UT_EXPECT_TRUE(ret);
        web::json::value json_val = web::json::value::parse(str);
        UT_EXPECT_EQ(HasElement(json_val, "test_pw_user", "current_user"), true);

        ret = new_client.CallCypher(str,
                                    "CALL dbms.security.changePassword('24680@TuGraph',"
                                    " '13579@TuGraph')",
                                    "");
        UT_EXPECT_TRUE(ret);
        // client auto logout after changing password
        ret = new_client.CallCypher(str, "MATCH (n) RETURN n LIMIT 100", "default");
        UT_EXPECT_FALSE(ret);
    }

    {
        lgraph::RpcClient new_client("0.0.0.0:19099", "test_pw_user", "13579@TuGraph");
        ret = new_client.CallCypher(str, "CALL dbms.security.showCurrentUser()", "");
        UT_EXPECT_TRUE(ret);
        web::json::value json_val = web::json::value::parse(str);
        UT_EXPECT_EQ(HasElement(json_val, "test_pw_user", "current_user"), true);
    }
}

void test_cpp_procedure(lgraph::RpcClient& client) {
    UT_LOG() << "test listProcedure , deleteProcedure , LoadProcedure , CallProcedure";
    build_so();
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.dropDB()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size() == 0, true);
    std::string code_so_path = "./sortstr.so";

    ret = client.LoadProcedure(str, code_so_path, "CPP", "test_procedure1",
                               "SO", "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);

    ret = client.LoadProcedure(str, code_so_path, "CPP", "test_procedure1",
                               "SO", "this is a test procedure", true, "v1");
    UT_EXPECT_FALSE(ret);

    std::string code_scan_graph_path = "./scan_graph.so";
    ret = client.LoadProcedure(str, code_scan_graph_path, "CPP", "test_procedure2", "SO",
                            "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);
    std::string code_add_label_path = "./add_label.so";
    ret = client.LoadProcedure(str, code_add_label_path, "CPP", "test_procedure3", "SO",
                            "this is a test procedure", false, "v1");
    UT_EXPECT_TRUE(ret);

    std::string code_zip_path = "../../test/test_procedures/sortstr.zip";
    ret = client.LoadProcedure(str, code_zip_path, "CPP", "test_procedure4", "ZIP",
                            "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);

    std::string code_cpp_path = "../../test/test_procedures/sortstr.cpp";
    ret = client.LoadProcedure(str, code_cpp_path, "CPP", "test_procedure5", "CPP",
                            "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);

    std::string multi_procedure_path = "../../test/test_procedures/multi_files.cpp";
    std::string multi_header_path = "../../test/test_procedures/multi_files.h";
    std::string multi_core_path = "../../test/test_procedures/multi_files_core.cpp";
    ret = client.LoadProcedure(str, std::vector<std::string>{
                                        multi_procedure_path, multi_header_path, multi_core_path},
                               "CPP", "test_procedure6", "CPP",
                               "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);

#ifndef __SANITIZE_ADDRESS__
    ret = client.CallCypher(str, "CALL db.plugin.getPluginInfo('PY','countPersons')");
    UT_EXPECT_FALSE(ret);
    ret = client.CallCypher(str, "CALL db.plugin.listUserPlugins()");
    UT_EXPECT_TRUE(ret);
#endif
    ret = client.ListProcedures(str, "CPP", "any");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.as_array().size(), 6);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_procedure1",
                                "STRING"), true);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_procedure2",
                                "STRING"), true);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_procedure3",
                                "STRING"), true);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_procedure4",
                                "STRING"), true);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_procedure5",
                                "STRING"), true);
    UT_EXPECT_EQ(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_procedure6",
                                "STRING"), true);
    ret = client.CallProcedure(str, "CPP", "test_procedure1", "bcefg");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "bcefg", "result"), true);
    ret = client.CallProcedureToLeader(str, "CPP", "test_procedure1", "bcefg");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "bcefg", "result"), true);

    ret = client.CallProcedure(str, "CPP", "test_procedure2",
                               "{\"scan_edges\":true, \"times\":2}", 100.10);
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);

    ret = client.CallProcedure(str, "CPP", "test_procedure3",
                               "{\"label\":\"vertex1\"}", 20.0);
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "vertex1", "label"), true);
    ret = client.CallProcedure(str, "CPP", "test_procedure4", "9876543210", 10);
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "0123456789", "result"), true);

    ret = client.CallProcedure(str, "CPP", "test_procedure5", "a2o4i18u5eq3", 10.1);
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(HasElement(json_val, "123458aeioqu", "result"), true);
}

void test_python_procedure(lgraph::RpcClient& client) {
    std::string code_sleep("./code_sleep.py");
    std::string code_read("./code_read.py");

    WritePythonProcedure();
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.dropDB()");
    UT_EXPECT_TRUE(ret);

    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size() == 0, true);

    ret = client.LoadProcedure(str, code_sleep, "PY", "python_procedure1",
                               "PY", "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);

    ret = client.LoadProcedure(str, code_read, "PY", "python_procedure2",
                               "PY", "this is a test procedure", true, "v1");
    UT_EXPECT_TRUE(ret);

    ret = client.ListProcedures(str, "PY", "any");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.as_array().size(), 2);

    ret = client.DeleteProcedure(str, "PY", "python_procedure1");
    UT_EXPECT_TRUE(ret);
    ret = client.DeleteProcedure(str, "PY", "python_procedure2");
    UT_EXPECT_TRUE(ret);
    ret = client.DeleteProcedure(str, "PY", "python_procedure2");
    UT_EXPECT_FALSE(ret);

    ret = client.ListProcedures(str, "PY", "any");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size() == 0, true);
}

void test_cypher(lgraph::RpcClient& client) {
    UT_LOG() << "test CallCypher";
    std::string str;
    bool ret = client.CallCypher(str, "match (n) return count(n)");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(n)"].as_integer(), 6);
    ret = client.CallCypherToLeader(str, "match (n) return count(n)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(n)"].as_integer(), 6);
}

void test_gql(lgraph::RpcClient& client) {
    UT_LOG() << "test CallGQL";
    std::string str;
    bool ret = client.CallGql(str, "match (n) return count(n)");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(n)"].as_integer(), 6);
    ret = client.CallGqlToLeader(str, "match (n) return count(n)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(n)"].as_integer(), 6);
}

void test_import_file(lgraph::RpcClient& client) {
    UT_LOG() << "test ImportSchemaFromFile,ImportDataFromFile";
    WriteYagoFiles();
    std::string conf_file("./yago.conf");
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.dropDB()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size() == 0, true);
    ret = client.ImportSchemaFromFile(str, conf_file);
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size(), 3);
    UT_EXPECT_EQ(HasElement(json_val, "Person", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "City", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "Film", "label"), true);
    ret = client.CallCypher(str, "CALL db.edgeLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size(), 6);
    UT_EXPECT_EQ(HasElement(json_val, "HAS_CHILD", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "MARRIED", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "BORN_IN", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "DIRECTED", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "WROTE_MUSIC_FOR", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "ACTED_IN", "label"), true);
    ret = client.ImportDataFromFile(str, conf_file, ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (m:Person) return count(m)");
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(m)"].as_integer(), 13);

    ret = client.CallCypher(str, "match (m:City) return count(m)");
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(m)"].as_integer(), 3);

    ret = client.CallCypher(str, "match (m:Film) return count(m)");
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(m)"].as_integer(), 5);

    ret = client.CallCypher(str, "match (n)-[r:HAS_CHILD]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 7);

    ret = client.CallCypher(str, "match (n)-[r:MARRIED]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 4);

    ret = client.CallCypher(str, "match (n)-[r:BORN_IN]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 6);

    ret = client.CallCypher(str, "match (n)-[r:DIRECTED]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 1);

    ret = client.CallCypher(str, "match (n)-[r:WROTE_MUSIC_FOR]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 2);

    ret = client.CallCypher(str, "match (n)-[r:ACTED_IN]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 8);
}

void test_import_content(lgraph::RpcClient& client) {
    UT_LOG() << "test ImportSchemaFromContent,ImportDataFromContent";
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.dropDB()");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    web::json::value json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size() == 0, true);
    ret = client.ImportSchemaFromContent(str, sImportContent["schema"]);
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size(), 3);
    UT_EXPECT_EQ(HasElement(json_val, "Person", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "City", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "Film", "label"), true);
    ret = client.CallCypher(str, "CALL db.edgeLabels()");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val.size(), 7);
    UT_EXPECT_EQ(HasElement(json_val, "HAS_CHILD", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "MARRIED", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "BORN_IN", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "DIRECTED", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "WROTE_MUSIC_FOR", "label"), true);
    UT_EXPECT_EQ(HasElement(json_val, "ACTED_IN", "label"), true);

    ret = client.ImportDataFromContent(str, sImportContent["person_desc"], sImportContent["person"],
                                       ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (m:Person) return count(m)");
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(m)"].as_integer(), 13);

    ret =
        client.ImportDataFromContent(str, sImportContent["city_desc"], sImportContent["city"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (m:City) return count(m)");
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(m)"].as_integer(), 3);

    ret =
        client.ImportDataFromContent(str, sImportContent["film_desc"], sImportContent["film"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (m:Film) return count(m)");
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(m)"].as_integer(), 5);

    ret = client.ImportDataFromContent(str, sImportContent["has_child_desc"],
                                       sImportContent["has_child"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (n)-[r:HAS_CHILD]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 7);

    ret = client.ImportDataFromContent(str, sImportContent["married_desc"],
                                       sImportContent["married"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (n)-[r:MARRIED]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 4);

    ret = client.ImportDataFromContent(str, sImportContent["born_in_desc"],
                                       sImportContent["born_in"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (n)-[r:BORN_IN]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 6);

    ret = client.ImportDataFromContent(str, sImportContent["directed_desc"],
                                       sImportContent["directed"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (n)-[r:DIRECTED]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 1);

    ret = client.ImportDataFromContent(str, sImportContent["wrote_desc"], sImportContent["wrote"],
                                       ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (n)-[r:WROTE_MUSIC_FOR]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 2);

    ret = client.ImportDataFromContent(str, sImportContent["acted_in_desc"],
                                       sImportContent["acted_in"], ",");
    UT_EXPECT_TRUE(ret);
    ret = client.CallCypher(str, "match (n)-[r:ACTED_IN]->(m) return count(r)");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["count(r)"].as_integer(), 8);

    ret = client.CallCypher(str, "CALL db.getVertexSchema('Person')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["schema"]["properties"].size(), 3);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "schema", "label", "Person", "STRING"), true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "schema", "primary", "name", "STRING"), true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "schema", "type", "VERTEX", "STRING"), true);

    ret = client.CallCypher(str, "CALL db.getEdgeSchema('PLAY_IN')");
    UT_EXPECT_TRUE(ret);
    json_val = web::json::value::parse(str);
    UT_EXPECT_EQ(json_val[0]["schema"]["constraints"].size(), 1);
    UT_EXPECT_EQ(json_val[0]["schema"]["constraints"][0].size(), 2);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "schema", "label", "PLAY_IN", "STRING"), true);
    UT_EXPECT_EQ(CheckObjectElementEqual(json_val, "schema", "type", "EDGE", "STRING"), true);
}

void* test_rpc_client(void*) {
    using namespace lgraph;
    std::unique_lock<std::mutex> l(lock_rpc);
    if (stage_3 == 0) cond.wait(l);
    // start test user login
    UT_LOG() << "admin user login";
    {
        RpcClient client3("0.0.0.0:19099", "admin", "73@TuGraph");
        test_cypher(client3);
        test_gql(client3);
        test_label(client3);
        test_relationshipTypes(client3);
        test_index(client3);
        test_warmup(client3);
        test_createlabel(client3);
        test_label_field(client3);
        test_procedure(client3);
        test_graph(client3);
        test_allow_host(client3);
        test_info(client3);
        test_configration(client3);
    }
    {
        RpcClient client3("0.0.0.0:19099", "admin", "73@TuGraph");
        test_configration_valid(client3);
        test_role(client3);
        test_user(client3);
        test_flushDb(client3);
        test_password(client3);
        test_cpp_procedure(client3);
#ifndef __SANITIZE_ADDRESS__
        test_python_procedure(client3);
#endif
        test_import_file(client3);
        test_import_content(client3);
        test_procedure_privilege(client3);
    }

    stage_3++;
    cond.notify_one();
    UT_LOG() << __func__ << " thread exit";
    return nullptr;
}

class TestRPC : public TuGraphTest {};

TEST_F(TestRPC, RPC) {
    // fma_common::Logger::Get().SetLevel(fma_common::LogLevel::LL_DEBUG);
    std::thread tid_https[2] = {std::thread(test_rpc_server, nullptr),
                                std::thread(test_rpc_client, nullptr)};
    tid_https[0].join();
    tid_https[1].join();
}
