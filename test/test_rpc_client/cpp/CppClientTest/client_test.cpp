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

#include "lgraph/lgraph_rpc_client.h"
#include "tools/json.hpp"
#include <boost/lexical_cast.hpp>

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
Lindsay Lohan,New York,20.62
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
Dennis Quaid,The Parent Trap,Nick Parker
Lindsay Lohan,The Parent Trap,Halle/Annie
Liam Neeson,Batman Begins,Henri Ducard
)"}};

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

bool Equal(const nlohmann::json& lval, const std::string& rval, const std::string& type) {
    auto it = TypeMap.find(type);
    if (it == TypeMap.end()) return false;
    try {
        switch (it->second) {
        case INTEGER:
            {
                int val = boost::lexical_cast<int>(rval);
                return lval.get<int>() == val;
            }
        case BOOL:
            {
                bool val = boost::lexical_cast<bool>(rval);
                return lval.get<bool>() == val;
            }
        case DOUBLE:
            {
                double val1 = boost::lexical_cast<double>(rval);
                double val2 = lval.get<double>();
                if ((val1 - val2 > -0.000001) && (val1 - val2 < 0.000001)) {
                    return true;
                }
                return false;
            }
        case STRING:
            {
                return lval.get<std::string>() == rval;
            }
        }
    } catch (boost::bad_lexical_cast& e) {
        return false;
    }
    return false;
}

