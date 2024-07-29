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
// Created by wt on 6/12/18.
//

#include <memory>
#include <stack>
#include "arithmetic/arithmetic_expression.h"
#include "db/galaxy.h"

#include "execution_plan/ops/op.h"
#include "graph/common.h"
#include "graph/graph.h"
#include "cypher/cypher_exception.h"
#include "ops/ops.h"
#include "monitor/memory_monitor_allocator.h"
#include "optimization/pass_manager.h"
#include "procedure/procedure.h"
#include "cypher/execution_plan/validation/graph_name_checker.h"
#include "cypher/execution_plan/execution_plan.h"
#include "server/bolt_session.h"

namespace cypher {
using namespace parser;
using namespace lgraph_log;
static void BuildQueryGraph(const QueryPart &part, PatternGraph &graph) {
    graph.symbol_table = part.symbol_table;
    /* Introduce nodes & relationships from MATCH pattern. */
    if (part.match_clause) {
        auto &pattern = std::get<0>(*part.match_clause);
        for (auto &pattern_part : pattern) {
            auto &pattern_element = std::get<1>(pattern_part);
            auto &node_pattern = std::get<0>(pattern_element);
            auto &pattern_element_chains = std::get<1>(pattern_element);
            NodeID curr, prev;
            prev = graph.BuildNode(node_pattern, Node::MATCHED);
            for (auto &chain : pattern_element_chains) {
                auto &relp_patn = std::get<0>(chain);
                auto &node_patn = std::get<1>(chain);
                curr = graph.BuildNode(node_patn, Node::MATCHED);
                graph.BuildRelationship(relp_patn, prev, curr, Relationship::MATCHED);
                prev = curr;
            }
        }
    }
    /* Introduce argument nodes & relationships */
    // TODO(anyone) revisit when arguments also in MATCH
    int invisible_node_idx = 0;
    for (auto &a : graph.symbol_table.symbols) {
        if (a.second.scope == SymbolNode::ARGUMENT) {
            if (a.second.type == SymbolNode::NODE && graph.GetNode(a.first).Empty()) {
                graph.AddNode("", a.first, Node::ARGUMENT);
            } else if (a.second.type == SymbolNode::RELATIONSHIP &&
                       graph.GetRelationship(a.first).Empty()) {
                auto src_alias = std::string(INVISIBLE).append("NODE_").append(
                    std::to_string(invisible_node_idx++));
                auto dst_alias = std::string(INVISIBLE).append("NODE_").append(
                    std::to_string(invisible_node_idx++));
                auto src_nid = graph.AddNode("", src_alias, Node::ARGUMENT);
                auto dst_nid = graph.AddNode("", dst_alias, Node::ARGUMENT);
                graph.AddRelationship(std::set<std::string>{}, src_nid, dst_nid,
                                      parser::LinkDirection::UNKNOWN, a.first,
                                      Relationship::ARGUMENT, {});
                auto &src_node = graph.GetNode(src_nid);
                auto &dst_node = graph.GetNode(dst_nid);
                src_node.Visited() = true;
                dst_node.Visited() = true;
            }
        }
    }
    // TODO(anyone) only build named entities
    /* Introduce nodes & relationships from CREATE/MERGE pattern. */
    for (auto &c : part.clauses) {
        if (c.type == Clause::CREATE) {
            for (auto &pattern_part : c.GetCreate()) {
                auto &pattern_element = std::get<1>(pattern_part);
                auto &node_pattern = std::get<0>(pattern_element);
                auto &pattern_element_chains = std::get<1>(pattern_element);
                auto &var = std::get<0>(node_pattern);
                const auto &node = graph.GetNode(var);
                if (!node.Empty() && pattern_element_chains.empty()) {
                    throw lgraph::EvaluationException("`" + var + "` already declared.");
                }
                NodeID curr, prev;
                prev = graph.BuildNode(node_pattern, Node::CREATED);
                for (auto &chain : pattern_element_chains) {
                    auto &relp_patn = std::get<0>(chain);
                    auto &node_patn = std::get<1>(chain);
                    curr = graph.BuildNode(node_patn, Node::CREATED);
                    graph.BuildRelationship(relp_patn, prev, curr, Relationship::CREATED);
                    prev = curr;
                }
            }
        } else if (c.type == Clause::MERGE) {
            auto &pattern_part = std::get<0>(c.GetMerge());
            auto &pattern_element = std::get<1>(pattern_part);
            auto &node_pattern = std::get<0>(pattern_element);
            auto &pattern_element_chains = std::get<1>(pattern_element);
            auto &var = std::get<0>(node_pattern);
            const auto &node = graph.GetNode(var);
            if (!node.Empty() && pattern_element_chains.empty()) {
                throw lgraph::EvaluationException("`" + var + "` already declared.");
            }
            NodeID curr, prev;
            prev = graph.BuildNode(node_pattern, Node::MERGED);
            for (auto &chain : pattern_element_chains) {
                auto &relp_patn = std::get<0>(chain);
                auto &node_patn = std::get<1>(chain);
                curr = graph.BuildNode(node_patn, Node::MERGED);
                graph.BuildRelationship(relp_patn, prev, curr, Relationship::MERGED);
                prev = curr;
            }
        } else if (c.type == Clause::INQUERYCALL) {
            const auto &call = c.GetCall();
            const auto &yield_items = std::get<2>(call);
            for (const auto &item : yield_items) {
                const auto &name = item.first;
                const auto &type = item.second;
                if (type == lgraph_api::LGraphType::NODE) {
                    TUP_PROPERTIES props = {Expression(), ""};
                    VEC_STR labels = {};
                    TUP_NODE_PATTERN node_pattern = {name, labels, props};
                    graph.BuildNode(node_pattern, Node::YIELD);
                }
            }
        }
    }  // for clauses
}

static void BuildResultSetInfo(const QueryPart &stmt, ResultInfo &result_info) {
    if (stmt.return_clause || stmt.with_clause) {
        bool distinct =
            stmt.return_clause ? std::get<0>(*stmt.return_clause) : std::get<0>(*stmt.with_clause);
        result_info.distinct = distinct;
        auto &ret_body =
            stmt.return_clause ? std::get<1>(*stmt.return_clause) : std::get<1>(*stmt.with_clause);
        auto &ret_items = std::get<0>(ret_body);
        auto &sort_items = std::get<1>(ret_body);
        auto &skip = std::get<2>(ret_body);
        auto &limit = std::get<3>(ret_body);
        for (auto &item : ret_items) {
            auto &e = std::get<0>(item);
            auto &alias = std::get<1>(item);
            bool isHidden = std::get<2>(item);
            if (isHidden) {
                continue;
            }
            ArithExprNode ae(e, stmt.symbol_table);
            bool aggregate = ae.ContainsAggregation();
            if (aggregate) result_info.aggregated = true;
            auto name = e.ToString(false);
            auto it = stmt.symbol_table.symbols.find(name);
            auto type = SymbolNode::CONSTANT;
            if (it == stmt.symbol_table.symbols.end()) {
                if (e.type == Expression::VARIABLE) {
                    throw lgraph::CypherException("Unknown variable: " + name);
                }
            } else {
                type = it->second.type;
            }
            switch (type) {
            case SymbolNode::CONSTANT:
            case SymbolNode::PARAMETER:
                result_info.header.colums.emplace_back(name, alias, aggregate,
                                                       lgraph_api::LGraphType::ANY);
                break;
            case SymbolNode::NODE:
                result_info.header.colums.emplace_back(name, alias, aggregate,
                                                       lgraph_api::LGraphType::NODE);
                break;
            case SymbolNode::RELATIONSHIP:
                result_info.header.colums.emplace_back(name, alias, aggregate,
                                                       lgraph_api::LGraphType::RELATIONSHIP);
                break;
            case SymbolNode::NAMED_PATH:
                result_info.header.colums.emplace_back(name, alias, aggregate,
                                                       lgraph_api::LGraphType::PATH);
                break;
            default:
                throw lgraph::CypherException("Unknown type: " + SymbolNode::to_string(type));
                break;
            }
        }
        result_info.ordered = !sort_items.empty();
        result_info.sort_items = sort_items;
        if (skip.type == parser::Expression::NA) {
            result_info.skip = -1;
        } else if (skip.type != parser::Expression::INT) {
            throw lgraph::CypherException(
                "It is not allowed to refer to expression "
                "other than an integer in SKIP: " +
                skip.ToString());
        } else if (skip.Int() < 0) {
            throw lgraph::CypherException("Invalid input, expect a positive integer in SKIP: " +
                                          skip.ToString());
        } else {
            result_info.skip = skip.Int();
        }
        if (limit.type == parser::Expression::NA) {
            result_info.limit = -1;
        } else if (limit.type != parser::Expression::INT) {
            throw lgraph::CypherException(
                "It is not allowed to refer to expression "
                "other than an integer in LIMIT: " +
                limit.ToString());
        } else if (limit.Int() < 0) {
            throw lgraph::CypherException("Invalid input, expect a positive integer in LIMIT: " +
                                          limit.ToString());
        } else {
            result_info.limit = limit.Int();
        }
    } else if (stmt.sa_call_clause) {
        // WARNING: Plugin header will be reset in runtime. Procedure's header is set now.
        //          We will set all header in the same place in next or more version.
        auto &procedure_name = std::get<0>(*stmt.sa_call_clause);
        auto p = global_ptable.GetProcedure(procedure_name);
        if (p == nullptr) {
            result_info.header.colums.emplace_back(procedure_name);
        } else {
            auto &yield_items = std::get<2>(*stmt.sa_call_clause);
            auto &result = p->signature.result_list;
            if (yield_items.empty()) {
                for (auto &r : result) {
                    result_info.header.colums.emplace_back(r.name, r.name, false, r.type);
                }
            } else {
                for (auto &yield_item : yield_items) {
                    for (auto &r : result) {
                        if (yield_item.first == r.name) {
                            result_info.header.colums.emplace_back(yield_item.first,
                                                                   yield_item.first, false, r.type);
                            break;
                        }
                    }
                }
            }
        }
    } else if (!stmt.create_clause.empty() || stmt.delete_clause ||
               !stmt.merge_clause.empty() || !stmt.set_clause.empty()) {
        CYPHER_THROW_ASSERT(result_info.header.colums.empty());
        result_info.header.colums.emplace_back(
            "<SUMMARY>", "", false, lgraph_api::LGraphType::STRING);
    }
}

static OpBase *_LocateOp(OpBase *root, OpType type) {
    if (!root) return nullptr;
    if (root->type == type) return root;
    for (auto child : root->children) {
        auto op = _LocateOp(child, type);
        if (op) return op;
    }
    return nullptr;
}

// Locate the only tap of stream
static OpBase *_LocateTap(OpBase *root) {
    if (!root || root->children.size() > 1) return nullptr;
    if (root->children.empty()) return root;
    return _LocateTap(root->children[0]);
}

// Locates all "taps" (entry points) of root.
static void _StreamTaps(OpBase *root, std::vector<OpBase *> &taps) {
    if (!root->children.empty()) {
        for (auto child : root->children) _StreamTaps(child, taps);
    } else {
        taps.emplace_back(root);
    }
}

// Connect ops into a single branch stream.
static OpBase *_SingleBranchConnect(const std::vector<OpBase *> &ops) {
    if (ops.empty()) return nullptr;
    OpBase *child, *parent = ops[0];
    for (int i = 1; i < (int)ops.size(); i++) {
        child = ops[i];
        parent->AddChild(child);
        parent = child;
    }
    return ops[0];
}

static void _UpdateStreamRoot(OpBase *new_root, OpBase *&root) {
    if (root) {
        /* The new root should have no parent, but may have children if we've constructed
         * a chain of traversals/scans. */
        CYPHER_THROW_ASSERT(!root->parent && !new_root->parent);
        /* Find the deepest child of the new root operation.
         * Currently, we can only follow the first child.
         * TODO(anyone) This may be inadequate later. */
        OpBase *tail = new_root;
        while (!tail->children.empty()) {
            if (tail->children.size() > 1 || tail->type == OpType::CARTESIAN_PRODUCT ||
                tail->type == OpType::APPLY) {
                CYPHER_TODO();
            }
            tail = tail->children[0];
        }
        // Append the old root to the tail of the new root's chain.
        tail->AddChild(root);
    }
    root = new_root;
}

void ExecutionPlan::_AddScanOp(const parser::QueryPart &part, const SymbolTable *sym_tab,
                               Node *node, std::vector<OpBase *> &ops, bool skip_arg_op) {
    auto it = sym_tab->symbols.find(node->Alias());
    if (it == sym_tab->symbols.end()) {
        throw lgraph::CypherException("Unknown variable: " + node->Alias());
    }
    auto &pf = node->Prop();
    OpBase *scan_op = nullptr;
    bool has_arg = false;
    for (auto &a : sym_tab->symbols) {
        if (a.second.scope == SymbolNode::ARGUMENT) {
            has_arg = true;
            break;
        }
    }
    if (!has_arg) {
        // no argument type in symbol table
        if (pf.type == Property::VALUE || pf.type == Property::PARAMETER) {
            /* use index when possible. weak index lookup if label absent */
            scan_op = new NodeIndexSeek(node, sym_tab);
        } else if (pf.type == Property::VARIABLE) {
            /* UNWIND [1,2] AS x MATCH (n {id:x}) RETURN n */
            scan_op = new NodeIndexSeekDynamic(node, sym_tab);
            if (sym_tab->symbols.find(pf.value_alias) == sym_tab->symbols.end()) {
                throw lgraph::CypherException("Unknown variable: " + pf.value_alias);
            }
            if (!part.unwind_clause) CYPHER_TODO();
        }
        if (!scan_op) {
            if (!node->Label().empty()) {
                /* labeled */
                scan_op = new NodeByLabelScan(node, sym_tab);
            } else {
                /* Node not labeled, no other option but a full scan. */
                scan_op = new AllNodeScan(node, sym_tab);
            }
        }
        ops.emplace_back(scan_op);
    } else {
        // argument type exists in symbol table
        if (it->second.scope == SymbolNode::ARGUMENT) {
            if (skip_arg_op) return;
            scan_op = new Argument(sym_tab);
        } else if (pf.type == Property::VALUE || pf.type == Property::PARAMETER) {
            /* use index when possible. weak index lookup if label absent */
            scan_op = new NodeIndexSeekDynamic(node, sym_tab);
            auto argument = new Argument(sym_tab);
            ops.emplace_back(argument);
        } else if (pf.type == Property::VARIABLE) {
            scan_op = new NodeIndexSeekDynamic(node, sym_tab);
            /* WITH 'sth' AS x MATCH (n {name:x}) RETURN n  */
            /* WITH {a: 'sth'} AS x MATCH (n {name:x.a}) RETURN n  */
            auto i = sym_tab->symbols.find(pf.value_alias);
            if (i == sym_tab->symbols.end())
                throw lgraph::CypherException("Unknown variable: " + pf.value_alias);
            if (i->second.scope == SymbolNode::ARGUMENT) {
                auto arg = new Argument(sym_tab);
                ops.emplace_back(arg);
            } else if (i->second.scope == SymbolNode::DERIVED_ARGUMENT) {
                /* WITH [] AS y UNWIND y AS x MATCH (n {id:x}) RETURN n
                 * The plan fragment should be sth. like this:
                 *   indexseek (n)
                 *     unwind (x)
                 *       argument (y)
                 * We add the argument operation earlier in buildUnwind.
                 */
                if (!part.unwind_clause) CYPHER_TODO();
            } else {
                if (!part.unwind_clause) CYPHER_TODO();
            }
        }
        if (!scan_op) {
            if (!node->Label().empty()) {
                scan_op = new NodeByLabelScanDynamic(node, sym_tab);
            } else {
                /* Node not labeled, no other option but a full scan. */
                scan_op = new AllNodeScanDynamic(node, sym_tab);
            }
            auto argument = new Argument(sym_tab);
            ops.emplace_back(argument);
        }
        ops.emplace_back(scan_op);
    }
}

#if 0
    void SglExecutionPlan::_HandleJoinHints(const std::vector<std::string> &join_hints) {
        /* USING JOIN ON hint:
         * 1. -->()<--      expand all () expand into
         * 2. -->()-->      expand into () reversed expand all
         * 3. <--()<--      reversed expand all () expand into
         * 4. <--()-->      ignore hint  */
        for (auto &hint : join_hints) {
            auto &node = _pattern_graph.GetNode(hint);
            if (node.Empty()) continue;
            if (node.InDegree() == 2 && node.OutDegree() == 0) {
                _MergeNodes(&node);
            } else if (node.InDegree() == 1 && node.OutDegree() == 1) {
                OpBase *in = nullptr;
                OpBase *out = nullptr;
                std::vector<OpBase *> nodes_to_visit;
                nodes_to_visit.push_back(_root);
                while (!nodes_to_visit.empty()) {
                    auto current = nodes_to_visit.back();
                    nodes_to_visit.pop_back();
                    if (current->operation->type == OpType::EXPAND_ALL) {
                        auto op = std::static_pointer_cast<ExpandAll>(current->operation);
                        if (op->_dst == &node) in = current;
                        if (op->_src == &node) out = current;
                    } else if (current->operation->type == OpType::REVERSED_EXPAND_ALL) {
                        auto op = std::static_pointer_cast<ReversedExpandAll>(current->operation);
                        if (op->_dst == &node) in = current;
                        if (op->_src == &node) out = current;
                    }
                    for (auto child : current->children) nodes_to_visit.push_back(child);
                }
                CYPHER_THROW_ASSERT(in != nullptr && out != nullptr);

                /* replace IN (expand all) to expand into */
                auto op = std::static_pointer_cast<ExpandAll>(in->operation);
                std::shared_ptr<OpBase> op_expand_into = std::make_shared<ExpandInto>(TxnRef(),
                        op->_src, op->_dst, op->_relp);
                in->operation = op_expand_into;
                OpBase *expand_into = in;

                // link OUT operation with expand into
                expand_into->AddChild(out);

                // expand into should inherit OUT's parents
                for (auto parent : out->parents) {
                    if (parent == expand_into) continue;
                    if (!parent->ContainChild(expand_into)) parent->AddChild(expand_into);
                    parent->RemoveChild(out);
                }
            } else if (node.InDegree() == 0 && node.OutDegree() == 2) {
                // just ignore
            } else {
                CYPHER_TODO();
            }
        }
    }
#endif

void ExecutionPlan::_BuildArgument(const parser::QueryPart &part,
                                   cypher::PatternGraph &pattern_graph, cypher::OpBase *&root) {
    /* Argument used in reading clauses will built in traversal, we only
     * build argument for pure updating clauses here.  */
    if (part.match_clause || part.unwind_clause) return;
    auto &sym_tab = pattern_graph.symbol_table;
    for (auto &a : sym_tab.symbols) {
        if (a.second.scope == SymbolNode::ARGUMENT) {
            auto argument = new Argument(&sym_tab);
            return _UpdateStreamRoot(argument, root);
        }
    }
}

void ExecutionPlan::_BuildStandaloneCallOp(const parser::QueryPart &part,
                                           const PatternGraph &pattern_graph, OpBase *&root) {
    /* If the cypher query is a StandaloneCall, these should be no
     * RegularQuery else.  */
    CYPHER_THROW_ASSERT(part.sa_call_clause && part.clauses.size() == 1 && !root);
    root = new StandaloneCall(&part);
}

static bool _SkipHangingArgumentOp(const PatternGraph &graph, const SymbolTable &sym_tab) {
    /* If any argument node is matched, the argument op will built during traversal,
     * so we should skip building redundant argument ops for hanging node.  */
    for (auto &s : sym_tab.symbols) {
        if (s.second.scope == SymbolNode::ARGUMENT && s.second.type == SymbolNode::NODE) {
            auto &n = graph.GetNode(s.first);
            if (n.derivation_ == Node::CREATED || n.derivation_ == Node::MERGED) continue;
            if (n.derivation_ == Node::MATCHED) return true;
        }
    }
    for (auto &n : graph.GetNodes()) {
        if (n.Prop().type == Property::VARIABLE) {
            auto it = sym_tab.symbols.find(n.Prop().value_alias);
            if (it != sym_tab.symbols.end() && (it->second.scope == SymbolNode::ARGUMENT ||
                                                it->second.scope == SymbolNode::DERIVED_ARGUMENT)) {
                return true;
            }
        }
    }
    return false;
}

// Build expand ops in DFS traversal order
void ExecutionPlan::_BuildExpandOps(const parser::QueryPart &part, PatternGraph &pattern_graph,
                                    OpBase *&root) {
    CYPHER_THROW_ASSERT(part.match_clause);
    /* Construct start nodes for traversal  */
    VEC_STR join_hints, start_hints;
    auto hints = std::get<1>(*part.match_clause);
    for (auto &hint : hints) {
        if (hint.substr(hint.length() - 2, 2) == "@J") {
            join_hints.emplace_back(hint.substr(0, hint.length() - 2));
        } else if (hint.substr(hint.length() - 2, 2) == "@S") {
            start_hints.emplace_back(hint.substr(0, hint.length() - 2));
        }
    }
    // TODO(botu.wzy): A better implementation of picking the starting node
    std::vector<NodeID> start_nodes;
    /* The argument nodes are specific, we add them into start nodes first.
     * If there are both specific node & argument in pattern, prefer the former.
     * e.g.
     * MATCH (a {name:'Dennis Quaid'}) WITH a,['London','Houston'] AS cids
     * UNWIND cids AS cid MATCH (c {name:cid})<-[]-(a) RETURN a,count(c)
     */
    for (auto &n : pattern_graph.GetNodes()) {
        auto &prop = n.Prop();
        if (n.derivation_ != Node::CREATED && prop.type == Property::VARIABLE) {
            auto it = pattern_graph.symbol_table.symbols.find(prop.value_alias);
            CYPHER_THROW_ASSERT(it != pattern_graph.symbol_table.symbols.end());
            start_nodes.emplace_back(n.ID());
        }
    }
    std::map<size_t, NodeID> args_ordered;
    for (auto &s : pattern_graph.symbol_table.symbols) {
        if (s.second.scope == SymbolNode::ARGUMENT && s.second.type == SymbolNode::NODE) {
            auto &n = pattern_graph.GetNode(s.first);
            if (!n.Empty()) args_ordered.emplace(s.second.id, n.ID());
        }
    }
    for (auto &a : args_ordered) start_nodes.emplace_back(a.second);
    for (auto &s : start_hints) start_nodes.emplace_back(pattern_graph.GetNode(s).ID());
    for (auto &n : pattern_graph.GetNodes()) {
        if (n.derivation_ == Node::MATCHED && !n.Label().empty() &&
            n.Prop().type == Property::VALUE) {
            start_nodes.emplace_back(n.ID());
        }
    }
    for (auto &n : pattern_graph.GetNodes()) {
        if (n.derivation_ == Node::MATCHED && !n.Label().empty()) {
            start_nodes.emplace_back(n.ID());
        }
    }
    for (auto &n : pattern_graph.GetNodes()) {
        if (n.derivation_ != Node::CREATED && n.derivation_ != Node::MERGED)
            start_nodes.emplace_back(n.ID());
    }
    bool skip_hanging_argument_op =
        _SkipHangingArgumentOp(pattern_graph, pattern_graph.symbol_table);
    auto expand_streams = pattern_graph.CollectExpandStreams(start_nodes, true);
    /* If we have multiple graph components, the root operation is a Cartesian Product.
     * Each chain of traversals will be a child of this op. */
    OpBase *traversal_root = nullptr;
    for (auto &stream : expand_streams) {
        std::vector<OpBase *> expand_ops;
        bool hanging = false;  // if the stream is a hanging node
        for (auto &step : stream) {
            auto &start = pattern_graph.GetNode(std::get<0>(step));
            auto &relp = pattern_graph.GetRelationship(std::get<1>(step));
            auto &neighbor = pattern_graph.GetNode(std::get<2>(step));
            if (relp.Empty() && neighbor.Empty()) {
                // neighbor and relationship are both empty, it's a hanging node
                /* Node doesn't have any incoming nor outgoing edges,
                 * this is an hanging node "()", create a scan operation. */
                CYPHER_THROW_ASSERT(stream.size() == 1);
                hanging = true;
                _AddScanOp(part, &pattern_graph.symbol_table, &start, expand_ops,
                           skip_hanging_argument_op);
                /* Skip all the rest hanging arguments after one is added.
                 * e.g. MATCH (a),(b) WITH a, b MATCH (c) RETURN a,b,c  */
                auto it = pattern_graph.symbol_table.symbols.find(start.Alias());
                if (it != pattern_graph.symbol_table.symbols.end() &&
                    it->second.scope == SymbolNode::ARGUMENT) {
                    // the previous argument op added
                    skip_hanging_argument_op =
                        true;  // avoid `MATCH (a),(b) WITH a, b a,b` generates multiple arguments
                }
            } else if (relp.VarLen()) {
                // expand when neighbor is not null
                OpBase *expand_op = new VarLenExpand(&pattern_graph, &start, &neighbor, &relp);
                expand_ops.emplace_back(expand_op);
            } else {
                OpBase *expand_op = new ExpandAll(&pattern_graph, &start, &neighbor, &relp);
                expand_ops.emplace_back(expand_op);
            }
            // add property filter op
            auto pf = neighbor.Prop();
            if (!pf.field.empty()) {
                ArithExprNode ae1, ae2;
                ae1.SetOperand(ArithOperandNode::AR_OPERAND_VARIADIC, neighbor.Alias(), pf.field,
                               pattern_graph.symbol_table);
                if (pf.type == Property::PARAMETER) {
                    // TODO(anyone) use record
                    ae2.SetOperand(ArithOperandNode::AR_OPERAND_PARAMETER,
                                   cypher::FieldData(lgraph::FieldData(pf.value_alias)));
                } else if (pf.type == Property::VARIABLE) {
                    ae2.SetOperandVariable(ArithOperandNode::AR_OPERAND_VARIABLE,
                                        pf.hasMapFieldName, pf.value_alias, pf.map_field_name);
                } else {
                    ae2.SetOperand(ArithOperandNode::AR_OPERAND_CONSTANT,
                                   cypher::FieldData(pf.value));
                }
                std::shared_ptr<lgraph::Filter> filter =
                    std::make_shared<lgraph::RangeFilter>(lgraph::CompareOp::LBR_EQ, ae1, ae2,
                                                        &pattern_graph.symbol_table);
                OpBase *filter_op = new OpFilter(filter);
                expand_ops.emplace_back(filter_op);
            }
        }  // end for steps
        /* Add optional match.
         * Do not add optional op if the expand ops is empty, e.g. when hanging argument. */
        if (part.match_clause && std::get<3>(*part.match_clause) && !expand_ops.empty()) {
            OpBase *optional = new Optional();
            expand_ops.emplace_back(optional);
        }
        /* Save expand ops in reverse order. */
        std::reverse(expand_ops.begin(), expand_ops.end());
        /* Locates expand all operations which do not have a child operation,
         * And adds a scan operation as a new child. */
        if (!hanging) {
            // add a scan op when it is not a hanging node
            CYPHER_THROW_ASSERT(!stream.empty());
            std::vector<OpBase *> scan_ops;
            auto &start_node = pattern_graph.GetNode(std::get<0>(stream[0]));
            _AddScanOp(part, &pattern_graph.symbol_table, &start_node, scan_ops, false);
            for (auto it = scan_ops.rbegin(); it != scan_ops.rend(); it++) {
                expand_ops.emplace_back(*it);
            }
        }
        if (!_SingleBranchConnect(expand_ops)) continue;
        if (!traversal_root) {
            // We've built the only necessary traversal chain
            traversal_root = expand_ops[0];
        } else {
            // We have multiple disjoint traversal chains.
            // Add each chain as a child under the Cartesian Product.
            if (traversal_root->type == OpType::CARTESIAN_PRODUCT) {
                traversal_root->AddChild(expand_ops[0]);
            } else {
                auto cartesian = new CartesianProduct();
                cartesian->AddChild(traversal_root);
                cartesian->AddChild(expand_ops[0]);
                traversal_root = cartesian;
            }
        }
    }  // end for streams
    if (!traversal_root) {
        // judge if nodes in stream are dangling and of type ARGUMENT when traversal_root is null
        // _AddScanOp will be skipped in this case. No op will be generated
        // 1. judge if ARGUMENT type exists in symbol table
        bool has_arg = false;
        for (auto &a : pattern_graph.symbol_table.symbols) {
            if (a.second.scope == SymbolNode::ARGUMENT) {
                has_arg = true;
                break;
            }
        }
        if (!has_arg) CYPHER_TODO();
        // 2. judge if all nodes are dangiling
        // all streams are dangling nodes
        for (auto &stream : expand_streams) {
            if (stream.size() != 1 ||
                !pattern_graph.GetRelationship(std::get<1>(stream[0])).Empty()) {
                CYPHER_TODO();
            }
        }
        // throw exception when argument or the stream is not dangling node
        // add one more argument when condition is true
        traversal_root = new Argument(&pattern_graph.symbol_table);
    }
    /* Restructure the tree to move the argument down
     * 1. remove cartesian product and emplace children[1] as the root
     * 2. update the scan op to the dynamic one
     * Only works when there are only two children of CARTESIAN_PRODUCT. E.g,
     * before：
     * Cartesian Product
            Argument [c,f]
            Expand(All) [film <-- p]
                Expand(All) [m --> film]
                    Node By Label Scan [m:Person]
     * after：
     * Expand(All) [film <-- p]
            Expand(All) [m --> film]
                Node By Label Scan Dynamic [m:Person]
                    Argument [c,f]
     **/
    if (traversal_root) {
        if (traversal_root->type == OpType::CARTESIAN_PRODUCT &&
            traversal_root->children[0]->type == OpType::ARGUMENT) {
            if (traversal_root->children.size() != 2) CYPHER_TODO();
            OpBase *new_root = traversal_root->children[1];
            traversal_root->RemoveChild(new_root);
            OpBase::FreeStream(traversal_root);
            traversal_root = new_root;
        }
    }

    if (root) {
        /* Only ReadingClauses may appear before, that is one of the following:
         * Match | Unwind | InQueryCall
         * ArgumentOp that built by no clause may also built before traversal. */
        if (root->type != OpType::UNWIND && root->type != OpType::ARGUMENT) CYPHER_TODO();
        if (traversal_root->type == OpType::CARTESIAN_PRODUCT) {
            std::vector<OpBase *> taps;
            _StreamTaps(traversal_root, taps);
            for (auto t : taps) {
                if (t->IsScan()) continue;
                if (!t->IsDynamicScan()) CYPHER_TODO();
                t->AddChild(root);
                root = traversal_root;
                return;
            }
        } else {
            auto tap = _LocateTap(traversal_root);
            if (tap->IsScan()) {
                auto cartesian = new CartesianProduct();
                cartesian->AddChild(traversal_root);
                traversal_root = cartesian;
            } else {
                if (!tap->IsDynamicScan()) CYPHER_TODO();
                return _UpdateStreamRoot(traversal_root, root);
            }
        }
        traversal_root->AddChild(root);
    }
    root = traversal_root;
}

/* UNWIND expands a list into a sequence of rows, common usages:
 *   Unwinding a list
 *   Creating a distinct list
 *   Using UNWIND with any expression returning a list
 *   Using UNWIND with a list of lists
 *   Using UNWIND with an empty list
 *   Using UNWIND with an expression that is not a list
 *   Creating nodes from a list parameter
 * Note that in the examples above, UNWIND is always the operation that provides
 * original data for the part (leaf op).  */
void ExecutionPlan::_BuildUnwindOp(const parser::QueryPart &part, const PatternGraph &pattern_graph,
                                   OpBase *&root) {
    CYPHER_THROW_ASSERT(part.unwind_clause);
    ArithExprNode exp(std::get<0>(*part.unwind_clause), pattern_graph.symbol_table);
    auto unwind = new Unwind(exp, std::get<1>(*part.unwind_clause), &pattern_graph.symbol_table);
    /* Add argument operation if the list is an argument. e.g.:
     * WITH [] AS y UNWIND y AS x MATCH (n {id:x}) RETURN n
     */
    auto &sym_tab = pattern_graph.symbol_table;
    auto it = sym_tab.symbols.find(std::get<1>(*part.unwind_clause));
    if (it != sym_tab.symbols.end() && it->second.scope == SymbolNode::DERIVED_ARGUMENT) {
        auto arg = new Argument(&sym_tab);
        unwind->AddChild(arg);
    }
    _UpdateStreamRoot(unwind, root);
}

/* CALL dbms.procedures() YIELD name, signature // Standalone Call
 * CALL dbms.procedures() YIELD name, signature RETURN signature    // In Query Call  */
void ExecutionPlan::_BuildInQueryCallOp(const parser::QueryPart &part,
                                        const PatternGraph &pattern_graph, OpBase *&root) {
    CYPHER_THROW_ASSERT(part.iq_call_clause);
    auto call_op = new InQueryCall(&pattern_graph, &part);
    _UpdateStreamRoot(call_op, root);
}

void ExecutionPlan::_BuildCreateOp(const parser::QueryPart &part,
                                   cypher::PatternGraph &pattern_graph, cypher::OpBase *&root) {
    OpBase *create = new OpCreate(&part, &pattern_graph);
    _UpdateStreamRoot(create, root);
}

void ExecutionPlan::_BuildMergeOp(const parser::QueryPart &part,
                                  cypher::PatternGraph &pattern_graph, cypher::OpBase *&root) {
    OpBase *merge = new OpMerge(&part, &pattern_graph);
    _UpdateStreamRoot(merge, root);
}

void ExecutionPlan::_BuildDeleteOp(const parser::QueryPart &part,
                                   cypher::PatternGraph &pattern_graph, cypher::OpBase *&root) {
    OpBase *del = new OpDelete(&part, &pattern_graph);
    _UpdateStreamRoot(del, root);
}

void ExecutionPlan::_BuildSetOp(const parser::QueryPart &part, cypher::PatternGraph &pattern_graph,
                                cypher::OpBase *&root) {
    OpBase *set = new OpSet(&part, &pattern_graph);
    _UpdateStreamRoot(set, root);
}

void ExecutionPlan::_BuildRemoveOp(const parser::QueryPart &part,
                                   cypher::PatternGraph &pattern_graph, cypher::OpBase *&root) {
    OpBase *remove = new OpRemove(&part, &pattern_graph);
    _UpdateStreamRoot(remove, root);
}

void ExecutionPlan::_BuildReturnOps(const parser::QueryPart &part,
                                    const cypher::PatternGraph &pattern_graph,
                                    cypher::OpBase *&root) {
    std::vector<OpBase *> ops;
    auto result = new ProduceResults();
    ops.emplace_back(result);
    if (_result_info.limit >= 0) {
        auto limit = new Limit(_result_info.limit);
        ops.emplace_back(limit);
    }
    if (_result_info.skip >= 0) {
        auto skip = new Skip(_result_info.skip);
        ops.emplace_back(skip);
    }
    if (_result_info.ordered) {
        auto sort = new Sort(_result_info.sort_items, _result_info.skip, _result_info.limit);
        ops.emplace_back(sort);
    }
    if (_result_info.aggregated) {
        auto op = new Aggregate(&part, &pattern_graph.symbol_table, _result_info.header);
        ops.emplace_back(op);
    } else {
        /* NOTE: Incase of aggregated query, there's no need to distinct check,
         * groups are already distinct by key. */
        if (_result_info.distinct) {
            auto distinct = new Distinct();
            ops.emplace_back(distinct);
        }
        auto op = new Project(&part, &pattern_graph.symbol_table);
        ops.emplace_back(op);
    }
    _SingleBranchConnect(ops);
    _UpdateStreamRoot(ops[0], root);
}

void ExecutionPlan::_BuildWithOps(const parser::QueryPart &part,
                                  const cypher::PatternGraph &pattern_graph,
                                  cypher::OpBase *&root) {
    // TODO(anyone) same as return op
    std::vector<OpBase *> ops;
    if (_result_info.limit >= 0) {
        auto limit = new Limit(_result_info.limit);
        ops.emplace_back(limit);
    }
    if (_result_info.skip >= 0) {
        auto skip = new Skip(_result_info.skip);
        ops.emplace_back(skip);
    }
    if (_result_info.ordered) {
        auto sort = new Sort(_result_info.sort_items, _result_info.skip, _result_info.limit);
        ops.emplace_back(sort);
    }
    if (_result_info.aggregated) {
        auto op = new Aggregate(&part, &pattern_graph.symbol_table, _result_info.header);
        ops.emplace_back(op);
    } else {
        /* NOTE: Incase of aggregated query, there's no need to distinct check,
         * groups are already distinct by key. */
        if (_result_info.distinct) {
            auto distinct = new Distinct();
            ops.emplace_back(distinct);
        }
        auto op = new Project(&part, &pattern_graph.symbol_table);
        ops.emplace_back(op);
    }
    _SingleBranchConnect(ops);
    _UpdateStreamRoot(ops[0], root);
}

void ExecutionPlan::_BuildClause(const parser::Clause &clause, const parser::QueryPart &part,
                                 PatternGraph &pattern_graph, OpBase *&root) {
    switch (clause.type) {
    case parser::Clause::STANDALONECALL:
        _BuildStandaloneCallOp(part, pattern_graph, root);
        break;
    case parser::Clause::MATCH:
        _BuildExpandOps(part, pattern_graph, root);
        break;
    case parser::Clause::UNWIND:
        _BuildUnwindOp(part, pattern_graph, root);
        break;
    case parser::Clause::INQUERYCALL:
        _BuildInQueryCallOp(part, pattern_graph, root);
        break;
    case parser::Clause::CREATE:
        // Only add at most one Create op, which will do all create clauses.
        if (!_LocateOp(root, OpType::CREATE)) {
            _BuildCreateOp(part, pattern_graph, root);
        }
        break;
    case parser::Clause::MERGE:
        if (!_LocateOp(root, OpType::MERGE)) {
            _BuildMergeOp(part, pattern_graph, root);
        }
        break;
    case parser::Clause::DELETE_:
        _BuildDeleteOp(part, pattern_graph, root);
        break;
    case parser::Clause::SET:
        if (!_LocateOp(root, OpType::UPDATE)) {
            _BuildSetOp(part, pattern_graph, root);
        }
        break;
    case parser::Clause::REMOVE:
        _BuildRemoveOp(part, pattern_graph, root);
        break;
    case parser::Clause::RETURN:
        _BuildReturnOps(part, pattern_graph, root);
        break;
    case parser::Clause::WITH:
        _BuildWithOps(part, pattern_graph, root);
        break;
    default:
        CYPHER_TODO();
    }
    if (&clause == &part.clauses.back()) {
        if (!part.return_clause && !part.with_clause) {
            auto result = new ProduceResults();
            _UpdateStreamRoot(result, root);
        }
    }
}

void ExecutionPlan::_PlaceFilterOps(const parser::QueryPart &part, OpBase *&root) {
    if (part.match_clause) {
        auto &where_expr = std::get<2>(*part.match_clause);
        if (where_expr.type == Expression::FILTER) {
            _PlaceFilter(where_expr.Filter(), root);
            _MergeFilter(root);
        }
    }
    if (part.with_clause) {
        auto &where_expr = std::get<2>(*part.with_clause);
        if (where_expr.type == Expression::FILTER) {
            _PlaceFilter(where_expr.Filter(), root);
            _MergeFilter(root);
        }
    }
    if (part.iq_call_clause) {
        auto &where_expr = std::get<3>(*part.iq_call_clause);
        if (where_expr.type == Expression::FILTER) {
            _PlaceFilter(where_expr.Filter(), root);
            _MergeFilter(root);
        }
    }
}

// if there are two adjacent filters in the op tree, merge them together
void ExecutionPlan::_MergeFilter(OpBase *&root) {
    if (root == nullptr) return;
    if (root->type == OpType::FILTER && root->children.size() == 1 &&
        root->children[0]->type == OpType::FILTER) {
        auto *filter = dynamic_cast<OpFilter *>(root);
        auto *child_filter = dynamic_cast<OpFilter *>(filter->children[0]);
        std::shared_ptr<lgraph::Filter> merged_filter(new lgraph::Filter(
            lgraph::LogicalOp::LBR_AND, filter->Filter(), child_filter->Filter()));
        OpBase *new_root = new OpFilter(merged_filter);
        for (auto child : root->children[0]->children) new_root->AddChild(child);
        new_root->parent = root->parent;
        root = new_root;
        delete filter;
        delete child_filter;
        _MergeFilter(root);
    } else {
        for (auto &child : root->children) _MergeFilter(child);
    }
}

// TODO(anyone) filter simplification
void ExecutionPlan::_PlaceFilter(std::shared_ptr<lgraph::Filter> f, OpBase *&root) {
    if (f == nullptr) return;
    /* if filter contains no alias, do filter as soon as possible.
     * a. constant filter: where 1 = 2
     * b. invalid args: where type(4) = 'ACTED_IN'  */
    if (f->Alias().empty()) {
        // find the leaf op
        OpBase *leaf_op = root;
        while (!leaf_op->children.empty()) {
            leaf_op = leaf_op->children[0];
        }
        OpBase *node_filter = new OpFilter(f);
        leaf_op->parent->PushInBetween(node_filter);
    } else {
        // If a filter's LogicalOp is AND, do its subtrees separately
        if (f->LogicalOp() == lgraph::LogicalOp::LBR_AND) {
            _PlaceFilter(f->Left(), root);
            _PlaceFilter(f->Right(), root);
        } else {
            if (!_PlaceFilterToNode(f, root)) {
                if (f->ContainAlias(root->modifies)) {
                    /* NOTE: Re-align the alias_id_map for this filter later.  */
                    OpBase *opf = new OpFilter(f);
                    opf->AddChild(root);
                    root = opf;
                } else {
                    LOG_WARN() << "ignored filter: " << f->ToString();
                }
            }
        }
    }
}

static std::vector<std::string> GetModifiesForNode(OpBase *node) {
    std::vector<std::string> seen;
    if (!node->modifies.empty())
        seen.insert(seen.end(), node->modifies.begin(), node->modifies.end());
    for (auto rit = node->children.rbegin(); rit != node->children.rend(); ++rit) {
        auto saw = GetModifiesForNode(*rit);
        seen.insert(seen.end(), saw.begin(), saw.end());
    }
    return seen;
}

bool ExecutionPlan::_PlaceFilterToNode(std::shared_ptr<lgraph::Filter> &f, OpBase *node) {
    // if f is not successfully placed, return false
    if (node == nullptr && f != nullptr) return false;
    // check if rit modifies at least one of f's aliases
    bool containModifies = false;
    for (auto rit = node->children.rbegin(); rit != node->children.rend(); ++rit) {
        for (auto alias : f->Alias())
            if (std::find((*rit)->modifies.begin(), (*rit)->modifies.end(), alias) !=
                (*rit)->modifies.end()) {
                containModifies = true;
                break;
            }
    }
    // check if the subtree of node modifies all of f's aliases
    auto mod = GetModifiesForNode(node);
    bool allModified = true;
    for (auto &a : f->Alias())
        if (std::find(mod.begin(), mod.end(), a) == mod.end()) {
            allModified = false;
            break;
        }
    // do the filter at rit if containModifies and allModified are true
    if (containModifies && allModified) {
        OpBase *node_filter = new OpFilter(f);
        OpBase *insert = node, *current = node;
        while (current) {
            if (current->children.size() == 1) {
                insert = current;
                break;
            }
            current = current->parent;
        }
        insert->PushInBetween(node_filter);
        return true;
    } else {
        // if this filter cannot be placed at rit, try to place this filter
        // to its children
        for (auto rit = node->children.rbegin(); rit != node->children.rend(); ++rit) {
            if (_PlaceFilterToNode(f, *rit)) {
                return true;
            }
        }
    }
    return false;
}

OpBase *ExecutionPlan::BuildPart(const parser::QueryPart &part, int part_id) {
    OpBase *segment_root = nullptr;
    /* init alias id map */
    auto &pattern_graph = _pattern_graphs[part_id];
    std::string dummy;
    _read_only = _read_only && part.ReadOnly(dummy, dummy);

    /* build the pattern graph */
    BuildQueryGraph(part, pattern_graph);

    /* Build result_set info (header etc.) for current part */
    _result_info = ResultInfo();
    BuildResultSetInfo(part, _result_info);

    _BuildArgument(part, pattern_graph, segment_root);
    for (auto &clause : part.clauses) {
        _BuildClause(clause, part, pattern_graph, segment_root);
    }

    _PlaceFilterOps(part, segment_root);

    return segment_root;
}

static inline void _StreamArgs(OpBase *root, std::vector<OpBase *> &args) {
    if (root->type == OpType::ARGUMENT) args.emplace_back(root);
    for (auto child : root->children) _StreamArgs(child, args);
}

static void _RealignAliasId(OpBase *root, const SymbolTable &sym_tab) {
    OpBase *op = root;
    while (op) {
        switch (op->type) {
        case OpType::PROJECT:
        case OpType::AGGREGATE:
            return;
        case OpType::FILTER:
            dynamic_cast<OpFilter *>(op)->Filter()->RealignAliasId(sym_tab);
            break;
        case OpType::CARTESIAN_PRODUCT:
        case OpType::APPLY:
            return;
        case OpType::DISTINCT:
        case OpType::SORT:
        case OpType::SKIP:
        case OpType::LIMIT:
            break;
        default:
            CYPHER_TODO();
        }
        CYPHER_THROW_ASSERT(op->children.size() == 1);
        op = op->children[0];
    }
}

static OpBase *_Connect(OpBase *lhs, OpBase *rhs, PatternGraph *pattern_graph) {
    std::vector<OpBase *> taps;
    _StreamTaps(rhs, taps);
    //
    // see issue #357: https://code.alipay.com/fma/tugraph-db/issues/357
    // see issue #188: https://code.alipay.com/fma/tugraph-db/issues/188
    //
    // # Example A
    // WITH 'a' as a UNWIND ['a', 'b'] as k RETURN a, k
    //
    // Execution Plan:
    // Produce Results
    //     Project [a,k]
    //         Cartesian Product
    //             Unwind [[a,b],k]
    //             Project [a]
    //
    // The execution plan before issue#357&#188 fix is as follows:
    //
    // Plan parts:
    // Project [a]
    // Produce Results
    //     Project [a,k]
    //         Unwind [[a,b],k]
    //
    // -----
    // # Example B
    // WITH [1, 3, 5, 7] as lst UNWIND [9]+lst AS x RETURN x, lst
    //
    // Execution Plan:
    // Produce Results
    //     Project [x,lst]
    //         Apply
    //             Unwind [([9],lst,+),x]
    //                 Argument [lst]
    //             Project [lst]
    //
    // The execution plan before issue#357&#188 fix is as follows:
    //
    // Execution Plan:
    // Produce Results
    //     Project [x,lst]
    //         Unwind [([9],lst,+),x]
    //             Project [lst]
    //
    if (taps.size() == 1 && !taps[0]->IsScan() && taps[0]->type != OpType::UNWIND) {
        /* Single tap, entry point isn't a SCAN operation, e.g.
         * MATCH (b) WITH b.v AS V RETURN V
         * MATCH (b) WITH b.v+1 AS V CREATE (n {v:V})
         * MATCH (b) WITH b RETURN b */
        if (taps[0]->type == OpType::PROJECT && lhs->type == OpType::PROJECT) {
            auto parent = taps[0]->parent;
            parent->RemoveChild(taps[0]);
            parent->AddChild(lhs);
            delete taps[0];
        } else {
            taps[0]->AddChild(lhs);
        }
    } else {
        /* Multiple taps or a single SCAN tap, e.g.
         * MATCH (b) WITH b.v AS V MATCH (c) return V,c
         * MATCH (b) WITH b.v AS V MATCH (c),(d) return c, V, d */
        OpBase *connection = nullptr;
        std::vector<OpBase *> args;  // argument operations
        _StreamArgs(rhs, args);
        if (!args.empty()) {
            if (args.size() > 1) CYPHER_TODO();
            connection = new Apply(dynamic_cast<Argument *>(args[0]), pattern_graph);
        } else {
            connection = new CartesianProduct();
        }
        OpBase *stream_root = rhs;
        // find the *root* project/aggregate op
        while (!stream_root->IsStreamRoot()) {
            if (stream_root->children.size() != 1) CYPHER_TODO();
            stream_root = stream_root->children[0];
        }
        CYPHER_THROW_ASSERT(stream_root->IsStreamRoot());
        stream_root->AddChild(lhs);
        stream_root->PushInBetween(connection);
    }
    return rhs;
}

bool ExecutionPlan::_WorkWithoutTransaction(const parser::SglQuery &stmt) const {
    for (auto &part : stmt.parts) {
        for (auto &s : part.symbol_table.symbols) {
            if (s.second.type == SymbolNode::NODE || s.second.type == SymbolNode::RELATIONSHIP ||
                s.second.type == SymbolNode::NAMED_PATH) {
                return false;
            }
        }
    }
    return true;
}

OpBase *ExecutionPlan::BuildSgl(const parser::SglQuery &stmt, size_t parts_offset) {
    OpBase *sgl_root = nullptr;
    std::vector<OpBase *> segments;
    for (int i = 0; i < (int)stmt.parts.size(); i++) {
        auto segment = BuildPart(stmt.parts[i], parts_offset + i);
        segments.emplace_back(segment);
    }
#ifndef NDEBUG
    LOG_DEBUG() << "Plan parts:";
    std::string s;
    for (auto seg : segments) OpBase::DumpStream(seg, 0, false, s);
    LOG_DEBUG() << s;
#endif
    // merge segments
    sgl_root = segments[0];
    for (int i = 1; i < (int)stmt.parts.size(); i++) {
        /* Need to re-align the alias_id_map for operations on top of project/aggregate,
         * for the record of project/aggregate operation changes the original alias_id_map.
         * Think about this query:
         *   MATCH (n:Person {name:'Michael Redgrave'})--(m)
         *   WITH m, count(*) AS edge_num WHERE edge_num > 1
         *   RETURN m.name,edge_num
         * When we do the WHERE filter AFTER the aggregation, we use the record produced
         * by aggregate, whose alias_id_map is changed. For instance, the id of edge_num
         * in the original alias_id_map is 3, while the new id is 1.  */
        _RealignAliasId(sgl_root, _pattern_graphs[parts_offset + i].symbol_table);
        sgl_root = _Connect(sgl_root, segments[i], &_pattern_graphs[parts_offset + i]);
    }
    return sgl_root;
}

static bool CheckReturnElements(const std::vector<parser::SglQuery> &stmt) {
    Clause::TYPE_RETURN ret;
    if (!stmt.empty()) {
        auto &last_part = stmt[0].parts.back();
        if (last_part.return_clause) ret = *last_part.return_clause;
    }
    for (size_t i = 1; i < stmt.size(); i++) {
        auto &last_part = stmt[i].parts.back();
        auto last_ret = last_part.return_clause;
        if (!last_ret) return false;
        auto &ret_items = std::get<0>(std::get<1>(ret));
        auto &last_ret_items = std::get<0>(std::get<1>(*last_ret));
        // some certain single query (e.g. MATCH(n) RETURN *) without no return items
        // cannot be unioned.
        if (ret_items.size() == 0 || last_ret_items.size() == 0) {
            return false;
        }
        // if two queries return different size of items, cannot be unioned
        if (ret_items.size() != last_ret_items.size()) {
            return false;
        }
        for (int j = 0; j < (int)ret_items.size(); j++) {
            auto &e1 = std::get<0>(ret_items[j]);
            auto &v1 = std::get<1>(ret_items[j]);
            auto s1 = v1.empty() ? e1.ToString(false) : v1;
            auto &e2 = std::get<0>(last_ret_items[j]);
            auto &v2 = std::get<1>(last_ret_items[j]);
            auto s2 = v2.empty() ? e2.ToString(false) : v2;
            if (s1 != s2) return false;
        }
    }
    return true;
}

void ExecutionPlan::Build(const std::vector<parser::SglQuery> &stmt, parser::CmdType cmd,
                          cypher::RTContext *ctx) {
    // check return elements first
    if (!CheckReturnElements(stmt)) {
        throw lgraph::CypherException(
            "All sub queries in an UNION must have the same column names.");
    }
    _cmd_type = cmd;
    /* read_only = AND(parts read_only)
     * so the initial value should be true.  */
    _read_only = true;
    size_t parts_size = 0;
    for (auto &s : stmt) parts_size += s.parts.size();
    _pattern_graphs.resize(parts_size);
    _root = BuildSgl(stmt[0], 0);
    size_t parts_off = stmt[0].parts.size();
    for (size_t i = 1; i < stmt.size(); i++) {
        auto sgl = BuildSgl(stmt[i], parts_off);
        parts_off += stmt[i].parts.size();
        CYPHER_THROW_ASSERT(_root && !_root->children.empty());
        auto op_union = _root->children[0];
        if (op_union->type != OpType::UNION) {
            op_union = new Union();
            _root->PushInBetween(op_union);
        }
        for (auto child : sgl->children) op_union->AddChild(child);
        delete sgl;
        // NOTE: handle plan's destructor with care!
    }
    // Optimize the operations in the ExecutionPlan.
    // TODO(seijiang): split context-free optimizations & context-dependent ones
    PassManager pass_manager(_root, ctx);
    pass_manager.ExecutePasses();
}

void ExecutionPlan::PreValidate(
    cypher::RTContext *ctx,
    const std::unordered_map<std::string, std::set<std::string>>& node,
    const std::unordered_map<std::string, std::set<std::string>>& edge) {
    if (node.empty() && edge.empty()) {
        return;
    }
    if (ctx->graph_.empty()) {
        return;
    }
    auto graph = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
    auto txn = graph.CreateReadTxn();
    const auto& si = txn.GetSchemaInfo();
    for (const auto& pair : node) {
        auto s = si.v_schema_manager.GetSchema(pair.first);
        if (!s) {
            THROW_CODE(CypherException, "No such vertex label: {}", pair.first);
        }
        for (const auto& name : pair.second) {
            size_t fid;
            if (!s->TryGetFieldId(name, fid)) {
                THROW_CODE(CypherException, "No such vertex property: {}.{}", pair.first, name);
            }
        }
    }
    for (const auto& pair : edge) {
        auto s = si.e_schema_manager.GetSchema(pair.first);
        if (!s) {
            THROW_CODE(CypherException, "No such edge label: {}", pair.first);
        }
        for (const auto& name : pair.second) {
            size_t fid;
            if (!s->TryGetFieldId(name, fid)) {
                THROW_CODE(CypherException, "No such edge property: {}.{}", pair.first, name);
            }
        }
    }
    txn.Abort();
}

void ExecutionPlan::Validate(cypher::RTContext *ctx) {
    // todo(kehuang): Add validation manager here.
    GraphNameChecker checker(_root, ctx);
    checker.Execute();
}

ExecutionPlan::~ExecutionPlan() {
    // free OpBases
    OpBase::FreeStream(_root);
}

void ExecutionPlan::Reset() {
    // reset the operation tree completely
    OpBase::ResetStream(_root, true);
    // clean plan's state (iterators, etc.)
    // TODO(anyone) remove state from plan
    for (auto &g : _pattern_graphs) {
        auto &nodes = g.GetNodes();
        for (auto &n : nodes) {
            if (n.ItRef()) n.ItRef()->FreeIter();
        }
        auto &relps = g.GetRelationships();
        for (auto &r : relps) {
            if (r.ItRef()) r.ItRef()->FreeIter();
        }
    }
}

int ExecutionPlan::Execute(RTContext *ctx) {
    // check input
    std::string msg;
#ifndef NDEBUG
    std::thread::id entry_id = std::this_thread::get_id();  // check if tid changes in this function
#endif
    if (!ctx->Check(msg)) throw lgraph::CypherException(msg);
    // instantiate db, transaction

    size_t memory_limit = ctx->galaxy_->GetUserMemoryLimit(ctx->user_, ctx->user_);
    AllocatorManager.BindTid(ctx->user_);
    AllocatorManager.SetMemoryLimit(memory_limit);

    if (ctx->graph_.empty()) {
        ctx->ac_db_.reset(nullptr);
    } else {
        // We have already created ctx->ac_db_ in opt_rewrite_with_schema_inference.h
        if (!ctx->ac_db_) {
            ctx->ac_db_ = std::make_unique<lgraph::AccessControlledDB>(
                ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_));
        }
        lgraph_api::GraphDB db(ctx->ac_db_.get(), ReadOnly());
        if (ReadOnly()) {
            ctx->txn_ = std::make_unique<lgraph_api::Transaction>(db.CreateReadTxn());
        } else {
            ctx->txn_ =
                std::make_unique<lgraph_api::Transaction>(db.CreateWriteTxn(ctx->optimistic_));
        }
    }

