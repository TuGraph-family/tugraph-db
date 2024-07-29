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
// Created by wt on 11/26/19.
//
#pragma once

#include "core/data_type.h"
#include "cypher/execution_plan/ops/op.h"
#include "graph/common.h"

namespace cypher {

class NodeIndexSeekDynamic : public OpBase {
    std::unique_ptr<lgraph::Transaction> *txn_ = nullptr;
    Node *node_ = nullptr;
    lgraph::VIter *it_ = nullptr;  // also can be derived from node
    std::string alias_;            // also can be derived from node
    std::string field_;
    std::string map_field_name_;
    bool hasMapFieldName;
    lgraph::FieldData value_;
    int value_rec_idx_;  // index of input variable
    int node_rec_idx_;   // index of node in record
    int rec_length_;     // number of entries in a record.
    const SymbolTable *sym_tab_;

    OpResult HandOff() {
        if (!it_ || !it_->IsValid()) return OP_REFRESH;
        it_->Next();
        return it_->IsValid() ? OP_OK : OP_REFRESH;
    }

 public:
    NodeIndexSeekDynamic(Node *node, const SymbolTable *sym_tab)
        : OpBase(OpType::NODE_INDEX_SEEK_DYNAMIC, "Node VertexIndex Seek (Dynamic)"),
          node_(node),
          sym_tab_(sym_tab) {
        if (node) {
            it_ = node->ItRef();
            alias_ = node->Alias();
            map_field_name_ = node->Prop().map_field_name;
            hasMapFieldName = node->Prop().hasMapFieldName;
            modifies.emplace_back(alias_);
        }
        rec_length_ = sym_tab->symbols.size();
        auto it = sym_tab->symbols.find(alias_);
        CYPHER_THROW_ASSERT(node && it != sym_tab->symbols.end());
        if (it == sym_tab->symbols.end()) {
            throw lgraph::CypherException("Undefined variable: " + alias_);
        }
        node_rec_idx_ = it->second.id;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        record = children[0]->record;
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;
        auto &pf = node_->Prop();
        field_ = pf.field;
        value_rec_idx_ = -1;
        switch (pf.type) {
        case Property::VALUE:
            value_ = pf.value;
            break;
        case Property::PARAMETER:
            {
                auto it = ctx->param_tab_.find(pf.value_alias);
                if (it == ctx->param_tab_.end())
                    throw lgraph::CypherException("invalid parameter: " + pf.value_alias);
                if (it->second.type != cypher::FieldData::SCALAR) {
                    throw lgraph::CypherException("parameter with wrong type: " + pf.value_alias);
                }
                value_ = it->second.scalar;
                break;
            }
        case Property::VARIABLE:
            {
                auto it = sym_tab_->symbols.find(pf.value_alias);
                if (it == sym_tab_->symbols.end()) {
                    throw lgraph::CypherException("Undefined variable: " + pf.value_alias);
                }
                value_rec_idx_ = it->second.id;
                if (!record->values[value_rec_idx_].IsScalar()) CYPHER_TODO();
                break;
            }
        default:
            CYPHER_INTL_ERR();
        }
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        /* Always set node's vid to -1:
         * - if found a new vertex ok, pull vid when use it;
         * - otherwise, set node to -1 in case mistaken for the
         *   vertex to be valid.
         * */
        node_->SetVid(-1);
        if (HandOff() == OP_OK) {
            return OP_OK;
        }
        auto child = children[0];
        while (child->Consume(ctx) == OP_OK) {
            // generate a new vertex iterator
            lgraph::FieldData value;
            if (value_rec_idx_ < 0) {
                value = value_;
            } else {
                if (record->values[value_rec_idx_].constant.IsMap()) {
                    auto map_data = *(record->values[value_rec_idx_].constant.map);
                    value = map_data[node_->Prop().map_field_name].scalar;
                } else if (record->values[value_rec_idx_].constant.IsArray()) {
                    THROW_CODE(CypherException, "Type error, do not support list as parameter.");
                } else {
                    value = record->values[value_rec_idx_].constant.scalar;
                }
            }
            if (!node_->Label().empty() && ctx->txn_->GetTxn()->IsIndexed(node_->Label(), field_)) {
                it_->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::INDEX_ITER,
                                node_->Label(), field_, value, value);
            } else {
                // Weak index iterator
                it_->Initialize(ctx->txn_->GetTxn().get(), node_->Label(), field_, value);
            }
            if (it_->IsValid()) return OP_OK;
        }
        return OP_DEPLETED;
    }

    OpResult ResetImpl(bool complete = false) override {
        if (complete) {
            if (it_ && it_->Initialized()) it_->FreeIter();
        } else {
            /* Invalidated iterator here, since it should be re-initialized
             * with a new input.
             * See also the examples in Apply::PullFromLhs.  */
            if (it_ && it_->Initialized()) it_->FreeIter();
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(alias_).append("]");
        return str;
    }

    Node *GetNode() const { return node_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
