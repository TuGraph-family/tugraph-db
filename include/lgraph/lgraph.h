//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include "lgraph/lgraph_db.h"
#include "lgraph/lgraph_edge_iterator.h"
#include "lgraph/lgraph_galaxy.h"
#include "lgraph/lgraph_vertex_index_iterator.h"
#include "lgraph/lgraph_edge_index_iterator.h"
#include "lgraph/lgraph_txn.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_vertex_iterator.h"

#ifdef _WIN32
#pragma warning(disable : 4251)
#ifdef BUILD_DLL
#define LGAPI __declspec(dllexport)
#elif defined(USE_DLL)
#define LGAPI __declspec(dllimport)
#else
#define LGAPI
#endif
#else
#define LGAPI
#endif
#ifdef BOOL
#undef BOOL
#endif

namespace lgraph_api {
class Result;
}

namespace lgraph_api {
typedef bool GetSignature(SigSpec &sig_spec);
typedef bool Process(lgraph_api::GraphDB &db, const std::string &input, std::string &output);
typedef bool ProcessInTxn(lgraph_api::Transaction& txn,
                          const std::string &input,
                          lgraph_api::Result &output);
}

/*
 * TuGraph can be used as a server (sever mode) or a library (embedded mode).
 *
 * -------------
 * Embedded Mode
 * -------------
 * When used as a library, TuGraph can be linked to make a standalone
 * executable. The programmer can use lgraph_api::GraphDB() to construct a DB
 * instance, which can then be used to perform various operations.
 *
 * In embedded mode, neither the Restful server or the RPC server is started. So
 * only DB in local disk can be accessed in this mode. The embedded mode
 * provides a simple way to build graph capabilities into your own program
 * easily. It also makes debugging plugins easier: you can write a simple
 * embedding program which constructs GraphDB and then call your plugin code. A
 * quick example is attached at the end of this file.
 *
 * -------------
 * Server Mode
 * -------------
 * In server mode, the Restful service and RPC service are started along with
 * the server process. Users can then access the server via network using HTTP
 * or RPC clients. Plugins can be loaded dynamically into the server process and
 * called across network.
 *
 * Each C++ plugin must be packed into a dynamically loaded library (.so or
 * .dll) and must define the Process function with signature like the following:
 *
 *  extern "C" bool Process(GraphDB& db, const std::string& input, std::string&
 * output);
 *
 * The Process() function takes a reference of the DB, an binary input string,
 * and returns whether the call succeeded. If the call succeeded, it must return
 * true and the result data must be stored in output. In case of an error, the
 * output can be used to store the error message.
 *
 * The programmer is responsible for the correctness of the plugin, and make
 * sure that it does not cause a crash. Throwing exceptions is allowed inside
 * the Process function, in which case the call to this plugin is treated as a
 * failure and exception details are returned as a result of the request.
 */


// ------------------
// embed_main.cpp
// ------------------
// Sample embedding code to call a plugin, which can be linked with the plugin
// and used for debugging the plugin.
/**
#include <iostream>
#include "lgraph/lgraph.h"

extern "C" bool Process(lgraph_api::GraphDB &db, const std::string &request, std::string &response);

int main(int argc, char **argv) {
    lgraph_api::GraphDB db("./testdb");
    std::string input = "{\"scan_edges\":true, \"times\":1}";
    std::string output;
    bool r = Process(db, input, output);
    std::cout << r << std::endl;
    std::cout << output << std::endl;
    return 0;
}
*/

// ------------------
// plugin_example.cpp
// ------------------
// An example to show you how to write a plugin. It uses JSON to parse the
// input parameters and then scans the whole graph, counting number of vertices
// and nodes. Since input is a binary string, you can also pass parameters in
// other formats, such as Protobuf, BSON, or even XML.
/**
#include <iostream>
#include <string>
#include "json.hpp"
#include "lgraph/lgraph.h"
using json = nlohmann::json;
using namespace lgraph_api;

extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    bool scan_edges = false;
    int times = 10;
    try {
        std::cout << "input: " << request << std::endl;
        json input = json::parse(request);
        scan_edges = input["scan_edges"].get<bool>();
        times = input["times"].get<int>();
    } catch (std::exception &e) {
        response = std::string("error parsing json: ") + e.what();
        return false;
    }
    auto txn = db.CreateReadTxn();
    size_t num_vertices = 0;
    size_t num_edges = 0;
    for (int i = 0; i < times; i++)
        for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
            num_vertices += 1;
            if (scan_edges) {
                for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                    num_edges += 1;
                }
            }
        }
    json output;
    output["num_vertices"] = num_vertices;
    if (scan_edges) {
        output["num_edges"] = num_edges;
    }
    response = output.dump();
    return true;
}
*/

// ------------------
// build_embeded.sh
// ------------------
// This is a BASH command file to show you how to compile an embedding
// application. As an example, it links the example_plugin. You can also link
// with other libraries if you want.

/**
#!/bin/bash

EMBEDDING_CPP = . / embed_main.cpp
PLUGIN_CPP = . / plugin_example.cpp
LG_INCLUDE = / usr / include / lgraph
LG_LIB = / usr / lib / lgraph

g++ -fno -gnu -unique -fPIC -g --std = c++14 -rdynamic -O3 -fopenmp -DNDEBUG \
    -o embedding_app \
    "${LG_LIB}/liblgraph.so" \
    ${ PLUGIN_CPP } \
    ${ EMBEDDING_CPP }

*/
