/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 19-6-25.
//
#pragma once

#include "op.h"

namespace cypher {

class Limit : public OpBase {
    friend class LazyProjectTopN;
    size_t limit_ = 0;     // Max number of records to consume.
    size_t consumed_ = 0;  // Number of records consumed so far.

 public:
    explicit Limit(size_t limit) : OpBase(OpType::LIMIT, "Limit"), limit_(limit) {}

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        record = child->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        // Have we reached our limit?
        if (consumed_ >= limit_) return OP_DEPLETED;
        // Consume a single record.
        consumed_++;
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        auto res = child->Consume(ctx);
        return res;
    }

    OpResult ResetImpl(bool complete) override {
        consumed_ = 0;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(std::to_string(limit_)).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
