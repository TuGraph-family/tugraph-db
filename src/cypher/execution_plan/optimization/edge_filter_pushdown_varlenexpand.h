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
 * Created by bxj on 3/25/23.
 */
#pragma once

#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_var_len_expand.h"
#include "cypher/execution_plan/optimization/opt_pass.h"

namespace cypher {
/*
 * EdgeFilterPushdownVarLenExpand:
 * MATCH p=(src:Account)-[e:transfer*1..3]->(dst:Account) WHERE
 * isAsc(getMemberProp(e,'timestamp'))=true
 *
 * Plan before optimization:
 * Filter [{isasc(false,getmemberprop(false,e1,timestamp)) = true}]
 *      Variable Length Expand(All) [acc -->*1..3 dst]
 *
 * Plan after optimization:
 *      Variable Length Expand(All) [acc -->*1..3 dst VarLenEdgeFilter
 * {isasc(false,getmemberprop(false,e1,timestamp)) = true}]
 */

class EdgeFilterPushdownVarLenExpand : public OptPass {
    void _AddEdgeFilterOp(OpFilter *&op_filter, VarLenExpand *&op_varlenexpand) {
        // op_post -> op_filter -> op_no_edge_filter -> op_varlenexpand
        auto filter = op_filter->filter_;
        op_varlenexpand->PushDownEdgeFilter(filter);
        auto op_post = op_filter->parent;
        // find the place of op_filter, then replace it by op_filter->children[0]
        for (auto i = op_post->children.begin(); i != op_post->children.end(); i++) {
            if (*i == op_filter) {
                op_post->RemoveChild(op_filter);
                op_post->InsertChild(i, op_filter->children[0]);
                delete op_filter;
                op_filter = nullptr;
                break;
            }
        }
    }

    bool _FindEdgeFilter(OpBase *root, OpFilter *&op_filter, VarLenExpand *&op_varlenexpand) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            op->children[0]->type == OpType::VAR_LEN_EXPAND) {
            op_filter = dynamic_cast<OpFilter *>(op);
            op_varlenexpand = dynamic_cast<VarLenExpand *>(op->children[0]);
            // if exist filter on varlenexpand edge
            std::string edge_alias = op_varlenexpand->relp_->Alias();
            if (op_filter->filter_->ContainAlias({edge_alias}) &&
                op_filter->filter_->BinaryOnlyContainsAND()) {
                // if filter has edge_filter, split filters
                auto clone_filter = op_filter->filter_->Clone();

                // collect filters which only contain edge_alias
                // e.g. head(getMemberProp(e2, 'timestamp')) > 1662123596189
                op_filter->filter_->RemoveFilterWhen(op_filter->filter_,
                                                     [&edge_alias](const auto &b, const auto &e) {
                                                         for (auto it = b; it != e; it++) {
                                                             if (*it == edge_alias) return false;
                                                         }
                                                         return true;
                                                     });

                // collect filter which not contain edge_alias
                // e.g. {dst.id = 4687403336918373745}
                clone_filter->RemoveFilterWhen(clone_filter,
                                               [&edge_alias](const auto &b, const auto &e) {
                                                   for (auto it = b; it != e; it++) {
                                                       if (*it == edge_alias) return true;
                                                   }
                                                   return false;
                                               });

                // split into two filters, when both are not nullpter
                if (clone_filter && op_filter->filter_) {
                    // op_filter -> op_no_edge_filter -> varlenexpand
                    auto op_no_edge_filter = new OpFilter(clone_filter);
                    op_no_edge_filter->AddChild(op_filter->children[0]);
                    op_filter->RemoveChild(op_filter->children[0]);
                    op_filter->AddChild(op_no_edge_filter);
                }
                return true;
            }
        }
        for (auto child : op->children) {
            if (_FindEdgeFilter(child, op_filter, op_varlenexpand)) return true;
        }
        return false;
    }

    void _AdjustFilter(OpBase *root) {
        OpFilter *op_filter = nullptr;
        VarLenExpand *op_varlenexpand = nullptr;
        // traverse the query execution plan to judge whether edge_filter exists
        while (_FindEdgeFilter(root, op_filter, op_varlenexpand)) {
            _AddEdgeFilterOp(op_filter, op_varlenexpand);
        }
    }

 public:
    EdgeFilterPushdownVarLenExpand() : OptPass(typeid(EdgeFilterPushdownVarLenExpand).name()) {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        _AdjustFilter(root);
        return 0;
    }
};
}  // namespace cypher