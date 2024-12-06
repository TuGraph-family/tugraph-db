/**
 * Copyright 2023 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}

 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include <algorithm>
#include <unordered_map>
#include "cypher/cypher_types.h"
#include "common/exceptions.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/arithmetic/ast_expr_evaluator.h"
#include "cypher/resultset/record.h"
#include "cypher/utils/geax_util.h"

#ifndef DO_BINARY_EXPR
#define DO_BINARY_EXPR(func)                                       \
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));  \
    auto rig = std::any_cast<Entry>(node->right()->accept(*this)); \
    if (lef.type != Entry::RecordEntryType::CONSTANT ||            \
        rig.type != Entry::RecordEntryType::CONSTANT) {            \
        NOT_SUPPORT_AND_THROW();                                   \
    }                                                              \
    return Entry(cypher::func(lef.constant, rig.constant));
#endif

#ifndef DO_UNARY_EXPR
#define DO_UNARY_EXPR(func)                                        \
    auto expr = std::any_cast<Entry>(node->expr()->accept(*this)); \
    if (expr.type != Entry::RecordEntryType::CONSTANT) {           \
        NOT_SUPPORT_AND_THROW();                                   \
    }                                                              \
    return Entry(cypher::func(expr.constant));
#endif

Value doCallBuiltinFunc(const std::string& name, cypher::RTContext* ctx,
                                    const cypher::Record& record,
                                    const std::vector<cypher::ArithExprNode>& args) {
    static std::unordered_map<std::string, cypher::BuiltinFunction::FUNC> ae_registered_funcs =
        cypher::ArithOpNode::RegisterFuncs();
    auto it = ae_registered_funcs.find(name);
    if (it == ae_registered_funcs.end()) NOT_SUPPORT_AND_THROW();
    cypher::BuiltinFunction::FUNC func = it->second;
    auto data = func(ctx, record, args);
    return data;
}

const std::string& getNodeOrEdgeName(geax::frontend::AstNode* ast_node) {
    if (ast_node->type() == geax::frontend::AstNodeType::kNode) {
        geax::frontend::Node* node = (geax::frontend::Node*)ast_node;
        if (!node->filler()->v().has_value()) NOT_SUPPORT_AND_THROW();
        return node->filler()->v().value();
    } else if (ast_node->type() == geax::frontend::AstNodeType::kEdge) {
        geax::frontend::Edge* edge = (geax::frontend::Edge*)ast_node;
        if (!edge->filler()->v().has_value()) NOT_SUPPORT_AND_THROW();
        return edge->filler()->v().value();
    } else {
        NOT_SUPPORT_AND_THROW();
    }
}

namespace cypher {

static Value And(const Value& x, const Value& y) {
    Value ret;
    if (x.IsBool() && y.IsBool()) {
        ret = Value(x.AsBool() && y.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static Value Or(const Value& x, const Value& y) {
    Value ret;
    if (x.IsBool() && y.IsBool()) {
        ret = Value(x.AsBool() || y.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static Value Xor(const Value& x, const Value& y) {
    Value ret;
    if (x.IsBool() && y.IsBool()) {
        ret = Value(!x.AsBool() != !y.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static Value Not(const Value& x) {
    Value ret;
    if (x.IsBool()) {
        ret = Value(!x.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static Value Neg(const Value& x) {
    if (!((IsNumeric(x) || x.IsNull()))) {
        THROW_CODE(CypherException, "Type mismatch: expect Integer or Float in sub expr");
    }
    Value ret;
    if (x.IsNull()) return ret;
    if (x.IsInteger()) {
        ret = Value(-x.AsInteger());
        return ret;
    } else {
        ret = Value(-x.AsDouble());
        return ret;
    }
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::GetField* node) {
    auto expr = std::any_cast<Entry>(node->expr()->accept(*this));
    return Entry(expr.GetEntityField(ctx_, node->fieldName()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::TupleGet* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Not* node) { DO_UNARY_EXPR(Not); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::Neg* node) { DO_UNARY_EXPR(Neg); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::Tilde* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::VSome* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BEqual* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.EqualNull() && rig.EqualNull()) {
        return Entry(Value(true));
    }
    return Entry(Value(lef == rig));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BNotEqual* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.EqualNull() && rig.EqualNull()) {
        return Entry(Value(false));
    }
    return Entry(Value(lef != rig));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BGreaterThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(Value(lef > rig));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BNotSmallerThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(Value(!(lef < rig)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BSmallerThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(Value(lef < rig));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BNotGreaterThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(Value(!(lef > rig)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BSafeEqual* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BAdd* node) { DO_BINARY_EXPR(Add); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BSub* node) { DO_BINARY_EXPR(Sub); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BDiv* node) { DO_BINARY_EXPR(Div); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BMul* node) { DO_BINARY_EXPR(Mul); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BMod* node) { DO_BINARY_EXPR(Mod); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BSquare* node) { DO_BINARY_EXPR(Pow); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BAnd* node) { DO_BINARY_EXPR(And); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BOr* node) { DO_BINARY_EXPR(Or); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BXor* node) { DO_BINARY_EXPR(Xor); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitAnd* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitOr* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitXor* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitLeftShift* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitRightShift* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BConcat* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BIndex* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BLike* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::BIn* node) {
    auto l_val = std::any_cast<Entry>(node->left()->accept(*this));
    auto r_val = std::any_cast<Entry>(node->right()->accept(*this));
    if (!l_val.IsScalar()) NOT_SUPPORT_AND_THROW();
    if (!r_val.IsArray()) NOT_SUPPORT_AND_THROW();
    for (auto& val :  r_val.constant.AsArray()) {
        if (l_val.constant == val) {
            return Entry(Value(true));
        }
    }
    return Entry(Value(false));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::If* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::Function* node) {
    static std::unordered_map<std::string, BuiltinFunction::FUNC> ae_registered_funcs =
        ArithOpNode::RegisterFuncs();
    std::string func_name = node->name();
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
    auto it = ae_registered_funcs.find(func_name);
    if (it != ae_registered_funcs.end()) {
        std::vector<ArithExprNode> args;
        args.emplace_back(node, *sym_tab_);
        for (auto i : node->args()) {
            args.emplace_back(i, *sym_tab_);
        }
        return Entry(it->second(ctx_, *record_, args));
    }
    THROW_CODE(InputError, "Func [{}] does not exist.", func_name);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Case* node) {
    if (node->input().has_value()) {
        auto l_val = std::any_cast<Entry>(node->input().value()->accept(*this));
        for (auto& [cond, expr] : node->caseBodies()) {
            auto r_val = std::any_cast<Entry>(cond->accept(*this));
            if (l_val == r_val) {
                return expr->accept(*this);
            }
        }
        if (node->elseBody().has_value()) {
            return node->elseBody().value()->accept(*this);
        } else {
            return Entry(Value());
        }

    } else {
        for (auto& [cond, expr] : node->caseBodies()) {
            auto cond_val = std::any_cast<Entry>(cond->accept(*this));
            if (!cond_val.IsBool()) {
                NOT_SUPPORT_AND_THROW();
            }
            if (cond_val.constant.AsBool()) {
                return expr->accept(*this);
            }
        }
        if (node->elseBody().has_value()) {
            return node->elseBody().value()->accept(*this);
        } else {
            NOT_SUPPORT_AND_THROW();
        }
    }
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Cast* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::MatchCase* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::AggFunc* node) {
    //CYPHER_TODO();
    std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>> registered_agg_funcs =
        ArithOpNode::RegisterAggFuncs();
    std::string func_name = ToString(node->funcName());
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
    auto agg_it = registered_agg_funcs.find(func_name);
    if (agg_it != registered_agg_funcs.end()) {
        // Evalute Mode
        if (visit_mode_ == VisitMode::EVALUATE) {
            if (agg_pos_ >= agg_ctxs_.size()) {
                if (func_name == "count") {
                    return Entry(Value::Integer(0));
                } else {
                    return Entry(Value());
                }
            }
            return agg_ctxs_[agg_pos_++]->result;
        } else if (visit_mode_ == VisitMode::AGGREGATE) {
            // todo(...): registered_agg_funcs cannot be static and need improvement
            // return Entry(agg_it->second());
            if (agg_pos_ == agg_ctxs_.size()) {
                agg_ctxs_.emplace_back(agg_it->second());
            }
            if (agg_pos_ >= agg_ctxs_.size()) {
                NOT_SUPPORT_AND_THROW();
            }
            std::vector<Entry> args;
            if (func_name == "count" &&
                node->expr()->type() == geax::frontend::AstNodeType::kVString &&
                ((geax::frontend::VString*)node->expr())->val() == "*" && record_->Null()) {
                args.emplace_back(Value());
            } else {
                args.emplace_back(Value(node->isDistinct()));
                args.emplace_back(std::any_cast<Entry>(node->expr()->accept(*this)));
            }
            agg_ctxs_[agg_pos_]->Step(args);
            return Entry(Value());
        }
    }
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BAggFunc* node) {
    std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>> registered_agg_funcs =
        ArithOpNode::RegisterAggFuncs();
    std::string func_name = ToString(node->funcName());
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
    auto agg_it = registered_agg_funcs.find(func_name);
    if (agg_it != registered_agg_funcs.end()) {
        // Evalute Mode
        if (visit_mode_ == VisitMode::EVALUATE) {
            if (agg_pos_ >= agg_ctxs_.size()) {
                return Entry(Value());
            }
            return agg_ctxs_[agg_pos_++]->result;
        } else if (visit_mode_ == VisitMode::AGGREGATE) {
            // todo(...): registered_agg_funcs cannot be static and need improvement
            // return Entry(agg_it->second());
            if (agg_pos_ == agg_ctxs_.size()) {
                agg_ctxs_.emplace_back(agg_it->second());
            }
            if (agg_pos_ >= agg_ctxs_.size()) {
                NOT_SUPPORT_AND_THROW();
            }
            std::vector<Entry> args;
            auto& left = node->lExpr();
            args.emplace_back(Entry(Value(std::get<0>(left))));
            args.emplace_back(std::any_cast<Entry>(std::get<1>(left)->accept(*this)));
            args.emplace_back(std::any_cast<Entry>(node->rExpr()->accept(*this)));
            agg_ctxs_[agg_pos_]->Step(args);
            return Entry(Value());
        }
    }
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MultiCount* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Windowing* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkList* node) {
    const auto& elems = node->elems();
    std::vector<Value> list;
    for (auto& e : elems) {
        auto entry = std::any_cast<Entry>(e->accept(*this));
        if (!entry.IsConstant()) NOT_SUPPORT_AND_THROW();
        list.emplace_back(std::move(entry.constant));
    }
    return Entry(Value(std::move(list)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkMap* node) {
    const auto& elems = node->elems();
    std::unordered_map<std::string, Value> map;
    for (const auto& pair : elems) {
        auto key = std::any_cast<Entry>(std::get<0>(pair)->accept(*this));
        auto val = std::any_cast<Entry>(std::get<1>(pair)->accept(*this));
        if (!key.IsString()) NOT_SUPPORT_AND_THROW();
        if (!val.IsConstant()) NOT_SUPPORT_AND_THROW();
        map.emplace(key.constant.AsString(), std::move(val.constant));
    }
    return Entry(Value(std::move(map)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkRecord* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkSet* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkTuple* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::VBool* node) {
    return Entry(Value(node->val()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VInt* node) {
    return Entry(Value(node->val()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDouble* node) {
    return Entry(Value(node->val()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VString* node) {
    return Entry(Value(node->val()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDate* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDatetime* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDuration* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VTime* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::VNull* node) {
    return Entry(Value());
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VNone* node) { NOT_SUPPORT_AND_THROW(); }

/* Creates a path from a given sequence of graph entities.
 * The arguments are the sequence of graph entities combines the path.
 * The sequence is always in odd length and defined as:
 * Odd indices members are always representing the value of a single node.
 * Even indices members are either representing the value of a single edge,
 * or an sipath, in case of variable length traversal.  */
