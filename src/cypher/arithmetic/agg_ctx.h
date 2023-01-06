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

//
// Created by wt on 19-2-15.
//
#pragma once

#include "resultset/record.h"

namespace cypher {
class AggCtx {
 public:
    // void *fctx;
    std::string err;
    Entry result;
    virtual int Step(const std::vector<Entry> &args) = 0;
    virtual int ReduceNext() = 0;
};
}  // namespace cypher
