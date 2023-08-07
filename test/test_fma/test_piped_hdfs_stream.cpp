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
#include "fma-common/piped_hdfs_stream.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

FMA_UNIT_TEST(PipedHdfsStream) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

#if (!HAS_LIBHDFS)
    std::string path;
    int v_size = 1024;
    int nv = 1;
    Configuration config;
    config.Add(path, "file").Comment("File path to use");
    config.Add(v_size, "size", true).Comment("Size of each vector");
    config.Add(nv, "nVectors", true).Comment("Number of vectors to write");
    config.Parse(argc, argv);
    config.Finalize();

    LOG() << "Writing to " << path;
    std::vector<int> v(v_size, 1);
    double t1 = GetTime();
    OutputHdfsStream out(path);
    for (int i = 0; i < nv; i++) {
        out.Write(&v[0], sizeof(int) * v_size);
    }
    out.Close();
    double t2 = GetTime();
    double mb = (double)v_size * sizeof(int) * nv;
    double mbps = mb / 1024 / 1024 / (t2 - t1);
    LOG() << "Write finished at " << mbps << "MB/s";

    std::fill(v.begin(), v.end(), 0);
    double sum = 0;
    t1 = GetTime();
    double t_compute = 0;
    InputHdfsStream in(path);
    for (int i = 0; i < nv; i++) {
        size_t r = in.Read(&v[0], sizeof(int) * v_size);
        FMA_UT_CHECK_EQ(r, sizeof(int) * v_size) << "Error reading file: got size " << r;
        double tt1 = GetTime();
        for (int j = 0; j < v_size; j++) {
            sum += v[j];
        }
        std::fill(v.begin(), v.end(), 0);
        t_compute += GetTime() - tt1;
    }
    in.Close();
    t2 = GetTime();
    FMA_UT_CHECK_EQ(sum, (double)v_size * nv) << "Data is corrupted";
    LOG() << "Read finished at " << mb / 1024 / 1024 / (t2 - t1 - t_compute) << "MB/s";
#endif

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
