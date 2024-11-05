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

#include "cypher/execution_plan/ops/op_node_scan.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/parser/symbol_table.h"
#include "geax-front-end/ast/Ast.h"

namespace cypher {

static void ExtractLabelTree(std::vector<std::string>& labels,
                      const geax::frontend::LabelTree* root) {
    if (typeid(*root) == typeid(geax::frontend::SingleLabel)) {
        labels.emplace_back(((geax::frontend::SingleLabel*)root)->label());
    } else if (typeid(*root) == typeid(geax::frontend::LabelOr)) {
        auto* label_or = (geax::frontend::LabelOr*)root;
        ExtractLabelTree(labels, label_or->left());
        ExtractLabelTree(labels, label_or->right());
    } else {
        CYPHER_TODO();
    }
}

void NodeScan::ExtractPropertyKeys(std::unordered_set<std::string>& keys,
                         const geax::frontend::ElementFiller* filler) {
    auto& props = filler->predicates();
    for (auto p : props) {
        geax::frontend::PropStruct* prop = nullptr;
        checkedCast(p, prop);
        if (!prop) CYPHER_TODO();
        for (auto& property : prop->properties()) {
            keys.emplace(std::get<0>(property));
        }
    }
}

void NodeScan::ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& properties,
                                    const geax::frontend::ElementFiller* filler) {
    auto& props = filler->predicates();
    //if (props.size() > 1) CYPHER_TODO();
    for (auto p : props) {
        geax::frontend::PropStruct* prop = nullptr;
        checkedCast(p, prop);
        if (!prop) CYPHER_TODO();
        for (auto& property : prop->properties()) {
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto ent = ae.Evaluate(ctx, *record);
            if (!ent.IsConstant()) CYPHER_TODO();
            properties.emplace(std::get<0>(property), ent.constant);
        }
    }
}

NodeScan::NodeScan(txn::Transaction* txn, Node *node, const SymbolTable *sym_tab)
    : OpBase(OpType::NODE_SCAN, "Node Scan"), node_(node), sym_tab_(sym_tab) {
    CYPHER_THROW_ASSERT(node);
    alias_ = node->Alias();
    modifies.emplace_back(alias_);
    auto it = sym_tab->symbols.find(alias_);
    CYPHER_THROW_ASSERT(node && it != sym_tab->symbols.end());
    if (it != sym_tab->symbols.end()) node_rec_idx_ = it->second.id;
    rec_length_ = sym_tab->symbols.size();

    std::vector<std::string> labels;
    if (node_->ast_node_->filler()->label().has_value()) {
        ExtractLabelTree(labels,
                         node_->ast_node_->filler()->label().value());
    }
    if (labels.size() > 1) {
        CYPHER_TODO();
    }
    std::unordered_set<std::string> property_keys;
    ExtractPropertyKeys(property_keys, node_->ast_node_->filler());
    std::optional<std::string> label;
    if (!labels.empty()) {
        label = labels[0];
    }
    std::optional<std::unordered_set<std::string>> properties;
    if (!property_keys.empty()) {
        properties = property_keys;
    }
    vertex_iter_ = txn->GetVertexIteratorInfo(label, properties);
}

OpBase::OpResult NodeScan::NoChildInitialize(RTContext* ctx) {
    CYPHER_THROW_ASSERT(children.empty());
    record = std::make_shared<Record>(rec_length_, sym_tab_);
    record->values[node_rec_idx_].type = Entry::NODE;
    record->values[node_rec_idx_].node = node_;
    return OP_OK;
}

OpBase::OpResult NodeScan::NoChildRealConsume(RTContext* ctx) {
    if (!vit_) {
        // generate a new vertex iterator
        std::vector<std::string> labels;
        if (node_->ast_node_->filler()->label().has_value()) {
            ExtractLabelTree(labels,
                             node_->ast_node_->filler()->label().value());
        }
        if (!labels.empty()) {
            node_label_ = labels[0];
        }
        std::unordered_map<std::string, Value> properties;
        ExtractProperties(ctx, properties, node_->ast_node_->filler());
        if (!properties.empty()) {
            node_props_ = properties;
        }
        vit_ = ctx->txn_->NewVertexIterator(node_label_, node_props_);
        if (vit_->Valid()) {
            node_->vertex_ = vit_->GetVertex();
            return OP_OK;
        } else {
            return OP_DEPLETED;
        }
    } else {
        if (!vit_->Valid()) return OP_DEPLETED;
        vit_->Next();
        if (vit_->Valid()) {
            node_->vertex_ = vit_->GetVertex();
            return OP_OK;
        } else {
            return OP_DEPLETED;
        }
    }
}

OpBase::OpResult NodeScan::WithChildInitialize(RTContext* ctx) {
    CYPHER_THROW_ASSERT(!children.empty());
    auto child = children[0];
    auto res = child->Initialize(ctx);
    if (res != OP_OK) return res;
    // allocate a new record
    record = children[0]->record;
    record->values[node_rec_idx_].type = Entry::NODE;
    record->values[node_rec_idx_].node = node_;
    return OP_OK;
}

OpBase::OpResult NodeScan::HandOff() {
    if (!vit_ || !vit_->Valid()) return OP_REFRESH;
    vit_->Next();
    if (!vit_->Valid()) return OP_REFRESH;
    node_->vertex_ = vit_->GetVertex();
    return OP_OK;
}

OpBase::OpResult NodeScan::WithChildRealConsume(RTContext *ctx) {
    if (HandOff() == OP_OK) {
        return OP_OK;
    }
    auto child = children[0];
    while (child->Consume(ctx) == OP_OK) {
        // generate a new vertex iterator
        std::vector<std::string> labels;
        if (node_->ast_node_->filler()->label().has_value()) {
            ExtractLabelTree(labels, node_->ast_node_->filler()->label().value());
        }
        if (!labels.empty()) {
            node_label_ = labels[0];
        }
        std::unordered_map<std::string, Value> properties;
        ExtractProperties(ctx, properties, node_->ast_node_->filler());
        if (!properties.empty()) {
            node_props_ = properties;
        }
        vit_ = ctx->txn_->NewVertexIterator(node_label_, node_props_);
        if (vit_->Valid()) {
            node_->vertex_ = vit_->GetVertex();
            return OP_OK;
        }
    }
    return OP_DEPLETED;
}

OpBase::OpResult NodeScan::Initialize(RTContext *ctx) {
    if (children.empty()) {
        return NoChildInitialize(ctx);
    } else {
        return WithChildInitialize(ctx);
    }
}

OpBase::OpResult NodeScan::RealConsume(RTContext *ctx) {
    if (children.empty()) {
        return NoChildRealConsume(ctx);
    } else {
        return WithChildRealConsume(ctx);
    }
}

OpBase::OpResult NodeScan::ResetImpl(bool complete) {
    vit_.reset();
    node_label_.reset();
    node_props_.reset();
    return OP_OK;
}

}
