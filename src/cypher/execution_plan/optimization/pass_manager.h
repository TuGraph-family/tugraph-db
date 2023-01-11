﻿/**
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

#include "reduce_count.h"
#include "lazy_project_topn.h"
#include "edge_filter_pushdown_expand.h"
#include "var_len_expand_with_limit.h"
#include "locate_node_by_vid.h"
#include "locate_node_by_indexed_prop.h"
#include "parallel_traversal.h"

namespace cypher {

class PassManager {
    ExecutionPlan *plan_ = nullptr;
    std::vector<OptPass *> all_passes_;

 public:
    explicit PassManager(ExecutionPlan *plan) : plan_(plan) {
        all_passes_.emplace_back(new PassReduceCount());
        all_passes_.emplace_back(new EdgeFilterPushdownExpand());
        all_passes_.emplace_back(new LazyProjectTopN());
        all_passes_.emplace_back(new PassVarLenExpandWithLimit());
        all_passes_.emplace_back(new LocateNodeByVid());
        all_passes_.emplace_back(new LocateNodeByIndexedProp());
        all_passes_.emplace_back(new ParallelTraversal());
    }

    ~PassManager() {
        for (auto p : all_passes_) delete p;
        all_passes_.clear();
    }

    void RegisterPass();

    void ExecutePasses() {
        for (auto p : all_passes_) {
            if (p->Gate()) p->Execute(plan_);
        }
    }

    void DumpPasses() const;
};

}  // namespace cypher
