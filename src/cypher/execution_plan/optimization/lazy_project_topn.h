/**
 * Copyright 2024 AntGroup CO., Ltd.
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

/*
 * Created on 6/23/21
 */
#pragma once

#include "cypher/execution_plan/ops/op_project.h"
#include "cypher/execution_plan/ops/op_limit.h"
#include "cypher/execution_plan/ops/op_sort.h"
#include "cypher/execution_plan/ops/op_topn.h"
#include "cypher/execution_plan/optimization/opt_pass.h"

namespace cypher {
/*
 * preproject topN:
 * MATCH (n) RETURN n.a, n.b, n.c ORDER BY n.a desc LIMIT k;
 * Plan before optimization:
 * Produce Results
 *     Limit [20]
 *         Sort [{2}: 1:0, 3:1]
 *             Project [a,b,c,d]
 *
 * Plan after optimization:
 * Produce Results
 *          TopN [{1:0, 3:1},{20}]
 *
 */
class LazyProjectTopN : public OptPass {
    static bool _IdentifyElementAndTopN(OpBase *root, Limit *&op_limit, Sort *&op_sort,
                                        Project *&op_project) {
        auto op = root;
        while (op->type != OpType::LIMIT) {
            if (op->children.size() != 1) return false;
            op = op->children[0];
        }
        op_limit = dynamic_cast<Limit *>(op);
        op = op->children[0];
        if (op->type != OpType::SORT || op->children.size() != 1) return false;
        op_sort = dynamic_cast<Sort *>(op);
        op = op->children[0];
        if (op->type != OpType::PROJECT || op->children.size() != 1) return false;
        op_project = dynamic_cast<Project *>(op);
        if (op_project->return_elements_.size() == op_sort->sort_items_.size()) return false;
        return true;
    }

    void _AdjustProjectOrder(OpBase *root) {
        Limit *op_limit = nullptr;
        Sort *op_sort = nullptr;
        Project *op_project = nullptr;
        if (!_IdentifyElementAndTopN(root, op_limit, op_sort, op_project)) {
            return;
        }
        // add topN
        auto op_topn =
            new TopN(op_sort->sort_items_, op_limit->limit_, op_project->return_elements_);
        auto op_post = op_limit->parent;
        op_topn->parent = op_post;
        auto op_pre = op_project->children[0];
        op_sort->RemoveChild(op_project);
        op_limit->RemoveChild(op_sort);
        op_post->RemoveChild(op_limit);
        delete op_project;
        delete op_sort;
        delete op_limit;
        op_project = nullptr;
        op_sort = nullptr;
        op_limit = nullptr;
        op_post->AddChild(op_topn);
        op_topn->AddChild(op_pre);
        LOG_DEBUG() << "the stream:";
        std::string s;
        OpBase::DumpStream(op_post, 0, true, s);
        LOG_DEBUG() << s;
    }

 public:
    LazyProjectTopN() : OptPass(typeid(LazyProjectTopN).name()) {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        _AdjustProjectOrder(root);
        return 0;
    }
};
}  // namespace cypher
