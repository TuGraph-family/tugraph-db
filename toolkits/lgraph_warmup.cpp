
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

#include "tools/lgraph_log.h"
#include "fma-common/configuration.h"
#include "fma-common/utils.h"
#include "db/galaxy.h"

int main(int argc, char** argv) {
    std::string dir;
    std::string graph_list = "default";

    fma_common::Configuration config;
    config.ExitAfterHelp(true);
    config.Add(dir, "d,directory", false).Comment("Data directory");
    config.Add(graph_list, "g,graph_list", true)
        .Comment("List of graphs to warmup, separated with commas");
    try {
        config.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        return -1;
    }
    std::vector<std::string> graphs = fma_common::Split(graph_list, ",");

    double t1 = fma_common::GetTime();
    LOG_INFO() << "Warming up data in [" << dir << "]";
    LOG_INFO() << "Graph list: " << fma_common::ToString(graphs);
    lgraph::Galaxy g(dir, false);
    g.WarmUp("admin", graphs);
    double t2 = fma_common::GetTime();
    LOG_INFO() << "Warm up successful in " << t2 - t1 << " seconds.";
    return 0;
}
