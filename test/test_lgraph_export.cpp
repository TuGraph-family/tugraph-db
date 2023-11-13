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
#include "fma-common/string_util.h"
#include "gtest/gtest.h"

#include "lgraph/lgraph.h"
#include "./graph_factory.h"
#include "./graph_gen.h"
#include "./test_tools.h"


class TestLGraphExport : public TuGraphTest {};

TEST_F(TestLGraphExport, LGraphExport) {
    const std::string db_dir = "./testdb";
    const std::string export_dir = "./export_dir";

    const std::string admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string admin_password = lgraph::_detail::DEFAULT_ADMIN_PASS;
    const std::string default_graph = lgraph::_detail::DEFAULT_GRAPH_DB_NAME;
    #ifdef _WIN32
    const std::string export_exe = "lgraph_export.exe";
    const std::string import_exe = "lgraph_import.exe";
    #else
    const std::string export_exe = "./lgraph_export";
    const std::string import_exe = "./lgraph_import";
    #endif
    {
        lgraph::AutoCleanDir _test_dir(db_dir);
        lgraph::AutoCleanDir _export_dir(export_dir);
        GraphFactory::create_yago(db_dir);
        lgraph::SubProcess dumper(
            UT_FMT("{} -d {} -e {} -g {} -u {} -p {}",
                export_exe,
                db_dir, export_dir,
                default_graph,
                admin_user,
                admin_password));
        UT_EXPECT_TRUE(dumper.Wait(10000));

        const std::string& imported_db = "./db2";
        lgraph::AutoCleanDir _t2(imported_db);
        lgraph::SubProcess importer(
            UT_FMT("{} -c {}/import.config -d {}", import_exe, export_dir, imported_db));
        UT_EXPECT_TRUE(importer.Wait(100000));
        // check graph equals
        CheckGraphEqual(db_dir, default_graph, admin_user, admin_password,
            imported_db, default_graph, admin_user, admin_password);
    }

    {
        lgraph::AutoCleanDir _test_dir(db_dir);
        lgraph::AutoCleanDir _export_dir(export_dir);
        GraphFactory::create_yago(db_dir);
        lgraph::SubProcess dumper(
            UT_FMT("{} -d {} -e {} -g {} -u {} -p {} -f json",
                export_exe,
                db_dir, export_dir,
                default_graph,
                admin_user,
                admin_password));
        UT_EXPECT_TRUE(dumper.Wait(10000));

        const std::string& imported_db = "./db2";
        lgraph::AutoCleanDir _t2(imported_db);
        lgraph::SubProcess importer(
            UT_FMT("{} -c {}/import.config -d {}", import_exe, export_dir, imported_db));
        UT_EXPECT_TRUE(importer.Wait(100000));
        // check graph equals
        CheckGraphEqual(db_dir, default_graph, admin_user, admin_password, imported_db,
                        default_graph, admin_user, admin_password);
    }

    {
        lgraph::AutoCleanDir _test_dir(db_dir);
        lgraph::AutoCleanDir _export_dir(export_dir);
        GraphFactory::create_yago(db_dir);
        lgraph::SubProcess dumper(UT_FMT("{} -d {} -e {} -g {} -u {} -p {} -f csv", export_exe,
                                         db_dir, export_dir, default_graph, admin_user,
                                         admin_password));
        UT_EXPECT_TRUE(dumper.Wait(10000));

        const std::string& imported_db = "./db2";
        lgraph::AutoCleanDir _t2(imported_db);
        lgraph::SubProcess importer(
            UT_FMT("{} -c {}/import.config -d {}", import_exe, export_dir, imported_db));
        UT_EXPECT_TRUE(importer.Wait(100000));
        // check graph equals
        CheckGraphEqual(db_dir, default_graph, admin_user, admin_password, imported_db,
                        default_graph, admin_user, admin_password);
    }
    {
        const std::map<std::string, std::string> data = {
            {"yago.conf", R"(
{
    "schema": [
        {
            "label" : "Person",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name" : "name", "type":"STRING"},
                {"name" : "height", "type":"INT32", "index":true},
                {"name" : "birthyear", "type":"INT16", "optional":true}
            ]
        },
        {
            "label" : "City",
            "type" : "VERTEX",
            "primary" : "name",
            "properties" : [
                {"name": "name", "type":"STRING"},
                {"name": "id", "type":"INT32"}
            ]
        },
        {
            "constraints":[["Person","City"]],
            "label" : "BORN_IN",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT", "index":true, "pair_unique":true}
            ]
        },
        {"constraints":[["Person","Person"],["Person","City"]], "label" : "KNOWS", "type" : "EDGE"}
    ],
    "files" : [
        {
            "path" : "person.csv",
            "format" : "CSV",
            "label" : "Person",
            "columns" : ["name","birthyear", "height"]
        },
        {
            "path" : "city.csv",
            "format" : "CSV",
            "label" : "City",
            "columns" : ["name", "id"]
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
            "path" : "knows_person.csv",
            "format" : "CSV",
            "label" : "KNOWS",
            "SRC_ID" : "Person",
            "DST_ID" : "Person",
            "columns" : ["SRC_ID","DST_ID"]
        },
        {
            "path" : "knows_city.csv",
            "format" : "CSV",
            "label" : "KNOWS",
            "SRC_ID" : "Person",
            "DST_ID" : "City",
            "columns" : ["SRC_ID","DST_ID"]
        }
    ]
}
                    )"},
            {"knows_person.csv", R"(Rachel Kempson, Michael Redgrave
Vanessa Redgrave, Corin Redgrave
Liam Neeson, Natasha Richardson
Richard Harris, Dennis Quaid
Lindsay Lohan, Jemma Redgrave
)"},
            {"knows_city.csv", R"(Michael Redgrave, London
Vanessa Redgrave, Houston
Corin Redgrave, New York
)"},
            {"person.csv",
             R"(Rachel Kempson,1910,166
Michael Redgrave,1908,150
Vanessa Redgrave,1937,160
Corin Redgrave,1939,170
Liam Neeson,1952,181
Natasha Richardson,1963,159
Richard Harris,1930,169
Dennis Quaid,1954,170
Lindsay Lohan,1986,173
Jemma Redgrave,1965,172
Roy Redgrave,1873,171
John Williams,1932,167
Christopher Nolan,1970,159
)"},

            {"city.csv",
             R"(New York,100
London,200
Houston,300
)"},

            {"born_in.csv",
             R"(Vanessa Redgrave,London,20.21
Natasha Richardson,London,20.18
Christopher Nolan,London,19.93
Dennis Quaid,Houston,19.11
Lindsay Lohan,New York,20.62
John Williams,New York,20.55
)"}};

        lgraph::AutoCleanDir _test_dir(db_dir);
        lgraph::AutoCleanDir _export_dir(export_dir);

        GraphFactory::CreateCsvFiles(data);
        lgraph::import_v3::Importer::Config config;
        config.config_file = "./yago.conf";
        config.db_dir = db_dir;
        config.delete_if_exists = true;
        config.graph = "default";
        lgraph::import_v3::Importer offline_importer(config);
        offline_importer.DoImportOffline();

        lgraph::SubProcess dumper(UT_FMT("{} -d {} -e {} -g {} -u {} -p {} -f csv", export_exe,
                                         db_dir, export_dir, default_graph, admin_user,
                                         admin_password));
        UT_EXPECT_TRUE(dumper.Wait(10000));

        nlohmann::json import_conf, export_conf;
        {
            std::ifstream ifs("./yago.conf");
            ifs >> import_conf;
        }
        {
            std::ifstream ifs(export_dir + "/import.config");
            ifs >> export_conf;
        }
        std::sort(import_conf["schema"].begin(), import_conf["schema"].end());
        for (auto& schema : import_conf["schema"]) {
            if (schema.contains("properties")) {
                std::sort(schema["properties"].begin(), schema["properties"].end());
            }
        }
        std::sort(export_conf["schema"].begin(), export_conf["schema"].end());
        for (auto& schema : export_conf["schema"]) {
            if (schema.contains("properties")) {
                std::sort(schema["properties"].begin(), schema["properties"].end());
            }
        }
        UT_EXPECT_EQ(import_conf["schema"], export_conf["schema"]);

        const std::string& imported_db = "./db2";
        lgraph::AutoCleanDir _t2(imported_db);
        lgraph::SubProcess importer(
            UT_FMT("{} -c {}/import.config -d {}", import_exe, export_dir, imported_db));
        UT_EXPECT_TRUE(importer.Wait(100000));
        // check graph equals
        CheckGraphEqual(db_dir, default_graph, admin_user, admin_password, imported_db,
                        default_graph, admin_user, admin_password);
    }
}
