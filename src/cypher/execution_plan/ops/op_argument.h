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
// Created by wt on 19-10-11.
//

#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

struct ArgIndex {
    std::string alias;
    // ASSERT the indexes in THIS record and INPUT record are the same.
    size_t rec_idx;
    SymbolNode::Type type;

    ArgIndex(const std::string &a, size_t idx, SymbolNode::Type t)
        : alias(a), rec_idx(idx), type(t) {}
};

class Argument : public OpBase {
    std::vector<ArgIndex> args_;
    std::shared_ptr<Record> *input_record_ = nullptr;
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    PatternGraph *pattern_graph_ = nullptr;

 public:
    explicit Argument(const SymbolTable *sym_tab);

    void Receive(std::shared_ptr<Record> *input_record, PatternGraph *pattern_graph);

    OpResult Initialize(RTContext *ctx) override;

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete) override;

    std::string ToString() const override;

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
