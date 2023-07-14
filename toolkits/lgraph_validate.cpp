
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

#include <omp.h>
#include "fma-common/check_date.h"
#include "fma-common/configuration.h"
#include "lgraph/lgraph.h"

using namespace std;

inline std::string PrintEuid(const lgraph_api::EdgeUid& euid) {
    return fma_common::StringFormatter::Format("{}_{}_{}_{}", euid.src, euid.dst, euid.lid,
                                               euid.eid);
}

int main(int argc, char** argv) {
    std::string db_dir = "./testdb";
    std::string graph = "";
    std::string user, password;

    fma_common::Configuration config;
    config.Add(db_dir, "d,dir", true).Comment("Database directory");
    config.Add(graph, "g,graph", true).Comment("Graph to use");
    config.Add(user, "u,user", false).Comment("User name");
    config.Add(password, "p,password", false).Comment("Password");
    try {
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        FMA_ERR() << "Failed to parse command line option: " << e.what();
        return -1;
    }
    fma_common::Logger::Get().SetFormatter(std::make_shared<fma_common::TimedLogFormatter>());
    try {
        lgraph_api::Galaxy galaxy(db_dir, user, password, false, false);
        std::vector<std::string> graphs;
        if (graph.empty()) {
            auto g = galaxy.ListGraphs();
            for (auto& kv : g) graphs.push_back(kv.first);
        } else {
            graphs.push_back(graph);
        }
        for (auto& name : graphs) {
            FMA_LOG() << "Validating " << name;
            lgraph_api::GraphDB db = galaxy.OpenGraph(name, true);
            std::atomic<size_t> nv(0);
            std::atomic<size_t> ne(0);
            int64_t n_max = db.EstimateNumVertices();
            int64_t stride = 1000;
            int64_t last_print_vid_num = 0;
#pragma omp parallel for
            for (int64_t i = 0; i < n_max; i += stride) {
                auto txn = db.CreateReadTxn();
                auto vit = txn.GetVertexIterator(i);
                if (!vit.IsValid() || vit.GetId() >= i + stride) continue;
                for (; vit.IsValid() && vit.GetId() < i + stride; vit.Next()) {
                    nv++;
                    if (omp_get_thread_num() == 0 && nv - last_print_vid_num > 100000) {
                        last_print_vid_num = nv;
                        FMA_LOG() << "Checked " << nv << " vertices.";
                    }
                    for (auto oeit = vit.GetOutEdgeIterator(); oeit.IsValid(); oeit.Next()) {
                        ne++;
                        auto vvit = txn.GetVertexIterator(oeit.GetDst());
                        if (!vvit.IsValid()) {
                            FMA_WARN()
                                << "INCONSISTENT: edge(" << oeit.GetSrc() << ", " << oeit.GetDst()
                                << ") exists, but vertex " << oeit.GetDst() << " does not exist";
                        }
                        auto euid = oeit.GetUid();
                        auto ieit = txn.GetInEdgeIterator(euid);
                        if (!ieit.IsValid()) {
                            FMA_WARN() << "INCONSISTENT: outedge(" << PrintEuid(euid) << ") exists,"
                                       << " but corresponding inedge does not.";
                        } else {
                            if (oeit.GetAllFields() != ieit.GetAllFields()) {
                                FMA_WARN()
                                    << "INCONSISTENT: outedge(" << PrintEuid(euid) << ") has "
                                    << " different properties from its corresponding inedge: \n\t"
                                    << fma_common::ToString(oeit.GetAllFields()) << "\n\t"
                                    << fma_common::ToString(ieit.GetAllFields());
                            }
                        }
                    }
                    for (auto ieit = vit.GetInEdgeIterator(); ieit.IsValid(); ieit.Next()) {
                        auto vvit = txn.GetVertexIterator(ieit.GetSrc());
                        if (!vvit.IsValid()) {
                            FMA_WARN()
                                << "INCONSISTENT: edge(" << ieit.GetSrc() << ", " << ieit.GetDst()
                                << ") exists, but vertex " << ieit.GetSrc() << " does not exist";
                        }
                        auto euid = ieit.GetUid();
                        auto oeit = txn.GetInEdgeIterator(euid);
                        if (!oeit.IsValid()) {
                            FMA_WARN() << "INCONSISTENT: inedge(" << PrintEuid(euid) << ") exists,"
                                       << " but corresponding outedge does not.";
                        } else {
                            if (oeit.GetAllFields() != ieit.GetAllFields()) {
                                FMA_WARN()
                                    << "INCONSISTENT: inedge(" << PrintEuid(euid) << ") has "
                                    << " different properties from its corresponding outedge: \n\t"
                                    << fma_common::ToString(ieit.GetAllFields()) << "\n\t"
                                    << fma_common::ToString(oeit.GetAllFields());
                            }
                        }
                    }
                }
            }
            FMA_LOG() << "Finished checking " << name << ": nv=" << nv << ", ne=" << ne;
        }
    } catch (std::exception& e) {
        FMA_LOG() << "Error when peek db " << db_dir << ":\n\t" << e.what();
        return 1;
    }
    FMA_LOG() << "Data validation finished";
    return 0;
}
