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
// Created by wt on 19-7-18.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class CartesianProduct : public OpBase {
    bool init;

    // TODO(anyone) optimize, such as expand
    void ResetStream(OpBase *root) {
        root->Reset();
        for (auto c : root->children) ResetStream(c);
    }

    OpResult PullFromStreams(RTContext *ctx);

 public:
    CartesianProduct() : OpBase(OpType::CARTESIAN_PRODUCT, "Cartesian Product"), init(true) {}

    OpResult Initialize(RTContext *ctx) override;

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete) override {
        if (complete) {
            record = nullptr;
        }
        init = true;
        return OP_OK;
    }

    std::string ToString() const override { return name; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
