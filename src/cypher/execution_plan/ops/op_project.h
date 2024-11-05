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
// Created by wt on 6/14/18.
//
#pragma once
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class Project : public OpBase {
    friend class LazyProjectTopN;
    const SymbolTable &sym_tab_;
    std::vector<ArithExprNode> return_elements_;
    std::vector<std::string> return_alias_;
    bool single_response_;
    enum {
        Uninitialized,
        RefreshAfterPass,
        Resetted,
        Consuming,
    } state_;  // TODO(anyone) use OpBase state

 public:
    Project(const std::vector<std::tuple<ArithExprNode, std::string>> &items,
                     const SymbolTable *sym_tab);

    OpResult Initialize(RTContext *ctx) override {
        if (!children.empty()) {
            auto &child = children[0];
            auto res = child->Initialize(ctx);
            if (res != OP_OK) return res;
        }
        /* projection */
        record = std::make_shared<Record>(return_elements_.size());
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete) override {
        if (complete) {
            record = nullptr;
            single_response_ = false;
            state_ = Uninitialized;
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &i : return_alias_) {
            str.append(i).append(",");
        }
        if (!return_alias_.empty()) str.pop_back();
        str.append("]");
        return str;
    }

    const std::vector<ArithExprNode> &ReturnElements() const { return return_elements_; }

    const std::vector<std::string> &ReturnAlias() const { return return_alias_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
