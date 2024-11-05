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

#pragma once

#include <vector>
#include "cypher/execution_plan/ops/op.h"
#include "geax-front-end/ast/expr/Expr.h"
#include "cypher/graph/graph.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/procedure/procedure.h"

namespace cypher {
class GqlInQueryCall : public OpBase {
    const std::string func_name_;
    const std::vector<geax::frontend::Expr *> &args_;
    const std::optional<geax::frontend::YieldField *> &yield_;
    const PatternGraph *pattern_graph_ = nullptr;
    std::vector<std::vector<ProcedureResult>> buffer_;
    std::vector<size_t> yield_idx_;
    std::vector<ArithExprNode> params_;

    OpResult HandOff(RTContext *ctx, std::shared_ptr<Record> &r);
 public:
    GqlInQueryCall(const std::string &func_name, const std::vector<geax::frontend::Expr *> &args,
                   const std::optional<geax::frontend::YieldField *> &yield,
                   const PatternGraph *pattern_graph)
        : OpBase(OpType::GQL_INQUERY_CALL, "Gql In Query Call"),
          func_name_(func_name),
          args_(args),
          yield_(yield),
          pattern_graph_(pattern_graph) {
        state = StreamUnInitialized;
        std::vector<std::string> yield_names;
        CYPHER_THROW_ASSERT(yield_.has_value());
        for (auto &item : yield_.value()->items()) {
            yield_names.emplace_back(std::get<0>(item));
        }
        modifies = std::move(yield_names);
    }

    OpResult Initialize(RTContext *ctx) override {
        auto &sym_tab = pattern_graph_->symbol_table;
        if (children.empty()) {
            record = std::make_shared<Record>(sym_tab.symbols.size(), &sym_tab);
        } else {
            CYPHER_THROW_ASSERT(children.size() == 1);
            if (children[0]->Initialize(ctx) != OP_OK) return OP_ERR;
            record = children[0]->record;
        }
        for (auto &item : yield_.value()->items()) {
            auto it = sym_tab.symbols.find(std::get<0>(item));
            if (it == sym_tab.symbols.end()) CYPHER_TODO();
            yield_idx_.emplace_back(it->second.id);
            if (it->second.type == SymbolNode::NODE) {
                auto& n = const_cast<Node &>(pattern_graph_->GetNode(std::get<0>(item)));
                CYPHER_THROW_ASSERT(!n.Empty());
                CYPHER_THROW_ASSERT(!record->values[it->second.id].node);
                record->values[it->second.id].node = &n;
            }
        }
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override;
    OpResult ResetImpl(bool complete = false) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(func_name_).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
