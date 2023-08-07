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
#include "fma-common/hardware_info.h"
#include "fma-common/logging.h"
#include "./unit_test_utils.h"

FMA_SET_TEST_PARAMS(HardwareInfo, "-n 4 -r 1000");

FMA_UNIT_TEST(HardwareInfo) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    using namespace fma_common;
    size_t n_cpu = 10;
    size_t n_rand = 100000000;
    Configuration config;
    config.Add(n_cpu, "n,nIter", true).Comment("Number of iterations to run cpu rate.");
    config.Add(n_rand, "r,nRand", true)
        .Comment("Number of random numbers to generate between each cpu rate test.");
    config.ParseAndFinalize(argc, argv);

    LOG() << HardwareInfo::GetCpuId();
    LOG() << HardwareInfo::GetCpuModel();
    LOG() << ToString(HardwareInfo::ListMacAddr());
    LOG() << HardwareInfo::GetHostName();
    LOG() << HardwareInfo::GetAvailableMemory();
    // by qinwei MemoeyInfo
    HardwareInfo::MemoryInfo memoryInfo;
    HardwareInfo::GetMemoryInfo(memoryInfo);
    LOG() << memoryInfo.total << " KB total";
    LOG() << memoryInfo.free << " KB free";
    LOG() << memoryInfo.available << " KB available";
    LOG() << memoryInfo.used << " KB used";
    LOG() << memoryInfo.shared << " KB shared";
    LOG() << memoryInfo.buff << " KB buff";
    LOG() << memoryInfo.cache << " KB cache";
    LOG() << memoryInfo.dwMemoryLoad << " % used";
    LOG() << memoryInfo.selfMemory << " KB selfMemory";
    // by qinwei CPURate
    HardwareInfo::CPURate cpuRate;
    double sum = 0;
    for (size_t i = 0; i < 10; i++) {
        HardwareInfo::CPURate cpuRate = HardwareInfo::GetCPURate();
        LOG() << "CPU%: " << cpuRate.selfCPURate << "\t" << cpuRate.serverCPURate;
        for (size_t k = 0; k < n_rand; k++) {
            sum += 1.0 / (k + 100);
        }
    }
    FMA_LOG() << sum;
    for (size_t i = 0; i < 3; i++) {
        auto diskStat = HardwareInfo::GetDiskRate();
        LOG() << "READ  " << diskStat.readRate / 1024 / 1024 << " MB/s"
              << "\tWRITE " << diskStat.writeRate / 1024 / 1024 << " MB/s";
        SleepS(1);
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
