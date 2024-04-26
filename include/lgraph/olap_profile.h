//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

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
