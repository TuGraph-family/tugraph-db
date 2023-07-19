
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

#include <iostream>
#include "fma-common/configuration.h"
#include "core/lightning_graph.h"
#include "db/galaxy.h"

using namespace std;

static void PeekGraph(lgraph::AccessControlledDB& db, int64_t begin, int64_t end) {
    auto txn = db.CreateReadTxn();
    if (begin < 0) {
        auto it = txn.GetVertexIterator();
        while (it.IsValid()) {
            cout << txn.VertexToString(it) << endl;
            it.Next();
        }
    } else if (begin <= end) {
        auto it = txn.GetVertexIterator(begin, true);
        while (it.IsValid() && begin <= end) {
            cout << txn.VertexToString(it) << endl;
            it.Next();
            begin++;
        }
    } else {
        throw std::runtime_error("END should not be less than BEGIN");
    }
    txn.Abort();
}

int main(int argc, char** argv) {
    std::string db_dir = "./lgraph_db";
    std::string graph = "default";
    std::string user;
    std::string password;
    int64_t begin = -1;
    int64_t end = -1;
    int64_t vid = -1;

    fma_common::Configuration config;
    config.Add(db_dir, "d,dir", true).Comment("Database directory");
    config.Add(graph, "g,graph", true).Comment("Graph to peek");
    config.Add(user, "u,user", false).Comment("User name");
    config.Add(password, "p,password", false).Comment("Password");
    config.Add(begin, "b,begin", true).Comment("The position from where to dump");
    config.Add(end, "e,end", true).Comment("The position stop dumping");
    config.Add(vid, "v,vid", true).Comment("The vertex id to dump");
    try {
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        FMA_ERR() << "Failed to parse command line option: " << e.what();
        return -1;
    }
    if (!fma_common::FileSystem::GetFileSystem("/").IsDir(db_dir)) {
        FMA_LOG() << "DB directory " << db_dir << " does not exist.";
        return -1;
    }
    if (vid > 0) {
        begin = vid;
        end = vid;
    }
    FMA_LOG() << "Peeking graph [" << graph << "] stored in " << db_dir;
    if (begin < 0) {
        FMA_LOG() << "Peeking the whole graph.";
    } else {
        FMA_LOG() << "Peeking the graph: " << db_dir << "\n\tbegin:              " << begin
                  << "\n\tend:                " << end;
    }
    try {
        lgraph::Galaxy galaxy(db_dir);
        if (galaxy.GetUserToken(user, password).empty())
            throw lgraph::AuthError("Bad user/password.");
        lgraph::AccessControlledDB db = galaxy.OpenGraph(user, graph);
        PeekGraph(db, begin, end);
    } catch (std::exception& e) {
        FMA_LOG() << "Error when peek db " << db_dir << ":\n\t" << e.what();
        return 1;
    }
    return 0;
}
