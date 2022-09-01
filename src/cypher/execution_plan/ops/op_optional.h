/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 19-8-16.
//
#pragma once

#include "op.h"

namespace cypher {

class Optional : public OpBase {
 public:
    Optional() : OpBase(OpType::OPTIONAL_, "Optional") {}

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
        if (state == StreamDepleted) return OP_DEPLETED;
        auto &child = children[0];
        auto res = child->Consume(ctx);
        if (state == StreamUnInitialized) {
            if (res == OP_DEPLETED) {
                state = StreamDepleted;
                return OP_OK;
            }
            state = StreamConsuming;
        }
        return res;
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
