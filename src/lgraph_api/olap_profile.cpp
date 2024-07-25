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
#include <cstring>
#include <chrono>
#include <thread>

#include "lgraph/olap_profile.h"

namespace lgraph_api {
namespace olap {

int MemUsage::parseMemLine(char * line) {
    int i = strlen(line);
    const char * p = line;
    while (*p < '0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

void MemUsage::reset() {
    maxMemUsage = 0;
    std::cout << "  reset memory record..." << std::endl;
    startMemRecord();
    return;
}

void MemUsage::startMemRecord(unsigned int interval) {
    std::cout << "  start memory record..." << std::endl;
    auto updateMaxMem = [&] () -> void {
        FILE * file = fopen("/proc/self/status", "r");
        size_t result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                result = parseMemLine(line);
                break;
            }
        }
        fclose(file);
        if (result > maxMemUsage) {
            maxMemUsage = result;
        }
    };

    std::thread(
            [updateMaxMem, interval]() {
                while (true) {
                    auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
                    updateMaxMem();
                    std::this_thread::sleep_until(x);
                }
            }).detach();
    // this->time_start(this->updateMaxMem, interval);
}

void MemUsage::print() {
    int64_t v = maxMemUsage / 1000;
    printf("mem usage: %ld MB\n", v);
}

}  // namespace olap
}  // namespace lgraph_api
