/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 18-7-31.
//
#pragma once

#include "server/state_machine.h"
#include "procedure/procedure.h"
#include "op.h"

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
    StandaloneCall(const parser::QueryPart *stmt)
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
