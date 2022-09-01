/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 2019/12/19.
//
#pragma once

#include "op.h"

namespace cypher {

class Distinct : public OpBase {
    std::unordered_set<std::string> uset_; /* used to identify unique records. */

 public:
    Distinct() : OpBase(OpType::DISTINCT, "Distinct") {}

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        record = child->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        while (true) {
            auto res = child->Consume(ctx);
            if (res != OP_OK) return res;
            auto hash_id = record->ToString();
            auto ret = uset_.emplace(hash_id);
            if (ret.second) break;
        }
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        uset_.clear();
        return OP_OK;
    }

    std::string ToString() const override { return name; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
