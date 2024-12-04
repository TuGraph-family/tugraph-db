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
// Created by wt on 19-11-27.
//
#pragma once

#include "cypher/execution_plan/optimization/reduce_count.h"
#include "cypher/execution_plan/optimization/lazy_project_topn.h"
#include "cypher/execution_plan/optimization/edge_filter_pushdown_expand.h"
#include "cypher/execution_plan/optimization/var_len_expand_with_limit.h"
#include "cypher/execution_plan/optimization/locate_node_by_vid.h"
#include "cypher/execution_plan/optimization/locate_node_by_indexed_prop.h"
#include "cypher/execution_plan/optimization/parallel_traversal.h"
#include "cypher/execution_plan/optimization/opt_rewrite_with_schema_inference.h"
#include "execution_plan/optimization/locate_node_by_vid_v2.h"
#include "execution_plan/optimization/locate_node_by_indexed_prop_v2.h"
#include "execution_plan/optimization/locate_node_by_prop_range_filter.h"
#include "execution_plan/optimization/parallel_traversal_v2.h"
#include "execution_plan/optimization/rewrite_label_scan.h"

namespace cypher {

class PassManager {
    OpBase *root_ = nullptr;
    std::vector<OptPass *> all_passes_;

 public:
    explicit PassManager(OpBase *root, cypher::RTContext *ctx) : root_(root) {
        // all_passes_.emplace_back(new OptRewriteWithSchemaInference(ctx));
        // all_passes_.emplace_back(new PassReduceCount());
        // all_passes_.emplace_back(new EdgeFilterPushdownExpand());
        all_passes_.emplace_back(new LazyProjectTopN());
        all_passes_.emplace_back(new PassVarLenExpandWithLimit());
        all_passes_.emplace_back(new LocateNodeByVid());
        all_passes_.emplace_back(new LocateNodeByIndexedProp());
        all_passes_.emplace_back(new ParallelTraversal());
        all_passes_.emplace_back(new ParallelTraversalV2());
        all_passes_.emplace_back(new LocateNodeByVidV2());
        // all_passes_.emplace_back(new LocateNodeByIndexedPropV2());
        all_passes_.emplace_back(new ReplaceNodeScanWithIndexSeek(ctx));
        all_passes_.emplace_back(new LocateNodeByPropRangeFilter());
    }

    ~PassManager() {
        for (auto p : all_passes_) delete p;
        all_passes_.clear();
    }

    void RegisterPass();

    void ExecutePasses() {
        for (auto p : all_passes_) {
            if (p->Gate()) p->Execute(root_);
            std::function<void(cypher::OpBase *)> checkParent = [&](cypher::OpBase *op) {
                for (auto child : op->children) {
                    CYPHER_THROW_ASSERT(child->parent == op);
                    checkParent(child);
                }
            };
            checkParent(root_);
        }
    }

    void DumpPasses() const;
};

}  // namespace cypher
