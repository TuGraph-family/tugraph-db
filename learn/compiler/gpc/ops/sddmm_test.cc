/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include "compiler/gpc/ops/sddmm.h"

using namespace gpc;
int main(int argc, char const *argv[]) {
    int c = 1;
    std::vector<int> params{1, 1, 1, 1, 1, 0, 1, 0};
    std::vector<int> config{1, 1};
    auto op = new Sddmm(params, config, "ADD");
    return 0;
}