
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
#include "fma-common/logging.h"
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
    config.ParseAndFinalize(argc, argv);
    std::vector<std::string> graphs = fma_common::Split(graph_list, ",");

    double t1 = fma_common::GetTime();
    LOG() << "Warming up data in [" << dir << "]";
    FMA_LOG() << "Graph list: " << fma_common::ToString(graphs);
    lgraph::Galaxy g(dir, false);
    g.WarmUp("admin", graphs);
    double t2 = fma_common::GetTime();
    LOG() << "Warm up successful in " << t2 - t1 << " seconds.";
    return 0;
}
