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

#include <utility>

#include "cypher/execution_plan/ops/op.h"
#include "cypher/procedure/procedure.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace cypher {
    // TODO(lingsu): rename in the future
class GqlStandaloneCall : public OpBase {
    const std::string func_name_;
    const std::vector<geax::frontend::Expr*>& args_;
    const std::optional<geax::frontend::YieldField*>& yield_;
    const SymbolTable& symbol_table_;
    std::vector<std::vector<ProcedureResult>> buffer_;

    OpResult HandOff(RTContext *ctx, std::shared_ptr<Record> &r);
 public:
    GqlStandaloneCall(std::string  func_name,
                        const std::vector<geax::frontend::Expr*>& args,
                        const std::optional<geax::frontend::YieldField*>& yield,
                        const SymbolTable& symbol_table)
        : OpBase(OpType::GQL_STANDALONE_CALL, "Gql Standalone Call")
        , func_name_(std::move(func_name))
        , args_(args)
        , yield_(yield)
        , symbol_table_(symbol_table) {
        state = StreamUnInitialized;
    }

    OpResult Initialize(RTContext *ctx) override;

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    [[nodiscard]] std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(func_name_).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
