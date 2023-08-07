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

#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/fma_stream.h"
#include "./unit_test_utils.h"

using namespace fma_common;

FMA_UNIT_TEST(DD) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    Configuration config;
    int loop = 1;
    int nf = 16;
    std::string path = "./";
    config.Add(loop, "loop", true).Comment("Number of loops");
    config.Add(nf, "n_files", true).Comment("Number of files");
    config.Add(path, "path").Comment("Path prefix");
    config.Parse(argc, argv);

    for (int i = 0; i < loop; i++) {
        std::vector<OutputFmaStream> ofs(nf);
        for (int j = 0; j < nf; j++) {
            ofs[j].Open(path + std::to_string(i * nf + j));
            LOG() << i << " " << j;
        }
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
