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
// Created by wt on 18-8-30.
// Modified by bxj on 24-3-30.
//

#include "cypher/execution_plan/ops/op_var_len_expand.h"

namespace cypher {

// DFS State Class
DfsState::DfsState(RTContext *ctx, lgraph::VertexId id, int level, cypher::Relationship *relp,
                   ExpandTowards expand_direction, bool needNext, bool isMaxHop)
    : currentNodeId(id), level(level), count(1), needNext(needNext) {
    auto &types = relp->Types();
    auto iter_type = lgraph::EIter::NA;
    switch (expand_direction) {
    case ExpandTowards::FORWARD:
        iter_type = types.empty() ? lgraph::EIter::OUT_EDGE : lgraph::EIter::TYPE_OUT_EDGE;
        break;
    case ExpandTowards::REVERSED:
        iter_type = types.empty() ? lgraph::EIter::IN_EDGE : lgraph::EIter::TYPE_IN_EDGE;
        break;
    case ExpandTowards::BIDIRECTIONAL:
        iter_type = types.empty() ? lgraph::EIter::BI_EDGE : lgraph::EIter::BI_TYPE_EDGE;
        break;
    }
    if (!isMaxHop) {
        // if reach max hop, do not init eiter
        // level start from 1, mention it
        (relp->ItsRef()[level - 1]).Initialize(ctx->txn_->GetTxn().get(), iter_type, id, types);
        currentEit = &(relp->ItsRef()[level - 1]);
    }
}

void DfsState::getTime() {
    if (!currentEit || !currentEit->IsValid()) {
        return;
    }
    timestamp = lgraph::FieldData(currentEit->GetField("timestamp"));
}

// Predicate Class
bool ValidPredicate::eval(std::vector<DfsState> &stack) {
    // every eiter in stack must be valid
    return stack.back().currentEit->IsValid();
}

bool HeadPredicate::eval(std::vector<DfsState> &stack) {
    if (stack.size() >= 2) {
        return true;
    }
    // only check the first timestamp
    FieldData head = stack.front().timestamp;
    switch (op) {
    case lgraph::CompareOp::LBR_GT:
        return head > operand;
    case lgraph::CompareOp::LBR_GE:
        return head >= operand;
    case lgraph::CompareOp::LBR_LT:
        return head < operand;
    case lgraph::CompareOp::LBR_LE:
        return head <= operand;
    case lgraph::CompareOp::LBR_EQ:
        return head == operand;
    case lgraph::CompareOp::LBR_NEQ:
        return head != operand;
    default:
        return false;
    }
}

bool LastPredicate::eval(std::vector<DfsState> &stack) {
    // last timestamp, check every one
    FieldData last = stack.back().timestamp;
    switch (op) {
    case lgraph::CompareOp::LBR_GT:
        return last > operand;
    case lgraph::CompareOp::LBR_GE:
        return last >= operand;
    case lgraph::CompareOp::LBR_LT:
        return last < operand;
    case lgraph::CompareOp::LBR_LE:
        return last <= operand;
    case lgraph::CompareOp::LBR_EQ:
        return last == operand;
    case lgraph::CompareOp::LBR_NEQ:
        return last != operand;
    default:
        return false;
    }
}

bool IsAscPredicate::eval(std::vector<DfsState> &stack) {
    if (stack.size() == 1) {
        return true;
    }
    auto it = stack.end();
    if ((it - 1)->timestamp > (it - 2)->timestamp) {
        // check the last two timestamp, now is asc
        return true;
    } else {
        return false;
    }
}

bool IsDescPredicate::eval(std::vector<DfsState> &stack) {
    if (stack.size() == 1) {
        return true;
    }
    auto it = stack.end();
    if ((it - 1)->timestamp < (it - 2)->timestamp) {
        // is desc
        return true;
    } else {
        return false;
    }
}

bool MaxInListPredicate::eval(std::vector<DfsState> &stack) {
    FieldData maxInList;
    if (stack.size() == 1) {
        stack.back().maxTimestamp = stack.back().timestamp;
        maxInList = stack.back().maxTimestamp;
    } else {
        auto it = stack.end();
        if ((it - 1)->timestamp <= (it - 2)->maxTimestamp) {
            // if the last timestamp is no larger than the previous maxTimestamp
            (it - 1)->maxTimestamp = (it - 2)->maxTimestamp;
            return true;
        } else {
            (it - 1)->maxTimestamp = (it - 1)->timestamp;
            maxInList = (it - 1)->maxTimestamp;
        }
    }
    switch (op) {
    case lgraph::CompareOp::LBR_LT:
        return maxInList < operand;
    case lgraph::CompareOp::LBR_LE:
        return maxInList <= operand;
    case lgraph::CompareOp::LBR_GT:
        return maxInList > operand;
    case lgraph::CompareOp::LBR_GE:
        return maxInList >= operand;
    case lgraph::CompareOp::LBR_EQ:
        return maxInList == operand;
    case lgraph::CompareOp::LBR_NEQ:
        return maxInList != operand;
    default:
        return false;
    }
}

bool MinInListPredicate::eval(std::vector<DfsState> &stack) {
    FieldData minInList;
    if (stack.size() == 1) {
        stack.back().minTimestamp = stack.back().timestamp;
        minInList = stack.back().minTimestamp;
    } else {
        auto it = stack.end();
        if ((it - 1)->timestamp >= (it - 2)->minTimestamp) {
            (it - 1)->minTimestamp = (it - 2)->minTimestamp;
            return true;
        } else {
            (it - 1)->minTimestamp = (it - 1)->timestamp;
            minInList = (it - 1)->minTimestamp;
        }
    }
    switch (op) {
    case lgraph::CompareOp::LBR_GT:
        return minInList > operand;
    case lgraph::CompareOp::LBR_GE:
        return minInList >= operand;
    case lgraph::CompareOp::LBR_LT:
        return minInList < operand;
    case lgraph::CompareOp::LBR_LE:
        return minInList <= operand;
    case lgraph::CompareOp::LBR_EQ:
        return minInList == operand;
    case lgraph::CompareOp::LBR_NEQ:
        return minInList != operand;
    default:
        return false;
    }
}

// VarLenExpand Class
bool VarLenExpand::NextWithFilter(RTContext *ctx) {
    while (!stack.empty()) {
        auto &currentState = stack.back();
        // auto currentNodeId = currentState.currentNodeId;
        auto &currentEit = currentState.currentEit;
        auto currentLevel = currentState.level;  // actually, is the path length in stack

        // the part of count
        auto &currentCount = currentState.count;

        // if needNext is true, the back currentEit next, then set needNext to false
        auto &needNext = currentState.needNext;
        if (needNext) {
            // deal with the top eit, only Next in this block, don't pop stack
            // CYPHER_THROW_ASSERT(currentEit->IsValid());
            // CYPHER_THROW_ASSERT(currentEit->GetUid() ==
            //                     relp_->path_.GetNthEdgeWithTid(relp_->path_.Length() - 1));
            // check unique, delete previous edge
            if (ctx->path_unique_ && relp_->path_.Length() != 0) {
                CYPHER_THROW_ASSERT(pattern_graph_->VisitedEdges().Erase(
                    relp_->path_.GetNthEdgeWithTid(relp_->path_.Length() - 1)));
            }
            relp_->path_.PopBack();

            currentEit->Next();
            needNext = false;
            currentState.getTime();
            currentCount++;

            bool isFinding = true;
            while (isFinding) {
                bool continueFind = false;
                // check predicates here, path derived from eiters in stack
                for (auto &p : predicates) {
                    if (!p->eval(stack)) {
                        // not fit predicate
                        continueFind = true;
                        if (stack.back().currentEit->IsValid()) {
                            // the back eiter is still valid
                            stack.back().currentEit->Next();
                            stack.back().getTime();
                            stack.back().count++;
                        } else {
                            // now the back eiter of stack is invalid
                            isFinding = false;
                        }
                        break;
                    }
                }
                if (continueFind) {
                    continue;
                }
                // when reach here, the eit, path's predicate are ok
                // CYPHER_THROW_ASSERT(currentEit == stack.back().currentEit);
                // CYPHER_THROW_ASSERT(currentEit->IsValid());
                isFinding = false;

                // add edge's euid to path
                relp_->path_.Append(currentEit->GetUid());

                if (ctx->path_unique_ && pattern_graph_->VisitedEdges().Contains(*currentEit)) {
                    // if this edge has been added, find next edge from the same eiter
                    isFinding = true;
                    // set next
                    stack.back().currentEit->Next();
                    stack.back().getTime();
                    stack.back().count++;
                    // pop path
                    relp_->path_.PopBack();
                } else if (ctx->path_unique_) {
                    // this is ok, add edge to path unique
                    pattern_graph_->VisitedEdges().Add(*currentEit);
                }
            }
        }

        if ((int)relp_->path_.Length() == currentLevel && currentLevel == max_hop_) {
            // CYPHER_THROW_ASSERT(currentEit->IsValid());
            // the top eit is valid
            neighbor_->PushVid(currentEit->GetNbr(expand_direction_));
            needNext = true;

            // check label
            if (!neighbor_->Label().empty() && neighbor_->IsValidAfterMaterialize(ctx) &&
                neighbor_->ItRef()->GetLabel() != neighbor_->Label()) {
                continue;
            }

            return true;
        }

        if (currentEit->IsValid()) {
            // eit is valid, set currentNodeId's eiter's needNext to true
            needNext = true;

            auto neighbor = currentEit->GetNbr(expand_direction_);
            stack.emplace_back(ctx, neighbor, currentLevel + 1, relp_, expand_direction_, false,
                               currentLevel + 1 > max_hop_);
            stack.back().getTime();

            bool isFinding = true;
            while (isFinding) {
                bool continueFind = false;
                // check predicates here, path derived from eiters
                for (auto &p : predicates) {
                    if (!p->eval(stack)) {
                        // not fit predicate
                        continueFind = true;
                        if (stack.back().currentEit->IsValid()) {
                            // the back eiter is still valid
                            stack.back().currentEit->Next();
                            stack.back().getTime();
                            stack.back().count++;
                        } else {
                            // now the back eiter of stack is invalid
                            isFinding = false;
                        }
                        break;
                    }
                }
                if (continueFind) {
                    continue;
                }
                // when reach here, the eit, path's predicate are ok
                isFinding = false;

                // add edge's euid to path
                relp_->path_.Append(stack.back().currentEit->GetUid());

                if (ctx->path_unique_ &&
                    pattern_graph_->VisitedEdges().Contains(*stack.back().currentEit)) {
                    // if this edge has been added, find next edge
                    isFinding = true;
                    // set next
                    stack.back().currentEit->Next();
                    stack.back().getTime();
                    stack.back().count++;
                    // the edge occurs before, pop it
                    relp_->path_.PopBack();
                } else if (ctx->path_unique_) {
                    // this is ok, add edge
                    pattern_graph_->VisitedEdges().Add(*stack.back().currentEit);
                }
            }

        } else {
            // now the top eit is invaild
            stack.pop_back();
            auto pathLen = relp_->path_.Length();
            if (pathLen == stack.size() && pathLen >= (size_t)min_hop_) {
                // CYPHER_THROW_ASSERT(stack.back().currentEit->IsValid());
                neighbor_->PushVid(stack.back().currentEit->GetNbr(expand_direction_));

                stack.back().needNext = true;

                // check label
                if (!neighbor_->Label().empty() && neighbor_->IsValidAfterMaterialize(ctx) &&
                    neighbor_->ItRef()->GetLabel() != neighbor_->Label()) {
                    continue;
                }

                return true;
            }
        }
    }
    return false;
}

VarLenExpand::VarLenExpand(PatternGraph *pattern_graph, Node *start, Node *neighbor,
                           Relationship *relp)
    : OpBase(OpType::VAR_LEN_EXPAND, "Variable Length Expand"),
      pattern_graph_(pattern_graph),
      start_(start),
      neighbor_(neighbor),
      relp_(relp),
      min_hop_(relp->MinHop()),
      max_hop_(relp->MaxHop()) {
    modifies.emplace_back(neighbor_->Alias());
    modifies.emplace_back(relp_->Alias());
    auto &sym_tab = pattern_graph->symbol_table;
    auto sit = sym_tab.symbols.find(start_->Alias());
    auto dit = sym_tab.symbols.find(neighbor_->Alias());
    auto rit = sym_tab.symbols.find(relp_->Alias());
    CYPHER_THROW_ASSERT(sit != sym_tab.symbols.end() && dit != sym_tab.symbols.end() &&
                        rit != sym_tab.symbols.end());
    expand_direction_ = relp_->Undirected()            ? BIDIRECTIONAL
                        : relp_->Src() == start_->ID() ? FORWARD
                                                       : REVERSED;
    start_rec_idx_ = sit->second.id;
    nbr_rec_idx_ = dit->second.id;
    relp_rec_idx_ = rit->second.id;
}

void VarLenExpand::addPredicate(std::unique_ptr<Predicate> p) {
    predicates.push_back(std::move(p));
}

void VarLenExpand::PushFilter(std::shared_ptr<lgraph::Filter> filter) {
    if (filter) {
        if (filter->Type() == lgraph::Filter::RANGE_FILTER) {
            std::shared_ptr<lgraph::RangeFilter> tmp_filter =
                std::static_pointer_cast<lgraph::RangeFilter>(filter);
            if (tmp_filter->GetAeLeft().op.type == cypher::ArithOpNode::AR_OP_FUNC) {
                std::string func_name = tmp_filter->GetAeLeft().op.func_name;
                std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
                if (func_name == "isasc") {
                    auto p = std::make_unique<IsAscPredicate>();
                    addPredicate(std::move(p));
                } else if (func_name == "isdesc") {
                    auto p = std::make_unique<IsDescPredicate>();
                    addPredicate(std::move(p));
                } else if (func_name == "head") {
                    lgraph::CompareOp op = tmp_filter->GetCompareOp();
                    FieldData operand = tmp_filter->GetAeRight().operand.constant;
                    auto p = std::make_unique<HeadPredicate>(op, operand);
                    addPredicate(std::move(p));
                } else if (func_name == "last") {
                    lgraph::CompareOp op = tmp_filter->GetCompareOp();
                    FieldData operand = tmp_filter->GetAeRight().operand.constant;
                    auto p = std::make_unique<LastPredicate>(op, operand);
                    addPredicate(std::move(p));
                } else if (func_name == "maxinlist") {
                    lgraph::CompareOp op = tmp_filter->GetCompareOp();
                    FieldData operand = tmp_filter->GetAeRight().operand.constant;
                    auto p = std::make_unique<MaxInListPredicate>(op, operand);
                    addPredicate(std::move(p));
                } else if (func_name == "mininlist") {
                    lgraph::CompareOp op = tmp_filter->GetCompareOp();
                    FieldData operand = tmp_filter->GetAeRight().operand.constant;
                    auto p = std::make_unique<MinInListPredicate>(op, operand);
                    addPredicate(std::move(p));
                } else {
                    throw lgraph::CypherException("Not in 6 predicates.");
                }
            }
        }
        PushFilter(filter->Left());
        PushFilter(filter->Right());
    }
    return;
}

void VarLenExpand::PushDownEdgeFilter(std::shared_ptr<lgraph::Filter> edge_filter) {
    edge_filter_ = edge_filter;
    // add filter to local Predicates
    PushFilter(edge_filter);
}

OpBase::OpResult VarLenExpand::Initialize(RTContext *ctx) {
    CYPHER_THROW_ASSERT(!children.empty());
    auto child = children[0];
    auto res = child->Initialize(ctx);
    if (res != OP_OK) return res;
    record = child->record;
    record->values[start_rec_idx_].type = Entry::NODE;
    record->values[start_rec_idx_].node = start_;
    record->values[nbr_rec_idx_].type = Entry::NODE;
    record->values[nbr_rec_idx_].node = neighbor_;
    record->values[relp_rec_idx_].type = Entry::VAR_LEN_RELP;
    record->values[relp_rec_idx_].relationship = relp_;
    relp_->ItsRef().resize(max_hop_);

    auto p = std::make_unique<ValidPredicate>();
    addPredicate(std::move(p));
    return OP_OK;
}

OpBase::OpResult VarLenExpand::RealConsume(RTContext *ctx) {
    CYPHER_THROW_ASSERT(!children.empty());
    auto child = children[0];
    while (!NextWithFilter(ctx)) {
        auto res = child->Consume(ctx);
        relp_->path_.Clear();
        if (res != OP_OK) {
            return res;
        }
        // init the first of stack
        lgraph::VertexId startVid = start_->PullVid();
        if (startVid < 0) {
            continue;
        }
        CYPHER_THROW_ASSERT(stack.empty());
        // push the first node and the related eiter into the stack
        // the first node and the related edge is chosen, path length is 1
        stack.emplace_back(ctx, startVid, 1, relp_, expand_direction_, false, 1 > max_hop_);
        stack.back().getTime();

        // check the first node and edge
        bool nextNode = false;
        bool isFinding = true;
        while (isFinding) {
            // check predicates here, path derived from eiters in stack
            bool continueCheck = false;
            for (auto &p : predicates) {
                if (!p->eval(stack)) {
                    // when the edge in the stack not fit
                    if (stack.back().currentEit->IsValid()) {
                        // still vaild, find next edge
                        stack.back().currentEit->Next();
                        stack.back().getTime();
                        stack.back().count++;
                        continueCheck = true;
                    } else {
                        // the top eiter is not valid, should find next node
                        stack.pop_back();
                        nextNode = true;
                    }
                    break;
                }
            }
            if (continueCheck) {
                // find next edge of the same node
                // continueCheck = false;
                continue;
            } else {
                // if ok, it means this path is ok, find next hop
                isFinding = false;
            }
        }
        if (nextNode) {
            // find next node
            continue;
        }
        // when reach here, the first node and eiter are ok

        relp_->path_.SetStart(startVid);
        relp_->path_.Append(stack.back().currentEit->GetUid());
        if (ctx->path_unique_) {
            // add the first edge
            pattern_graph_->VisitedEdges().Add(*stack.back().currentEit);
        }
    }
    return OP_OK;
}

OpBase::OpResult VarLenExpand::ResetImpl(bool complete) {
    std::vector<DfsState>().swap(stack);
    // stack.clear();
    relp_->path_.Clear();
    return OP_OK;
}

std::string VarLenExpand::ToString() const {
    auto towards = expand_direction_ == FORWARD    ? "-->"
                   : expand_direction_ == REVERSED ? "<--"
                                                   : "--";
    std::string edgefilter_str = "VarLenEdgeFilter";
    return fma_common::StringFormatter::Format(
        "{}({}) [{} {}*{}..{} {} {}]", name, "All", start_->Alias(), towards,
        std::to_string(min_hop_), std::to_string(max_hop_), neighbor_->Alias(),
        edge_filter_ ? edgefilter_str.append(" (").append(edge_filter_->ToString()).append(")")
                     : "");
}

}  // namespace cypher
