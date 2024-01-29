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
 * Created on 7/14/21
 */
#pragma once

#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_expand_all.h"
#include "cypher/execution_plan/optimization/opt_pass.h"

namespace cypher {
/*
 * EdgeFilterPushdownExpand:
 * MATCH (n)-[hascreator:postHasCreator]->(m) WHERE hascreator.creationDate < 20111217 RETURN n;
 * MATCH (n)-[hascreator:postHasCreator]->(m) WHERE hascreator.creationDate < 20111217 AND m.id >
 * 1000 RETURN n;
 *
 * Plan before optimization:
 *      Filter [{edge.condition < int64}]
 *          Expand(All) [a <-- b]
 *
 * Plan after optimization:
 *          Expand(All) [a <-- b, EdgeFilter (edge.condition < int64)]
 */
class EdgeFilterPushdownExpand : public OptPass {
    void _AddEdgeFilterOp(OpFilter *&op_filter, ExpandAll *&op_expandall) {
        // add edge_filter
        auto filter = op_filter->filter_;
        op_expandall->PushDownEdgeFilter(filter);
        auto op_post = op_filter->parent;
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

    bool _FindEdgeFilter(OpBase *root, OpFilter *&op_filter, ExpandAll *&op_expandall) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            op->children[0]->type == OpType::EXPAND_ALL) {
            op_filter = dynamic_cast<OpFilter *>(op);
            op_expandall = dynamic_cast<ExpandAll *>(op->children[0]);
            // if filter on edge
            std::string edge_alias = op_expandall->relp_->Alias();
            std::string src_alias = op_expandall->start_->Alias();
            std::string neigh_alias = op_expandall->neighbor_->Alias();

            std::set<std::string> ret = op_filter->filter_->Alias();
            // adjust edge filter when filter tree contains sub-filter whose alias are completed
            // contained by {src_alias, edge_alias} e.g. (ns)-[r]->(ne): sub-filter has alias only
            // have ns, r or both.
            //   `ns.name = r.role AND r.role = ne.name`, `r.role = "Iron Man"`....
            //
            // If filter contains binary type logical op (i.e AND, OR, XOR...), only care about AND
            // case. It will split filter when containing other binary type logical op.
            if (op_filter->filter_->ContainAlias({src_alias, edge_alias}) &&
                op_filter->filter_->BinaryOnlyContainsAND()) {
                if (ret.size() != 1) {
                    // if filter has both edge_filter and node_filter,split filters
                    auto clone_filter = op_filter->filter_->Clone();

                    // collect filters which only contains neigh_alias
                    // neigh_alias, neigh_alias + edge_alias, neigh_alias + src_node_alias
                    clone_filter->RemoveFilterWhen(clone_filter,
                                                   [&neigh_alias](const auto &b, const auto &e) {
                                                       for (auto it = b; it != e; it++) {
                                                           if (*it == neigh_alias) return false;
                                                       }
                                                       return true;
                                                   });

                    // collect filter which not contain neigh_alias
                    // src_node_alias, src_node_alias + edge_alias, edge_alias
                    op_filter->filter_->RemoveFilterWhen(
                        op_filter->filter_, [&neigh_alias](const auto &b, const auto &e) {
                            for (auto it = b; it != e; it++) {
                                if (*it == neigh_alias) return true;
                            }
                            return false;
                        });

                    // clone_filter or op_filter->filter will be nullptr after removing.
                    // e.g. for case `r.role = "Iron Man"`
                    //  init state: op_filter->filter_: `r.role = "Iron Man"`
                    //  after removing: clone_filter: nullptr, op_filter->filter_: `r.role = "Iron
                    //  Man"`
                    //
                    // e.g. for case `r.role = "ne.name"`
                    //  init state: op_filter->filter_:  `r.role = "ne.name"`
                    //  after removing: clone_filter:  `r.role = "ne.name"`, op_filter->filter_:
                    //  nullptr
                    //
                    // split into two filters only when both are not nullptr
                    if (clone_filter && op_filter->filter_) {
                        auto op_node_filter = new OpFilter(clone_filter);
                        // The RemoveChild(child) operation should be performed before the
                        // AddChild(child) operation, otherwise the child->parent will be set to
                        // nullptr
                        op_filter->RemoveChild(op_filter->children[0]);
                        op_node_filter->AddChild(op_filter->children[0]);
                        op_filter->AddChild(op_node_filter);
                    }
                }
                return true;
            }
        }
        for (auto child : op->children) {
            if (_FindEdgeFilter(child, op_filter, op_expandall)) return true;
        }
        return false;
    }

    void _AdjustFilter(OpBase *root) {
        ExpandAll *op_expandall = nullptr;
        OpFilter *op_filter = nullptr;
        // traverse the query execution plan to judge whether edge_filter exists
        while (_FindEdgeFilter(root, op_filter, op_expandall)) {
            _AddEdgeFilterOp(op_filter, op_expandall);
        }
    }

 public:
    EdgeFilterPushdownExpand() : OptPass(typeid(EdgeFilterPushdownExpand).name()) {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        _AdjustFilter(root);
        return 0;
    }
};
}  // namespace cypher