Entry ToPath(RTContext *ctx, const Record &record, const std::vector<ArithExprNode> &args) {
    if (args.size() % 2 == 0) CYPHER_ARGUMENT_ERROR();
    Entry ret;
    ret.type = Entry::PATH;
    for (auto &arg : args) {
        auto r = arg.Evaluate(ctx, record);
        if (r.type == Entry::VAR_LEN_RELP) {
            // CYPHER_TODO();
            ret.path.clear();
            int len = int(r.relationship->path_.Length());
            for (int i = 0; i < len; ++i) {
                if (i != 0) {
                    ret.path.push_back(PathElement{false, r.relationship->path_.edges[i - 1]});
                }
                ret.path.push_back(PathElement{true, r.relationship->path_.vertexes[i]});
            }
            break ;
        } else {
            if ((r.IsNode() && !r.node->vertex_) || (r.IsRelationship() && !r.relationship->edge_)) {
                ret.path.clear();
                break;
            }
            if (r.IsNode()) {
                ret.path.emplace_back(PathElement{true, r.node->vertex_.value()});
            } else if (r.IsRelationship()) {
                ret.path.emplace_back(PathElement{false, r.relationship->edge_.value()});
            } else {
                // CYPHER_TODO();
            }
        }
    }
    return ret;
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Ref* node) {
    auto it = sym_tab_->symbols.find(node->name());
    if (it == sym_tab_->symbols.end()) NOT_SUPPORT_AND_THROW();
    switch (it->second.type) {
        case SymbolNode::NODE:
        case SymbolNode::RELATIONSHIP:
        case SymbolNode::CONSTANT:
        case SymbolNode::PARAMETER:
            return record_->values[it->second.id];
        case SymbolNode::NAMED_PATH: {
            auto iter = sym_tab_->anot_collection.path_elements.find(node->name());
            if (iter == sym_tab_->anot_collection.path_elements.end())
                THROW_CODE(CypherException, "path_elements error: " + node->name());
            const std::vector<std::shared_ptr<geax::frontend::Ref>>& elements = iter->second;
            std::vector<ArithExprNode> params;
            params.reserve(elements.size());
            for (const auto& ref : elements) {
                params.emplace_back(ref.get(), *sym_tab_);
            }
            return ToPath(ctx_, *record_, params);
            //return Entry(doCallBuiltinFunc(BuiltinFunction::INTL_TO_PATH, ctx_, *record_, params));
        }
        default: {
            CYPHER_TODO();
        }
    }
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Param* node) {
    auto& variabel = node->name();
    auto it = sym_tab_->symbols.find(variabel);
    if (it == sym_tab_->symbols.end()) {
        THROW_CODE(CypherException, "Parameter not defined: " + variabel);
    }
    if (record_->values[it->second.id].type == Entry::UNKNOWN) {
        THROW_CODE(CypherException, "Undefined parameter: " + variabel);
    }
    return record_->values[it->second.id];
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SingleLabel* node) {
    std::unordered_set<std::string> set;
    set.insert(node->label());
    return set;
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::LabelOr* node) {
    std::unordered_set<std::string> left;
    left = std::any_cast<std::unordered_set<std::string>>(node->left()->accept(*this));
    std::unordered_set<std::string> right;
    right = std::any_cast<std::unordered_set<std::string>>(node->right()->accept(*this));
    left.insert(right.begin(), right.end());
    return left;
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsLabeled* node) {
    auto e = std::any_cast<Entry>(node->expr()->accept(*this));
    CYPHER_THROW_ASSERT(e.IsNode() || e.IsRelationship());
    auto expect = std::any_cast<std::unordered_set<std::string>>(node->labelTree()->accept(*this));
    std::unordered_set<std::string> labels;
    if (e.IsNode()) {
        labels = e.node->vertex_->GetLabels();
    } else if (e.IsRelationship()) {
        auto type = e.relationship->edge_->GetType();
        labels.insert(type);
    }
    std::unordered_set<std::string> res;
    std::set_intersection(expect.begin(), expect.end(),
                          labels.begin(), labels.end(),
                          std::inserter(res, res.begin()));

    return Entry(Value(!res.empty()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsNull* node) {
    Value ret;
    return Entry(ret);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Exists* node) {
    auto path_chains = node->pathChains();
    if (path_chains.size() > 1) NOT_SUPPORT_AND_THROW();
    auto head = path_chains[0]->head();
    const std::string head_name = getNodeOrEdgeName(head);
    auto it_head = sym_tab_->symbols.find(head_name);
    if (it_head == sym_tab_->symbols.end() || it_head->second.type != SymbolNode::NODE)
        NOT_SUPPORT_AND_THROW();
    CYPHER_TODO();
    /*if (!record_->values[it_head->second.id].CheckEntityEfficient(ctx_))
        return Entry(cypher::FieldData(PropertyValue(false)));

    auto& tails = path_chains[0]->tails();
    for (auto& tail : tails) {
        auto relationship = std::get<0>(tail);
        const std::string rel_name = getNodeOrEdgeName(relationship);
        auto it_rel = sym_tab_->symbols.find(rel_name);
        if (it_rel == sym_tab_->symbols.end() || it_rel->second.type != SymbolNode::RELATIONSHIP)
            NOT_SUPPORT_AND_THROW();
        if (!record_->values[it_rel->second.id].CheckEntityEfficient(ctx_))
            return Entry(cypher::FieldData(PropertyValue(false)));
        auto neighbor = std::get<1>(tail);
        const std::string ne_name = getNodeOrEdgeName(neighbor);
        auto it_ne = sym_tab_->symbols.find(ne_name);
        if (it_ne == sym_tab_->symbols.end() || it_ne->second.type != SymbolNode::NODE)
            NOT_SUPPORT_AND_THROW();
        if (!record_->values[it_ne->second.id].CheckEntityEfficient(ctx_))
            return Entry(cypher::FieldData(PropertyValue(false)));
    }
    return Entry(cypher::FieldData(PropertyValue(true)));*/
}

std::any cypher::AstExprEvaluator::reportError() { return error_msg_; }

std::any AstExprEvaluator::visit(geax::frontend::ListComprehension* node) {
    geax::frontend::Ref *ref = nullptr;
    geax::frontend::Expr *in_expr = nullptr, *op_expr = nullptr;
    checkedCast(node->getVariable(), ref);
    checkedCast(node->getInExpression(), in_expr);
    checkedCast(node->getOpExpression(), op_expr);
    Entry in_e;
    in_e = std::any_cast<Entry>(in_expr->accept(*this));
    CYPHER_THROW_ASSERT(in_e.IsArray());
    const auto& data_array = in_e.constant.AsArray();
    std::vector<Value> ret_data;
    auto it = sym_tab_->symbols.find(ref->name());
    for (auto &data : data_array) {
        const_cast<Record*>(record_)->values[it->second.id] = Entry(data);
        Entry one_result;
        one_result = std::any_cast<Entry>(op_expr->accept(*this));
        ret_data.push_back(one_result.constant);
    }
    return Entry(Value(ret_data));
}

std::any AstExprEvaluator::visit(geax::frontend::PredicateFunction* node) {
    geax::frontend::Ref *ref = nullptr;
    geax::frontend::Expr *in_expr = nullptr, *where_expr = nullptr;
    checkedCast(node->getVariable(), ref);
    checkedCast(node->getInExpression(), in_expr);
    checkedCast(node->getWhereExpression(), where_expr);
    auto predicateType = cypher::PredicateType(node->getPredicateType());
    Entry in_e;
    in_e = std::any_cast<Entry>(in_expr->accept(*this));
    CYPHER_THROW_ASSERT(in_e.IsArray());
    const auto& data_array = in_e.constant.AsArray();
    std::vector<bool> ret_data;
    auto it = sym_tab_->symbols.find(ref->name());
    for (auto &data : data_array) {
        const_cast<Record*>(record_)->values[it->second.id] = Entry(data);
        Entry one_result;
        one_result = std::any_cast<Entry>(where_expr->accept(*this));
        ret_data.push_back(one_result.constant.AsBool());
    }
    bool ans = false;
    switch (predicateType) {
        case cypher::PredicateType::None:
        {
            ans = true;
            for (const auto &r : ret_data) {
                ans &= !r;
            }
            break;
        }
        case cypher::PredicateType::Single:
        {
            int count = 0;
            for (const auto &r : ret_data) {
                if (r) {
                    count++;
                }
            }
            ans = count == 1;
            break;
        }
        case cypher::PredicateType::Any:
        {
            for (const auto &r : ret_data) {
                if (r) {
                    ans = true;
                    break;
                }
            }
            break;
        }
        case cypher::PredicateType::All:
        {
            ans = true;
            for (const auto &r : ret_data) {
                ans &= r;
            }
            break;
        }
    }
    return Entry(Value(ans));
}

}  // namespace cypher
