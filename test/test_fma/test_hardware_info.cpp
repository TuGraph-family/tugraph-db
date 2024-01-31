/**
 * Copyright 2024 AntGroup CO., Ltd.
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

    LOG_INFO() << HardwareInfo::GetCpuId();
    LOG_INFO() << HardwareInfo::GetCpuModel();
    LOG_INFO() << ToString(HardwareInfo::ListMacAddr());
    LOG_INFO() << HardwareInfo::GetHostName();
    LOG_INFO() << HardwareInfo::GetAvailableMemory();
    // by qinwei MemoeyInfo
    HardwareInfo::MemoryInfo memoryInfo;
    HardwareInfo::GetMemoryInfo(memoryInfo);
    LOG_INFO() << memoryInfo.total << " KB total";
    LOG_INFO() << memoryInfo.free << " KB free";
    LOG_INFO() << memoryInfo.available << " KB available";
    LOG_INFO() << memoryInfo.used << " KB used";
    LOG_INFO() << memoryInfo.shared << " KB shared";
    LOG_INFO() << memoryInfo.buff << " KB buff";
    LOG_INFO() << memoryInfo.cache << " KB cache";
    LOG_INFO() << memoryInfo.dwMemoryLoad << " % used";
    LOG_INFO() << memoryInfo.selfMemory << " KB selfMemory";
    // by qinwei CPURate
    HardwareInfo::CPURate cpuRate;
    double sum = 0;
    for (size_t i = 0; i < 10; i++) {
        HardwareInfo::CPURate cpuRate = HardwareInfo::GetCPURate();
        LOG_INFO() << "CPU%: " << cpuRate.selfCPURate << "\t" << cpuRate.serverCPURate;
        for (size_t k = 0; k < n_rand; k++) {
            sum += 1.0 / (k + 100);
        }
    }
    LOG_INFO() << sum;
    for (size_t i = 0; i < 3; i++) {
        auto diskStat = HardwareInfo::GetDiskRate();
        LOG_INFO() << "READ  " << diskStat.readRate / 1024 / 1024 << " MB/s"
              << "\tWRITE " << diskStat.writeRate / 1024 / 1024 << " MB/s";
        SleepS(1);
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