    ctx->result_info_ = std::make_unique<ResultInfo>(GetResultInfo());
    std::vector<std::pair<std::string, lgraph_api::LGraphType>> header;

    for (auto &h : ctx->result_info_->header.colums) {
        std::pair<std::string, lgraph_api::LGraphType> column;
        column.first = h.alias.empty() ? h.name : h.alias;
        column.second = h.type;
        header.emplace_back(column);
    }
    ctx->result_ = std::make_unique<lgraph_api::Result>(lgraph_api::Result(header));

    if (ctx->bolt_conn_) {
        std::unordered_map<std::string, std::any> meta;
        meta["fields"] = ctx->result_->BoltHeader();
        bolt::PackStream ps;
        ps.AppendSuccess(meta);
        ctx->bolt_conn_->PostResponse(std::move(ps.MutableBuffer()));
        auto session = (bolt::BoltSession*)ctx->bolt_conn_->GetContext();
        session->state = bolt::SessionState::STREAMING;
        ctx->result_->MarkPythonDriver(session->python_driver);
    }

    try {
        OpBase::OpResult res;
        do {
            res = _root->Consume(ctx);
#ifndef NDEBUG
            LOG_DEBUG() << "root op result: " << res << " (" << OpBase::OP_OK << " for OK)";
#endif
        } while (res == OpBase::OP_OK);
        Reset();
    } catch (std::exception &e) {
        // clear ctx
        ctx->txn_.reset(nullptr);
        ctx->ac_db_.reset(nullptr);
        throw;
    }
    // finialize, clean db, transaction
    if (ctx->txn_) {
        if (!ReadOnly() && ctx->optimistic_) {
            try {
                ctx->txn_->Commit();
            } catch (std::exception &e) {
                ctx->txn_ = nullptr;
                std::string err = "Optimistic transaction commit failed: ";
                err.append(e.what());
                throw lgraph::TxnCommitException(err);
            }
        } else {
            ctx->txn_->Commit();
        }
    }
    // clear ctx

    ctx->txn_.reset(nullptr);
    ctx->ac_db_.reset(nullptr);
#ifndef NDEBUG
    std::thread::id out_id = std::this_thread::get_id();  // check if tid changes in this function
    if (entry_id != out_id) LOG_DEBUG() << "switch thread from: " << entry_id << " to " << out_id;
#endif
    return 0;
}

const ResultInfo &ExecutionPlan::GetResultInfo() const { return _result_info; }

std::string ExecutionPlan::DumpPlan(int indent, bool statistics) const {
    std::string s;
    s.append(FMA_FMT("ReadOnly:{}\n", ReadOnly()));
    s.append(statistics ? "Profile statistics:\n" : "Execution Plan:\n");
    OpBase::DumpStream(_root, indent, statistics, s);
    return s;
}

std::string ExecutionPlan::DumpGraph() const {
    std::string s;
    for (auto &g : _pattern_graphs) s.append(g.DumpGraph());
    return s;
}
}  // namespace cypher
