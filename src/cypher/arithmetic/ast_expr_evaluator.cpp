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
#include "core/data_type.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/resultset/record.h"
#include "cypher/utils/geax_util.h"
#include "cypher/arithmetic/ast_expr_evaluator.h"
#include "lgraph/lgraph_exceptions.h"

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

cypher::FieldData doCallBuiltinFunc(const std::string& name, cypher::RTContext* ctx,
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

static cypher::FieldData And(const cypher::FieldData& x, const cypher::FieldData& y) {
    cypher::FieldData ret;
    if (x.IsBool() && y.IsBool()) {
        ret.type = FieldData::SCALAR;
        ret.scalar = ::lgraph::FieldData(x.scalar.AsBool() && y.scalar.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static cypher::FieldData Or(const cypher::FieldData& x, const cypher::FieldData& y) {
    cypher::FieldData ret;
    if (x.IsBool() && y.IsBool()) {
        ret.type = FieldData::SCALAR;
        ret.scalar = ::lgraph::FieldData(x.scalar.AsBool() || y.scalar.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static cypher::FieldData Xor(const cypher::FieldData& x, const cypher::FieldData& y) {
    cypher::FieldData ret;
    if (x.IsBool() && y.IsBool()) {
        ret.type = FieldData::SCALAR;
        ret.scalar = ::lgraph::FieldData(!x.scalar.AsBool() != !y.scalar.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static cypher::FieldData Not(const cypher::FieldData& x) {
    cypher::FieldData ret;
    if (x.IsBool()) {
        ret.scalar = ::lgraph::FieldData(!x.scalar.AsBool());
        return ret;
    }
    THROW_CODE(ParserException, "Type error");
}

static cypher::FieldData Neg(const cypher::FieldData& x) {
    if (!((IsNumeric(x) || x.IsNull()))) {
        throw lgraph::CypherException("Type mismatch: expect Integer or Float in sub expr");
    }
    cypher::FieldData ret;
    if (x.IsNull()) return ret;
    if (x.IsInteger()) {
        ret.scalar = ::lgraph::FieldData(-x.scalar.integer());
        return ret;
    } else {
        ret.scalar = ::lgraph::FieldData(-x.scalar.real());
        return ret;
    }
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::GetField* node) {
    auto expr = std::any_cast<Entry>(node->expr()->accept(*this));
    return Entry(cypher::FieldData(expr.GetEntityField(ctx_, node->fieldName())));
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
        return Entry(cypher::FieldData(lgraph::FieldData(true)));
    }
    return Entry(cypher::FieldData(lgraph::FieldData(lef == rig)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BNotEqual* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.EqualNull() && rig.EqualNull()) {
        return Entry(cypher::FieldData(lgraph::FieldData(false)));
    }
    return Entry(cypher::FieldData(lgraph::FieldData(lef != rig)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BGreaterThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(cypher::FieldData(lgraph::FieldData(lef > rig)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BNotSmallerThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(cypher::FieldData(lgraph::FieldData(!(lef < rig))));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BSmallerThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(cypher::FieldData(lgraph::FieldData(lef < rig)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BNotGreaterThan* node) {
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));
    if (lef.type != Entry::RecordEntryType::CONSTANT ||
        rig.type != Entry::RecordEntryType::CONSTANT) {
        NOT_SUPPORT_AND_THROW();
    }
    return Entry(cypher::FieldData(lgraph::FieldData(!(lef > rig))));
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
    for (auto& val : *r_val.constant.array) {
        if (l_val.constant.scalar == val.scalar) {
            return Entry(cypher::FieldData(lgraph::FieldData(true)));
        }
    }
    return Entry(cypher::FieldData(lgraph::FieldData(false)));
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
    THROW_CODE(InputError, FMA_FMT("Plugin [{}] does not exist.", func_name));
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
            return Entry();
        }

    } else {
        for (auto& [cond, expr] : node->caseBodies()) {
            auto cond_val = std::any_cast<Entry>(cond->accept(*this));
            if (!cond_val.IsBool()) {
                NOT_SUPPORT_AND_THROW();
            }
            if (cond_val.constant.scalar.AsBool()) {
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
    std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>> registered_agg_funcs =
        ArithOpNode::RegisterAggFuncs();
    std::string func_name = ToString(node->funcName());
    std::transform(func_name.begin(), func_name.end(), func_name.begin(), ::tolower);
    auto agg_it = registered_agg_funcs.find(func_name);
    if (agg_it != registered_agg_funcs.end()) {
        // Evalute Mode
        if (visit_mode_ == VisitMode::EVALUATE) {
            if (agg_pos_ >= agg_ctxs_.size()) {
                return Entry(cypher::FieldData(lgraph_api::FieldData(0)));
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
                args.emplace_back(Entry(cypher::FieldData()));
            } else {
                args.emplace_back(Entry(cypher::FieldData(lgraph::FieldData(node->isDistinct()))));
                args.emplace_back(std::any_cast<Entry>(node->expr()->accept(*this)));
            }
            agg_ctxs_[agg_pos_]->Step(args);
            return Entry(cypher::FieldData());
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
                return Entry(cypher::FieldData(lgraph_api::FieldData(0)));
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
            args.emplace_back(Entry(cypher::FieldData(lgraph::FieldData(std::get<0>(left)))));
            args.emplace_back(std::any_cast<Entry>(std::get<1>(left)->accept(*this)));
            args.emplace_back(std::any_cast<Entry>(node->rExpr()->accept(*this)));
            agg_ctxs_[agg_pos_]->Step(args);
            return Entry(cypher::FieldData());
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
    std::vector<cypher::FieldData> list;
    for (auto& e : elems) {
        auto entry = std::any_cast<Entry>(e->accept(*this));
        if (!entry.IsMap() && !entry.IsScalar()) NOT_SUPPORT_AND_THROW();
        if (entry.IsScalar()) {
            list.emplace_back(entry.constant.scalar);
        } else {
            list.emplace_back(*entry.constant.map);
        }
    }
    return Entry(cypher::FieldData(list));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkMap* node) {
    const auto& elems = node->elems();
    std::unordered_map<std::string, cypher::FieldData> map;
    for (const auto& pair : elems) {
        auto key = std::any_cast<Entry>(std::get<0>(pair)->accept(*this));
        auto val = std::any_cast<Entry>(std::get<1>(pair)->accept(*this));
        if (!key.IsString()) NOT_SUPPORT_AND_THROW();
        if (!val.IsScalar() && !val.IsArray()) NOT_SUPPORT_AND_THROW();
        if (val.IsScalar()) {
            map.emplace(key.constant.ToString(), val.constant.scalar);
        } else {
            map.emplace(key.constant.ToString(), *val.constant.array);
        }
    }
    return Entry(cypher::FieldData(map));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkRecord* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkSet* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkTuple* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::VBool* node) {
    return Entry(cypher::FieldData(lgraph::FieldData(node->val())));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VInt* node) {
    return Entry(cypher::FieldData(lgraph::FieldData(node->val())));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDouble* node) {
    return Entry(cypher::FieldData(lgraph::FieldData(node->val())));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VString* node) {
    return Entry(cypher::FieldData(lgraph::FieldData(node->val())));
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
    return Entry(cypher::FieldData(lgraph::FieldData()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VNone* node) { NOT_SUPPORT_AND_THROW(); }

std::any cypher::AstExprEvaluator::visit(geax::frontend::Ref* node) {
    auto it = sym_tab_->symbols.find(node->name());
    if (it == sym_tab_->symbols.end()) NOT_SUPPORT_AND_THROW();
    switch (it->second.type) {
    case SymbolNode::NODE:
    case SymbolNode::RELATIONSHIP:
    case SymbolNode::CONSTANT:
    case SymbolNode::PARAMETER:
        return record_->values[it->second.id];
    case SymbolNode::NAMED_PATH:
        {
            auto iter = sym_tab_->anot_collection.path_elements.find(node->name());
            if (iter == sym_tab_->anot_collection.path_elements.end())
                throw lgraph::CypherException("path_elements error: " + node->name());
            const std::vector<std::shared_ptr<geax::frontend::Ref>>& elements = iter->second;
            std::vector<ArithExprNode> params;
            for (auto ref : elements) {
                params.emplace_back(ref.get(), *sym_tab_);
            }
            return Entry(doCallBuiltinFunc(BuiltinFunction::INTL_TO_PATH, ctx_, *record_, params));
        }
    }
    return std::any();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Param* node) {
    auto& variabel = node->name();
    auto it = sym_tab_->symbols.find(variabel);
    if (it == sym_tab_->symbols.end()) {
        throw lgraph::CypherException("Parameter not defined: " + variabel);
    }
    if (record_->values[it->second.id].type == Entry::UNKNOWN) {
        throw lgraph::CypherException("Undefined parameter: " + variabel);
    }
    return record_->values[it->second.id];
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SingleLabel* node) {
    std::unordered_set<std::string> set;
    set.insert(std::move(node->label()));
    return set;
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::LabelOr* node) {
    std::unordered_set<std::string> left;
    checkedAnyCast(node->left()->accept(*this), left);
    std::unordered_set<std::string> right;
    checkedAnyCast(node->left()->accept(*this), right);
    left.insert(right.begin(), right.end());
    return left;
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsLabeled* node) {
    Entry e;
    checkedAnyCast(node->expr()->accept(*this), e);
    CYPHER_THROW_ASSERT(e.IsNode() || e.IsRelationship());
    auto alias = e.constant.scalar.AsString();
    std::unordered_set<std::string> labels;
    checkedAnyCast(node->labelTree()->accept(*this), labels);
    bool exist = false;
    std::string label;
    if (e.IsNode()) {
        auto n = e.node;
        CYPHER_THROW_ASSERT(n && n->IsValidAfterMaterialize(ctx_));
        label = n->ItRef()->GetLabel();
    } else if (e.IsRelationship()) {
        auto rel = e.relationship;
        CYPHER_THROW_ASSERT(rel && rel->ItRef()->IsValid());
        label = rel->ItRef()->GetLabel();
    }
    exist = labels.count(label);
    return Entry(cypher::FieldData(lgraph::FieldData(exist)));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsNull* node) {
    Entry e;
    checkedAnyCast(node->expr()->accept(*this), e);
    cypher::FieldData ret;
    ret.type = FieldData::SCALAR;
    ret.scalar = ::lgraph::FieldData(e.IsNull());
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
    if (!record_->values[it_head->second.id].CheckEntityEfficient(ctx_))
        return Entry(cypher::FieldData(lgraph::FieldData(false)));

    auto& tails = path_chains[0]->tails();
    for (auto& tail : tails) {
        auto relationship = std::get<0>(tail);
        const std::string rel_name = getNodeOrEdgeName(relationship);
        auto it_rel = sym_tab_->symbols.find(rel_name);
        if (it_rel == sym_tab_->symbols.end() || it_rel->second.type != SymbolNode::RELATIONSHIP)
            NOT_SUPPORT_AND_THROW();
        if (!record_->values[it_rel->second.id].CheckEntityEfficient(ctx_))
            return Entry(cypher::FieldData(lgraph::FieldData(false)));
        auto neighbor = std::get<1>(tail);
        const std::string ne_name = getNodeOrEdgeName(neighbor);
        auto it_ne = sym_tab_->symbols.find(ne_name);
        if (it_ne == sym_tab_->symbols.end() || it_ne->second.type != SymbolNode::NODE)
            NOT_SUPPORT_AND_THROW();
        if (!record_->values[it_ne->second.id].CheckEntityEfficient(ctx_))
            return Entry(cypher::FieldData(lgraph::FieldData(false)));
    }
    return Entry(cypher::FieldData(lgraph::FieldData(true)));
}

std::any cypher::AstExprEvaluator::reportError() { return error_msg_; }

std::any AstExprEvaluator::visit(geax::frontend::ListComprehension* node) {
    geax::frontend::Ref *ref = nullptr;
    geax::frontend::Expr *in_expr = nullptr, *op_expr = nullptr;
    checkedCast(node->getVariable(), ref);
    checkedCast(node->getInExpression(), in_expr);
    checkedCast(node->getOpExpression(), op_expr);
    Entry in_e;
    checkedAnyCast(in_expr->accept(*this), in_e);
    CYPHER_THROW_ASSERT(in_e.IsArray());
    auto data_array = in_e.constant.array;
    std::vector<::lgraph::FieldData> ret_data;
    auto it = sym_tab_->symbols.find(ref->name());
    for (auto &data : *data_array) {
        const_cast<Record*>(record_)->values[it->second.id] = Entry(cypher::FieldData(data));
        Entry one_result;
        checkedAnyCast(op_expr->accept(*this), one_result);
        ret_data.push_back(one_result.constant.scalar);
    }
    return Entry(cypher::FieldData(ret_data));
}

}  // namespace cypher
