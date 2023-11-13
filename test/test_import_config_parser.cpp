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

#include "fma-common/logger.h"
#include "gtest/gtest.h"
#include "import/import_config_parser.h"
#include "./ut_utils.h"

class TestImportConfigParser : public TuGraphTest {};

TEST_F(TestImportConfigParser, ImportConfigParser) {
    // unit test
    {
        using namespace lgraph;
        using namespace import_v2;
        UT_EXPECT_EQ(KeyWordFunc::GetKeyWordFromStr("BOOL"), KeyWord::BOOL);
        UT_EXPECT_EQ(KeyWordFunc::GetKeyWordFromStr("BLOB"), KeyWord::BLOB);
        UT_EXPECT_EQ(KeyWordFunc::GetKeyWordFromStr("LABEL"), KeyWord::LABEL);
        UT_EXPECT_EQ(KeyWordFunc::GetKeyWordFromStr("OPTIONAL"), KeyWord::OPTIONAL_);
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetKeyWordFromStr("NON_KEYWORD"));
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetKeyWordFromStr("BOOL_WITH_TAIL"));

        UT_EXPECT_EQ(KeyWordFunc::GetKeyWordFromFieldType(FieldType::BOOL), KeyWord::BOOL);
        UT_EXPECT_EQ(KeyWordFunc::GetKeyWordFromFieldType(FieldType::BLOB), KeyWord::BLOB);
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetKeyWordFromFieldType(FieldType::NUL));

        UT_EXPECT_EQ(KeyWordFunc::GetFieldTypeFromKeyWord(KeyWord::INT8), FieldType::INT8);
        UT_EXPECT_EQ(KeyWordFunc::GetFieldTypeFromKeyWord(KeyWord::STRING), FieldType::STRING);
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetFieldTypeFromKeyWord(KeyWord::LABEL));

        UT_EXPECT_EQ(KeyWordFunc::GetFieldTypeFromStr("INT16"), FieldType::INT16);
        UT_EXPECT_EQ(KeyWordFunc::GetFieldTypeFromStr("DATETIME"), FieldType::DATETIME);
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetFieldTypeFromStr("NOT_FIELD_TYPE"));
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetFieldTypeFromStr("NUL"));

        UT_EXPECT_EQ(KeyWordFunc::GetStrFromKeyWord(KeyWord::HEADER), "HEADER");
        UT_EXPECT_EQ(KeyWordFunc::GetStrFromKeyWord(KeyWord::OPTIONAL_), "OPTIONAL");
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetStrFromKeyWord(KeyWord(1024)));

        UT_EXPECT_EQ(KeyWordFunc::GetStrFromFieldType(FieldType::FLOAT), "FLOAT");
        UT_EXPECT_EQ(KeyWordFunc::GetStrFromFieldType(FieldType::BLOB), "BLOB");
        UT_EXPECT_ANY_THROW(KeyWordFunc::GetStrFromFieldType(FieldType::NUL));
    }

    using namespace lgraph;
    using namespace import_v2;
    {
        UT_LOG() << "Testing ParseSchema & ParseFiles";
        std::string dir = "./import_data";
        AutoCleanDir cleaner(dir);
        auto WriteFile = [](const std::string &name, const std::string &content) {
            fma_common::OutputFmaStream out(name);
            out.Write(content.data(), content.size());
        };
        WriteFile("./import_data/actors.csv", "");
        WriteFile("./import_data/movies.csv", "");
        WriteFile("./import_data/roles.csv", "");

        auto conf_str = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ],
           "primary" : "aid"
        },
        {
            "label" : "movie", 
            "type" : "VERTEX",
            "properties" : [
                {"name" : "mid", "type":"STRING"},
                {"name" : "name", "type":"STRING"},
                {"name" : "year", "type":"STRING"},
                {"name":"rate", "type":"FLOAT", "optional":true}
            ],
           "primary" : "mid"
        },
        {
            "label" : "play_in",
            "type" : "EDGE",
            "properties" : [
                {"name" : "role", "type":"STRING"},
                {"name" : "id", "type" : "INT64"}
            ],
            "temporal" : "id",
            "constraints" : [["actor", "movie"]]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/actors.csv",
            "header" : 2,
            "format" : "JSON",
            "label" : "actor",
            "columns" : ["aid","SKIP","name"]
        },
        {
            "path" : "./import_data/movies.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "movie",
            "columns" : ["mid","name","year","rate"]
        },
        {
            "path" : "./import_data/roles.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "play_in",
            "SRC_ID" : "actor",
            "DST_ID" : "movie",
            "columns" : ["SRC_ID","role","DST_ID"]
        }
    ]
}
        )"_json;
        ImportConfParser::ParseSchema(conf_str);
        ImportConfParser::ParseFiles(conf_str);
    }

    {
        UT_LOG() << "Testing schema error";
        auto conf_str1 = R"(
{
    "schema": [
        {
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ],
           "primary" : "aid"
        }
    ]
}
        )"_json;
        auto conf_str2 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ],
           "primary" : "aid"
        }
    ]
}
        )"_json;
        auto conf_str3 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ]
        }
    ]
}
        )"_json;
        auto conf_str4 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "primary" : "aid"
        }
    ]
}
        )"_json;
        auto conf_str5 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"},
                { "name" : "SKIP", "type":"STRING"}
            ],
           "primary" : "aid"
        }
    ]
}
        )"_json;
        auto conf_str6 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"},
                { "name" : "SRC_ID", "type":"STRING"}
            ],
           "primary" : "aid"
        }
    ]
}
        )"_json;
        auto conf_str7 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "VERTEX",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"},
                { "name" : "DST_ID", "type":"STRING"}
            ],
           "primary" : "aid"
        }
    ]
}
        )"_json;

        auto conf_str8 = R"(
{
    "schema": [
        {
            "label" : "actor",
            "type" : "EDGE",
            "properties" : [
                { "name" : "aid", "type":"STRING"},
                { "name" : "name", "type":"STRING"}
            ],
            "temporal" : "aid"
        }
    ]
}
        )"_json;

        auto conf_str9 = R"(
{
    "schema": [
        {
            "label" : "BORN_IN",
            "type" : "EDGE",
            "properties" : [
                {
                    "name" : "weight",
                    "type":"FLOAT",
                    "optional":false,
                    "unique":true,
                    "pair_unique":true}
            ]
        }
    ]
}
        )"_json;

        auto conf_str10 = R"(
{
    "schema": [
        {
            "label" : "BORN_IN",
            "type" : "EDGE",
            "properties" : [
                {
                    "name" : "reg_time",
                    "type":"DATETIME",
                    "optional":false,
                    "unique":false,
                    "pair_unique":true},
                {
                    "name" : "real_time",
                    "type":"DATETIME",
                    "optional":true,
                    "unique":false,
                    "pair_unique":false},
                {
                    "name" : "weight",
                    "type":"FLOAT",
                    "optional":false,
                    "unique":true,
                    "pair_unique":false}
            ]
        }
    ]
}
        )"_json;

        auto conf_str11 = R"(
{
    "schema": [
        {
            "label" : "Person",
            "type" : "VERTEX",
            "primary" : "reg_time",
            "properties" : [
                {"name" : "reg_time", "type":"DATETIME", "optional":true},
                {
                    "name" : "weight",
                    "type":"FLOAT",
                    "optional":false,
                    "pair_unique":true}
            ]
        }
    ]
}
        )"_json;

        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str1));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str2));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str3));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str4));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str5));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str6));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str7));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str8));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str9));
        ImportConfParser::ParseSchema(conf_str10).Dump();
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseSchema(conf_str11));
    }

    {
        UT_LOG() << "Testing files error";
        std::string dir = "./import_data";
        AutoCleanDir cleaner(dir);
        auto WriteFile = [](const std::string &name, const std::string &content) {
            fma_common::OutputFmaStream out(name);
            out.Write(content.data(), content.size());
        };
        WriteFile("./import_data/actors.csv", "");
        WriteFile("./import_data/movies.csv", "");
        WriteFile("./import_data/roles.csv", "");

        auto conf_str1 = R"(
{
    "files" : [
        {
            "path"   : "./import_data/actors.csv",
            "header" : 2,
            "format" : "JSON",
            "columns" : ["aid","SKIP","name"]
        }
    ]
}
        )"_json;

        auto conf_str2 = R"(
{
    "files" : [
        {
            "path" : "./import_data/actors.csv",
            "header" : 2,
            "label" : "actor",
            "columns" : ["aid","SKIP","name"]
        }
    ]
}
        )"_json;

        auto conf_str3 = R"(
{
    "files" : [
        {
            "path" : "./import_data/actors.csv",
            "header" : 2,
            "format" : "JSON",
            "label" : "actor"
        }
    ]
}
        )"_json;

        auto conf_str4 = R"(
{
    "files" : [
        {
            "path" : "./import_data/roles.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "play_in",
            "DST_ID" : "movie",
            "columns" : ["SRC_ID","role","DST_ID"]
        }
    ]
}
        )"_json;

        auto conf_str5 = R"(
{
    "files" : [
        {
            "path" : "./import_data/roles.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "play_in",
            "SRC_ID" : "actor",
            "columns" : ["SRC_ID","role","DST_ID"]
        }
    ]
}
        )"_json;

        auto conf_str6 = R"(
{
    "files" : [
        {
            "path" : "./import_data/roles.csv",
            "header" : 2,
            "format" : "CSV",
            "label" : "play_in",
            "SRC_ID" : "actor",
            "DST_ID" : "movie",
            "columns" : ["role","DST_ID"]
        }
    ]
}
        )"_json;
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseFiles(conf_str1));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseFiles(conf_str2));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseFiles(conf_str3));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseFiles(conf_str4));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseFiles(conf_str5));
        UT_EXPECT_ANY_THROW(ImportConfParser::ParseFiles(conf_str6));
    }
}
