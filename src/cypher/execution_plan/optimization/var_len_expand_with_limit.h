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
// Created by wt on 21-07-01.
//
#pragma once

#include "cypher/execution_plan/optimization/opt_pass.h"

namespace cypher {

/*
 * // limit on paths
 * MATCH p = (n)-[*..5]->() RETURN p LIMIT 10
 * MATCH p = (n)-[*..5]->() RETURN properties(p) LIMIT 10
 * // limit on neighbors
 * MATCH (n)-[*..5]->(m) RETURN m LIMIT 10
 * MATCH (n)-[*..5]->(m) RETURN m.id,properties(m) LIMIT 10
 * // limit on n,m
 * MATCH (n)-[*..5]->(m) RETURN n,m LIMIT 10
 * // limit on constant
 * MATCH (n)-[*..5]->(m) RETURN 2021 LIMIT 10
 **/
class PassVarLenExpandWithLimit : public OptPass {
    static VarLenExpand *_LocateVarLenExpand(OpBase *op) {
        if (op->type == OpType::VAR_LEN_EXPAND) return dynamic_cast<VarLenExpand *>(op);
        for (auto c : op->children) {
            if (_LocateVarLenExpand(c)) return dynamic_cast<VarLenExpand *>(c);
        }
        return nullptr;
    }

    static Limit *_LocateLimit(OpBase *op) {
        switch (op->type) {
        case OpType::LIMIT:
            return dynamic_cast<Limit *>(op);
        case OpType::SORT:
        case OpType::FILTER:  // TODO(anyone) ignore irrelevant filters
        case OpType::AGGREGATE:
        case OpType::CARTESIAN_PRODUCT:
        case OpType::APPLY:
            return nullptr;
        default:
            if (op->parent) return _LocateLimit(op->parent);
        }
        return nullptr;
    }

    /* Checks if execution plan only performs this optimization. */
    static int _Identify(OpBase *root, Project *&op_project, Limit *&op_limit,
                         VarLenExpand *&op_var_len_expand) {
        op_var_len_expand = _LocateVarLenExpand(root);
        if (!op_var_len_expand) return 1;
        op_limit = _LocateLimit(op_var_len_expand);
        if (!op_limit) return 1;

        return 0;
    }

    void _DoPassVarLenExpandWithLimit(ExecutionPlan &plan) {
        Project *op_project = nullptr;
        Limit *op_limit = nullptr;
        VarLenExpand *op_var_len_expand = nullptr;
        if (_Identify(plan.Root(), op_project, op_limit, op_var_len_expand) != 0) {
            return;
        }
        // TODO(anyone) 20210701
    }

 public:
    PassVarLenExpandWithLimit() : OptPass(typeid(PassVarLenExpandWithLimit).name()) {}

    bool Gate() override { return true; }

    int Execute(ExecutionPlan *plan) override {
        _DoPassVarLenExpandWithLimit(*plan);
        return 0;
    }
};

}  // namespace cypher
