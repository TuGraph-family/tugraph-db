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
// Created by wt on 6/29/18.
//
#pragma once

#include "filter/filter.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class OpFilter : public OpBase {
    friend class EdgeFilterPushdownExpand;
    std::shared_ptr<lgraph::Filter> filter_;
    /* FilterState
     * Different states in which ExpandAll can be at. */
    enum OpFilterState {
        FilterUninitialized,  /* Filter wasn't initialized it. */
        FilterRequestRefresh, /* */
        FilterResetted,       /* Filter was just restarted. */
    } _state;

 public:
    explicit OpFilter(const std::shared_ptr<lgraph::Filter> &filter)
        : OpBase(OpType::FILTER, "Filter"), filter_(filter) {
        _state = FilterUninitialized;
    }

    const std::shared_ptr<lgraph::Filter> &Filter() const { return filter_; }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        OpBase *child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        record = child->record;
        InitializeFilter(filter_);
        return OP_OK;
    }

    void InitializeFilter(std::shared_ptr<lgraph::Filter> root) {
        if (root->Type() == lgraph::Filter::TEST_IN_FILTER) {
            auto tif = std::dynamic_pointer_cast<lgraph::TestInFilter>(root);
            LocateAndSetProducer(this, tif);
        }
        if (root->Left() != nullptr) {
            InitializeFilter(root->Left());
        }
        if (root->Right() != nullptr) {
            InitializeFilter(root->Right());
        }
    }

    bool LocateAndSetProducer(OpBase *op, std::shared_ptr<lgraph::TestInFilter> tif) {
        bool allModified;
        for (auto child : op->children) {
            allModified = true;
            if (child->type == cypher::OpType::ARGUMENT) {
                // containModifies is true when all ae_right symbols are included in arguments
                for (auto alias : tif->RhsAlias()) {
                    if (std::find(child->modifies.begin(), child->modifies.end(), alias) ==
                        child->modifies.end()) {
                        allModified = false;
                        break;
                    }
                }
                if (allModified == true) {
                    tif->SetProducerOp(child);
                    return true;
                } else {
                    if (LocateAndSetProducer(child, tif) == true) {
                        return true;
                    }
                }
            } else {
                // return true when ARGUMENT found in the child tree
                if (LocateAndSetProducer(child, tif) == true) {
                    return true;
                }
            }
        }
        return false;
    }

    OpResult RealConsume(RTContext *ctx) override {
        OpResult res;
        OpBase *child = children[0];
        while (true) {
            res = child->Consume(ctx);
            if (res != OP_OK) return res;
            if (filter_->DoFilter(ctx, *child->record)) {
                break;
            }
        }
#ifndef NDEBUG
        LOG_DEBUG() << "[" << __FILE__ << "] " << filter_->ToString() << " passed!";
#endif
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        if (complete) {
            _state = FilterUninitialized;
        } else {
            _state = FilterResetted;
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(filter_->ToString()).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
