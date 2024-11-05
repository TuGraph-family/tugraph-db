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

#include "cypher/execution_plan/ops/op.h"
#include "geax-front-end/ast/clause/ElementFiller.h"
namespace graphdb {
class VertexIterator;
}
namespace cypher {
class NodeScan : public OpBase {
    /* NOTE: Nodes in pattern graph are stored in std::vector, whose reference
     * will become INVALID after reallocation.
     * TODO(anyone) Make sure not add nodes to the pattern graph, otherwise use NodeId instead.  */
    friend class LocateNodeByVid;
    friend class LocateNodeByIndexedProp;

    Node *node_ = nullptr;
    std::unique_ptr<graphdb::VertexIterator> vit_ = nullptr;
    std::optional<std::string> node_label_;
    std::optional<std::unordered_map<std::string, Value>> node_props_;
    //lgraph::VIter *it_ = nullptr;           // also can be derived from node
    std::string alias_;                     // also can be derived from node
    std::string label_;                     // also can be derived from node
    size_t node_rec_idx_;                      // index of node in record
    size_t rec_length_;                        // number of entries in a record.
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    std::string vertex_iter_;
   OpResult HandOff();
   void ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& properties,
                                           const geax::frontend::ElementFiller* filler);
   void ExtractPropertyKeys(std::unordered_set<std::string>& keys,
                          const geax::frontend::ElementFiller* filler);

 public:
    NodeScan(txn::Transaction* txn, Node *node, const SymbolTable *sym_tab);

    OpResult Initialize(RTContext *ctx) override;
    OpResult NoChildInitialize(RTContext *ctx);
    OpResult WithChildInitialize(RTContext *ctx);

    OpResult RealConsume(RTContext *ctx) override;
    OpResult NoChildRealConsume(RTContext *ctx);
    OpResult WithChildRealConsume(RTContext *ctx);

    OpResult ResetImpl(bool complete) override;

    std::string ToString() const override {
        return fmt::format("{} [{},{}]", name, alias_, vertex_iter_);
    }

    Node *GetNode() const { return node_; }

    const SymbolTable *SymTab() const { return sym_tab_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
