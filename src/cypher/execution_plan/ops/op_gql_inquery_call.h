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
#include "procedure/procedure.h"
#include "cypher/execution_plan/ops/op.h"
#include "cypher/procedure/utils.h"
#include "resultset/record.h"
#include "tools/lgraph_log.h"

namespace cypher {
class GqlInQueryCall : public OpBase {
    const std::string func_name_;
    const std::vector<geax::frontend::Expr *> &args_;
    const std::optional<geax::frontend::YieldField *> &yield_;
    const PatternGraph *pattern_graph_ = nullptr;
    std::vector<Record> buffer_;
    std::vector<size_t> yield_idx_;
    std::vector<ArithExprNode> params_;
    std::unique_ptr<cypher::PluginAdapter> plugin_adapter_;

    OpResult HandOff(RTContext *ctx, std::shared_ptr<Record> &r) {
        if (buffer_.empty()) return OP_DEPLETED;
        auto &rec = buffer_.back();
        for (int i = 0; i < (int)yield_idx_.size(); i++) {
            if (rec.values[i].IsNode()) {
                cypher::Node &node =
                    const_cast<Node &>(pattern_graph_->GetNode(rec.values[i].node->Alias()));
                node.SetVid(rec.values[i].node->ID());
                node.ItRef()->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::VERTEX_ITER,
                                         rec.values[i].node->ID());
                r->values[yield_idx_[i]].type = Entry::NODE;
                r->values[yield_idx_[i]].node = &node;
            } else {
                r->values[yield_idx_[i]] = rec.values[i];
            }
        }
        buffer_.pop_back();
        return OP_OK;
    }

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
            record = std::make_shared<Record>(sym_tab.symbols.size(), &sym_tab, ctx->param_tab_);
        } else {
            CYPHER_THROW_ASSERT(children.size() == 1);
            if (children[0]->Initialize(ctx) != OP_OK) return OP_ERR;
            record = children[0]->record;
        }
        for (auto &item : yield_.value()->items()) {
            auto it = sym_tab.symbols.find(std::get<0>(item));
            if (it == sym_tab.symbols.end()) CYPHER_TODO();
            yield_idx_.emplace_back(it->second.id);
        }

        // build plugin adapter
        auto names = fma_common::Split(func_name_, ".");
        if (names.size() > 2 && names[0] == "plugin" && names[2] != "list") {
            std::string input, output;
            auto type = names[1] == "cpp" ? lgraph::PluginManager::PluginType::CPP
                                          : lgraph::PluginManager::PluginType::PYTHON;
            if (type == lgraph::PluginManager::PluginType::PYTHON) {
                throw lgraph::EvaluationException(
                    "Calling python plugin in CYPHER is disabled in this release.");
            }
            auto pm = ctx->ac_db_->GetLightningGraph()->GetPluginManager();
            lgraph_api::SigSpec *sig_spec = nullptr;
            pm->GetPluginSignature(type, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", names[2], &sig_spec);
            // it's ok for old plugins without signature
            // plugins without signature not support to be called using plugin_adapter_
            // use Utils::CallPlugin instead
            if (sig_spec) {
                plugin_adapter_ = std::make_unique<PluginAdapter>(sig_spec, type, names[2]);
                for (const auto &expr : args_) {
                    params_.emplace_back(expr, sym_tab);
                }
            }
        }
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        auto names = fma_common::Split(func_name_, ".");
        if (names.size() > 2 && names[0] == "plugin") {
            auto type = names[1] == "cpp" ? lgraph::PluginManager::PluginType::CPP
                                          : lgraph::PluginManager::PluginType::PYTHON;
            if (type == lgraph::PluginManager::PluginType::PYTHON) {
                throw lgraph::EvaluationException(
                    "Calling python plugin in CYPHER is disabled in this release.");
            }
            auto token = "A_DUMMY_TOKEN_FOR_CPP_PLUGIN";
            CYPHER_THROW_ASSERT(ctx->ac_db_);
            if (names[2] == "list") {
                buffer_.clear();
                auto plugins = ctx->ac_db_->ListPlugins(type, token);
                for (auto &p : plugins) {
                    std::string s;
                    s.append(p.name).append(" | ").append(p.desc).append(" | ").append(
                        p.read_only ? "READ" : "WRITE");
                    Record r;
                    r.AddConstant(lgraph::FieldData(s));
                    buffer_.emplace_back(std::move(r));
                }
            } else {
                if (!plugin_adapter_) CYPHER_TODO();
                // N.B. HERE NOT ABORT TXN, OTHERWISE WILL INVALID CHILDREN OPERATORS' ITERATOR
                // THE ROOT PROBLEM IS PLUGIN DESIGN PROBLEM.
                // `Process` function cannot accept ctx parameter or txn parameter
                //
                // N.B. signatured plugins cannot open txn in process, otherwise it will
                // cause sub-txn problem.
                // call custom plugin should have plugin_adapter
                CYPHER_THROW_ASSERT(plugin_adapter_ != nullptr);
                if (children.empty()) {
                    if (HandOff(ctx, record) == OP_OK) return OP_OK;
                    if (state == StreamDepleted) return OP_DEPLETED;
                    plugin_adapter_->Process(ctx, params_, *record, &buffer_);
                    std::reverse(buffer_.begin(), buffer_.end());
                    state = StreamDepleted;
                    return HandOff(ctx, record);
                } else {
                    if (HandOff(ctx, record) == OP_OK) return OP_OK;
                    auto child = children[0];
                    while (child->Consume(ctx) == OP_OK) {
                        plugin_adapter_->Process(ctx, params_, *record, &buffer_);
                        std::reverse(buffer_.begin(), buffer_.end());
                        if (HandOff(ctx, record) == OP_OK) return OP_OK;
                    }
                    return OP_DEPLETED;
                }
            }
        } else {
            if (children.empty()) {
                if (HandOff(ctx, record) == OP_OK) return OP_OK;
                if (state == StreamDepleted) return OP_DEPLETED;
                auto p = global_ptable.GetProcedure(func_name_);
                if (!p) {
                    throw lgraph::EvaluationException("unregistered standalone function: " +
                                                      func_name_);
                }
                std::vector<std::string> _yield_items;
                if (yield_.has_value()) {
                    for (auto &pair : yield_.value()->items()) {
                        _yield_items.emplace_back(std::get<0>(pair));
                    }
                }
                std::vector<Entry> parameters;
                parameters.reserve(args_.size());
                for (auto expr : args_) {
                    ArithExprNode node(expr, pattern_graph_->symbol_table);
                    parameters.emplace_back(node.Evaluate(ctx, *record.get()));
                }
                p->function(ctx, record.get(), parameters, _yield_items, &buffer_);
                std::reverse(buffer_.begin(), buffer_.end());
                state = StreamDepleted;
                return HandOff(ctx, record);
            } else {
                if (HandOff(ctx, record) == OP_OK) return OP_OK;
                auto child = children[0];
                while (child->Consume(ctx) == OP_OK) {
                    auto p = global_ptable.GetProcedure(func_name_);
                    if (!p) {
                        throw lgraph::EvaluationException("unregistered standalone function: " +
                                                          func_name_);
                    }
                    std::vector<std::string> _yield_items;
                    if (yield_.has_value()) {
                        for (auto &pair : yield_.value()->items()) {
                            _yield_items.emplace_back(std::get<0>(pair));
                        }
                    }
                    std::vector<Entry> parameters;
                    parameters.reserve(args_.size());
                    for (auto expr : args_) {
                        ArithExprNode node(expr, pattern_graph_->symbol_table);
                        parameters.emplace_back(node.Evaluate(ctx, *record.get()));
                    }
                    p->function(ctx, record.get(), parameters, _yield_items, &buffer_);
                    std::reverse(buffer_.begin(), buffer_.end());
                    if (HandOff(ctx, record) == OP_OK) return OP_OK;
                }
                return OP_DEPLETED;
            }
        }
        return OP_DEPLETED;
    }

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
