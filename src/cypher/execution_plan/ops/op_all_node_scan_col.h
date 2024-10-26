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

#include "cypher/execution_plan/ops/op.h"
#include "cypher/resultset/column_vector.h"
#include "cypher/execution_plan/ops/op_config.h"

namespace cypher {

class AllNodeScanCol : public OpBase {
    /* NOTE: Nodes in pattern graph are stored in std::vector, whose reference
     * will become INVALID after reallocation.
     * TODO(anyone) Make sure not add nodes to the pattern graph, otherwise use NodeId instead.  */
    friend class LocateNodeByVid;
    friend class LocateNodeByVidV2;
    friend class LocateNodeByIndexedProp;
    friend class LocateNodeByIndexedPropV2;

    Node *node_ = nullptr;
    lgraph::VIter *it_ = nullptr;           // also can be derived from node
    std::string alias_;                     // also can be derived from node
    std::string label_;                     // also can be derived from node
    int node_rec_idx_;                      // index of node in record
    int rec_length_;                        // number of entries in a record.
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    bool consuming_ = false;                // whether begin consuming

 public:
    AllNodeScanCol(Node *node, const SymbolTable *sym_tab)
        : OpBase(OpType::ALL_NODE_SCAN, "All Node Scan"), node_(node), sym_tab_(sym_tab) {
        if (node) {
            it_ = node->ItRef();
            alias_ = node->Alias();
            modifies.emplace_back(alias_);
        }
        auto it = sym_tab->symbols.find(alias_);
        CYPHER_THROW_ASSERT(node && it != sym_tab->symbols.end());
        if (it != sym_tab->symbols.end()) node_rec_idx_ = it->second.id;
        rec_length_ = sym_tab->symbols.size();
        consuming_ = false;
    }

    OpResult Initialize(RTContext *ctx) override {
        // allocate a new record
        record = std::make_shared<Record>(rec_length_, sym_tab_, ctx->param_tab_);
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;
        // transaction allocated before in plan:execute
        // TODO(anyone) remove patternGraph's state (ctx)
        node_->ItRef()->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::VERTEX_ITER);
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        uint32_t count = 0;
        columnar_ = std::make_shared<DataChunk>();
        while (count < FLAGS_BATCH_SIZE) {
            node_->SetVid(-1);
            if (!it_ || !it_->IsValid()) return (count > 0) ? OP_OK : OP_DEPLETED;
            if (!consuming_) {
                consuming_ = true;
            } else {
                it_->Next();
                if (!it_->IsValid()) {
                    return (count > 0) ? OP_OK : OP_DEPLETED;
                }
            }
            int64_t vid = it_->GetId();
            for (auto& property : node_->ItRef()->GetFields()) {
                const std::string& property_name = property.first;
                const lgraph_api::FieldData& field = property.second;
                if (field.type == lgraph_api::FieldType::STRING) {
                    if (columnar_->string_columns_.find(property_name) ==
                        columnar_->string_columns_.end()) {
                        columnar_->string_columns_[property_name] =
                            std::make_unique<ColumnVector>(sizeof(cypher_string_t),
                            FLAGS_BATCH_SIZE, field.type);
                        columnar_->property_positions_[property_name] = 0;
                    }
                    columnar_->property_vids_[property_name].push_back(vid);
                    uint32_t pos = columnar_->property_positions_[property_name]++;
                    StringColumn::AddString(
                        columnar_->string_columns_[property_name].get(), pos,
                        field.AsString().c_str(), field.AsString().size());
                } else {
                    if (columnar_->columnar_data_.find(property_name) ==
                        columnar_->columnar_data_.end()) {
                        size_t element_size = ColumnVector::GetFieldSize(field.type);
                        columnar_->columnar_data_[property_name] =
                            std::make_unique<ColumnVector>(element_size,
                                FLAGS_BATCH_SIZE, field.type);
                        columnar_->property_positions_[property_name] = 0;
                    }
                    columnar_->property_vids_[property_name].push_back(vid);
                    uint32_t pos = columnar_->property_positions_[property_name]++;
                    ColumnVector::InsertIntoColumnVector(
                        columnar_->columnar_data_[property_name].get(), field, pos);
                }
            }

            count++;
        }
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        consuming_ = false;
        if (complete) {
            // undo method initialize()
            record = nullptr;
            // TODO(anyone) cleaned in ExecutionPlan::Execute
            if (it_ && it_->Initialized()) it_->FreeIter();
        } else {
            if (it_ && it_->Initialized()) it_->Reset();
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(alias_).append("]");
        return str;
    }

    Node *GetNode() const { return node_; }

    const SymbolTable *SymTab() const { return sym_tab_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
