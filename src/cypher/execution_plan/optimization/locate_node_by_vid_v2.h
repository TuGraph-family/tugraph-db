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
 *
 */

#pragma once

#include <string>
#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_all_node_scan.h"
#include "cypher/execution_plan/ops/op_node_by_id_seek.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "cypher/filter/filter.h"
#include "execution_plan/optimization/id_filter_detector.h"
#include "utils/geax_expr_util.h"
#include "cypher/execution_plan/optimization/id_filter_detector.h"

namespace cypher {

/**
 * Plan before optimization:
 *
 *  Produce Results
 *      Project [n]
 *          Filter [{id(false,n) = 1}]
 *              All Node Scan [n]
 *
 * Plan after optimization:
 *
 *
 * Produce Results
 *      Project [n]
 *          Node By Id Seek[1,]
 *
 */

class LocateNodeByVidV2 : public OptPass {
    void _AdjustNodeVidFilter(OpBase *root) {
        OpFilter *op_filter = nullptr;
        ArithExprNode ae;
       std::unordered_map<std::string, std::set<uint64_t>> target_vids;
        if (_FindNodeVidFilter(root, op_filter, target_vids)) {
            auto op_post = op_filter->parent;
            Node *node;
            const SymbolTable *symtab;

            if (op_filter->children[0]->type == OpType::ALL_NODE_SCAN) {
                node = dynamic_cast<AllNodeScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<AllNodeScan *>(op_filter->children[0])->sym_tab_;
            } else {
                node = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->sym_tab_;
            }
            auto it = target_vids.find(node->Alias());
            if (it == target_vids.end()) return;
            std::vector<lgraph::VertexId> vids;
            vids.insert(vids.end(), it->second.begin(), it->second.end());
            auto op_node_by_id_seek = new NodeByIdSeek(vids, node, symtab);
            op_node_by_id_seek->parent = op_post;
            op_post->RemoveChild(op_filter);
            OpBase::FreeStream(op_filter);
            op_filter = nullptr;
            op_post->AddChild(op_node_by_id_seek);
        }
    }

    bool _CheckVidFilter(OpFilter *&op_filter, std::unordered_map<std::string,
                         std::set<uint64_t>> &target_vids) {
        /**
         * @brief
         *  Check if the Filter meets the requirement of having only Id() operation.
         *
         *  Applicable scenarios:
         *      where id(n) = 1
         *      where id(n) = 2 or id(n) = 3
         *      where id(n) in [1,2,3]
         */
        auto filter = op_filter->Filter();
        CYPHER_THROW_ASSERT(filter->Type() == lgraph::Filter::GEAX_EXPR_FILTER);
        auto geax_filter = ((lgraph::GeaxExprFilter*)filter.get())->GetArithExpr();
        geax::frontend::Expr* expr = geax_filter.expr_;
        IDFilterDetector detector;
        if (!detector.Build(expr)) return false;
        target_vids = detector.GetVids();
        if (target_vids.empty()) return false;
        return true;
    }

    bool _FindNodeVidFilter(OpBase *root, OpFilter *&op_filter,
                            std::unordered_map<std::string, std::set<uint64_t>> &target_vids) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            (op->children[0]->type == OpType::ALL_NODE_SCAN ||
             op->children[0]->type == OpType::NODE_BY_LABEL_SCAN)) {
            op_filter = dynamic_cast<OpFilter *>(op);
            if (_CheckVidFilter(op_filter, target_vids)) {
                return true;
            }
        }

        for (auto child : op->children) {
            if (_FindNodeVidFilter(child, op_filter, target_vids)) return true;
        }

        return false;
    }

 public:
    LocateNodeByVidV2() : OptPass(typeid(LocateNodeByVidV2).name()) {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        _AdjustNodeVidFilter(root);
        return 0;
    }
};

}  // namespace cypher
