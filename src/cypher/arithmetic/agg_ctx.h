/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
