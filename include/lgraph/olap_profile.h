/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <stdio.h>
#include <stdlib.h>

namespace lgraph_api {

namespace olap {

class MemUsage {
 private:
    size_t  maxMemUsage;

    int parseMemLine(char * line);
 public:
    MemUsage() {
        maxMemUsage = 0;
    }

    int64_t getMaxMemUsage() {
        return maxMemUsage;
    }

    void reset();

    void startMemRecord(unsigned int interval = 1000);

    void print();
};

}  // namespace olap
}  // namespace lgraph_api
