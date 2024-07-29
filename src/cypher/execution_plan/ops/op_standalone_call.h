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
// Created by wt on 18-7-31.
//
#pragma once

#include "server/state_machine.h"
#include "cypher/execution_plan/ops/op.h"
#include "cypher/procedure/procedure.h"

namespace cypher {
class StandaloneCall : public OpBase {
    std::string func_name_;
    Procedure *procedure_ = nullptr;
    parser::Clause::TYPE_CALL call_clause_;

    void SetFunc(const std::string &name) {
        auto p = global_ptable.GetProcedure(name);
        if (!p) {
            throw lgraph::EvaluationException("unregistered standalone function: " + name);
        }
        procedure_ = p;
        func_name_ = name;
    }

 public:
    explicit StandaloneCall(const parser::QueryPart *stmt)
        : OpBase(OpType::STANDALONE_CALL, "Standalone Call"), call_clause_(*stmt->sa_call_clause) {}

    OpResult Initialize(RTContext *ctx) override { return OP_OK; }

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete = false) override { return OP_OK; }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(func_name_).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