bool HasElement(const nlohmann::json& val, const std::string& value, const std::string& field) {
    if (val.is_array()) {
        for (int i = 0; i < (int)val.size(); ++i) {
            if (val.at(i).contains(field)) {
                if (val.at(i).at(field).get<std::string>() == value) {
                    return true;
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.contains(field)) return false;
        if (val.at(field).get<std::string>() == value) {
            return true;
        }
    }
    return false;
}

bool CheckObjectElementEqual(const nlohmann::json& val, const std::string& obj,
                             const std::string& field, const std::string& value,
                             const std::string& type) {
    if (val.is_array()) {
        for (int i = 0; i < (int)val.size(); ++i) {
            if (val.at(i).contains(obj)) {
                if (Equal(val.at(i).at(obj).at(field), value, type)) {
                    return true;
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.contains(obj)) return false;
        if (Equal(val.at(obj).at(field), value, type)) {
            return true;
        }
    }
    return false;
}

void test_import_file(lgraph::RpcClient& client) {
    std::string conf_file("./data/yago/yago.conf");
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.dropDB()");
    assert(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    assert(ret);
    nlohmann::json json_val = nlohmann::json::parse(str);
    assert(json_val.size() == 0);
    ret = client.ImportSchemaFromFile(str, conf_file);
    assert(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val.size() == 3);
    assert(HasElement(json_val, "Person", "label") == true);
    assert(HasElement(json_val, "City", "label") == true);
    assert(HasElement(json_val, "Film", "label") == true);
    ret = client.CallCypher(str, "CALL db.edgeLabels()");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val.size() == 6);
    assert(HasElement(json_val, "HAS_CHILD", "label") == true);
    assert(HasElement(json_val, "MARRIED", "label") == true);
    assert(HasElement(json_val, "BORN_IN", "label") == true);
    assert(HasElement(json_val, "DIRECTED", "label") == true);
    assert(HasElement(json_val, "WROTE_MUSIC_FOR", "label") == true);
    assert(HasElement(json_val, "ACTED_IN", "label") == true);
    ret = client.ImportDataFromFile(str, conf_file, ",");
    assert(ret);
    ret = client.CallCypher(str, "match (m:Person) return count(m)");
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(m)"].get<int>() == 13);

    ret = client.CallCypher(str, "match (m:City) return count(m)");
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(m)"].get<int>() == 3);

    ret = client.CallCypher(str, "match (m:Film) return count(m)");
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(m)"].get<int>() == 5);

    ret = client.CallCypher(str, "match (n)-[r:HAS_CHILD]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 7);

    ret = client.CallCypher(str, "match (n)-[r:MARRIED]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 4);

    ret = client.CallCypher(str, "match (n)-[r:BORN_IN]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 6);

    ret = client.CallCypher(str, "match (n)-[r:DIRECTED]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 1);

    ret = client.CallCypher(str, "match (n)-[r:WROTE_MUSIC_FOR]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 2);

    ret = client.CallCypher(str, "match (n)-[r:ACTED_IN]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 8);
}


void test_import_content(lgraph::RpcClient& client) {
    std::string str;
    bool ret = client.CallCypher(str, "CALL db.dropDB()");
    assert(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    assert(ret);
    nlohmann::json json_val = nlohmann::json::parse(str);
    assert(json_val.size() == 0);
    ret = client.ImportSchemaFromContent(str, sImportContent["schema"]);
    assert(ret);
    ret = client.CallCypher(str, "CALL db.vertexLabels()");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val.size() == 3);
    assert(HasElement(json_val, "Person", "label") == true);
    assert(HasElement(json_val, "City", "label") == true);
    assert(HasElement(json_val, "Film", "label") == true);
    ret = client.CallCypher(str, "CALL db.edgeLabels()");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val.size() == 7);
    assert(HasElement(json_val, "HAS_CHILD", "label") == true);
    assert(HasElement(json_val, "MARRIED", "label") == true);
    assert(HasElement(json_val, "BORN_IN", "label") == true);
    assert(HasElement(json_val, "DIRECTED", "label") == true);
    assert(HasElement(json_val, "WROTE_MUSIC_FOR", "label") == true);
    assert(HasElement(json_val, "ACTED_IN", "label") == true);

    ret = client.ImportDataFromContent(str, sImportContent["person_desc"], sImportContent["person"],
                                       ",");
    assert(ret);
    ret = client.CallCypher(str, "match (m:Person) return count(m)");
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(m)"].get<int>() == 13);

    ret =
        client.ImportDataFromContent(str, sImportContent["city_desc"], sImportContent["city"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (m:City) return count(m)");
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(m)"].get<int>() == 3);

    ret =
        client.ImportDataFromContent(str, sImportContent["film_desc"], sImportContent["film"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (m:Film) return count(m)");
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(m)"].get<int>() == 5);

    ret = client.ImportDataFromContent(str, sImportContent["has_child_desc"],
                                       sImportContent["has_child"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (n)-[r:HAS_CHILD]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 7);

    ret = client.ImportDataFromContent(str, sImportContent["married_desc"],
                                       sImportContent["married"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (n)-[r:MARRIED]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 4);

    ret = client.ImportDataFromContent(str, sImportContent["born_in_desc"],
                                       sImportContent["born_in"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (n)-[r:BORN_IN]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 6);

    ret = client.ImportDataFromContent(str, sImportContent["directed_desc"],
                                       sImportContent["directed"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (n)-[r:DIRECTED]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 1);

    ret = client.ImportDataFromContent(str, sImportContent["wrote_desc"], sImportContent["wrote"],
                                       ",");
    assert(ret);
    ret = client.CallCypher(str, "match (n)-[r:WROTE_MUSIC_FOR]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 2);

    ret = client.ImportDataFromContent(str, sImportContent["acted_in_desc"],
                                       sImportContent["acted_in"], ",");
    assert(ret);
    ret = client.CallCypher(str, "match (n)-[r:ACTED_IN]->(m) return count(r)");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["count(r)"].get<int>() == 8);

    ret = client.CallCypher(str, "CALL db.getVertexSchema('Person')");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["schema"]["properties"].size() == 3);
    assert(CheckObjectElementEqual(json_val, "schema", "label", "Person", "STRING") == true);
    assert(CheckObjectElementEqual(json_val, "schema", "primary", "name", "STRING") == true);
    assert(CheckObjectElementEqual(json_val, "schema", "type", "VERTEX", "STRING") == true);

    ret = client.CallCypher(str, "CALL db.getEdgeSchema('PLAY_IN')");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(json_val[0]["schema"]["constraints"].size() == 1);
    assert(json_val[0]["schema"]["constraints"][0].size() == 2);
    assert(CheckObjectElementEqual(json_val, "schema", "label", "PLAY_IN", "STRING") == true);
    assert(CheckObjectElementEqual(json_val, "schema", "type", "EDGE", "STRING") == true);
}

void test_plugin(lgraph::RpcClient& client) {
    std::string str;

    std::string code_so_path = "./sortstr.so";
    bool ret = client.LoadProcedure(str, code_so_path, "CPP", "test_plugin1", "SO",
                                 "this is a test plugin", true, "v1");
    assert(ret);

    ret = client.LoadProcedure(str, code_so_path, "CPP", "test_plugin1",
                               "SO", "this is a test plugin", true, "v1");
    assert(ret == false);

    std::string code_scan_graph_path = "./scan_graph.so";
    ret = client.LoadProcedure(str, code_scan_graph_path, "CPP", "test_plugin2", "SO",
                            "this is a test plugin", true, "v1");
    assert(ret);
    std::string code_add_label_path = "./add_label.so";

    ret = client.ListProcedures(str, "CPP", "any");
    assert(ret);
    nlohmann::json json_val = nlohmann::json::parse(str);
    assert(json_val.size()== 2);
    assert(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_plugin1", "STRING")
        == true);
    assert(
        CheckObjectElementEqual(json_val, "plugin_description", "name", "test_plugin2", "STRING")
        == true);

    ret = client.CallProcedure(str, "CPP", "test_plugin1", "gecfb");
    assert(ret);
    json_val = nlohmann::json::parse(str);
    assert(HasElement(json_val, "bcefg", "result") == true);

    ret = client.CallProcedure(str, "CPP", "test_plugin2",
                             "{\"scan_edges\":true, \"times\":2}", 100.00);
    assert(ret);
    json_val = nlohmann::json::parse(str);
    json_val[0]["result"] = nlohmann::json::parse(json_val[0]["result"].get<std::string>());
    assert(CheckObjectElementEqual(json_val, "result", "num_edges", "56", "INTEGER") == true);
    assert(CheckObjectElementEqual(json_val, "result", "num_vertices", "42", "INTEGER") == true);
}

int main(int argc, char** argv) {
    lgraph::RpcClient client("0.0.0.0:9092", "admin", "73@TuGraph");

    test_import_file(client);

    test_import_content(client);

    test_plugin(client);
}
