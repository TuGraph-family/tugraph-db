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
/**
 * This file shows how to write an embedding application that uses TuGraph as a library.
 * Embedding mode can come in handy when you want to debug your plugin.
 *
 * TuGraph's embedding mode shares exactly the same API with C++ plugins. You can refer
 * to plugin API document on how to use the APIs.
 *
 * In embedding mode, you start by creating a GraphDB instance, then create a transaction,
 * in which you can perform various operations.
 *
 * Since plugins already define the Process(DB, input, output) function in which operations
 * are performed, you can simply pass the GraphDB instance to the Process() function and
 * link this C++ source file with the plugin source file that you want to debug. You will
 * get an executable which you can run and debug easily.
 *
 * To make it easier to compile the embedding application, a BASH script named make_embed.sh
 * is provided, you can run it like this:
 *    user:~$ ./make_embed.sh ${PLUGIN_SOURCE_FILE_YOU_WANT_TO_DEBUG}
 */

#include <iostream>
#include "lgraph/lgraph.h"
#include "lgraph/olap_base.h"
using namespace std;

extern "C" bool Process(lgraph_api::GraphDB &db, const std::string &request, std::string &response);

int main(int argc, char **argv) {
    std::string db_path = "../build/output/lgraph_db";
    if (argc > 1)
        db_path = argv[1];
    lgraph_api::Galaxy g(db_path);
    g.SetCurrentUser("admin", "73@TuGraph");
    lgraph_api::GraphDB db = g.OpenGraph("default");
    std::string resp;
    bool r = Process(db, "{\"scan_edges\":true, \"times\":1}", resp);
    cout << r << endl;
    cout << resp << endl;
    return 0;
}
