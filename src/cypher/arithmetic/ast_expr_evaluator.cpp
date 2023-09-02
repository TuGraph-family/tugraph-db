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

#include "geax-front-end/ast/expr/VDouble.h"
#include "core/data_type.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/resultset/record.h"
#include "cypher/utils/geax_util.h"
#include "cypher/arithmetic/ast_expr_evaluator.h"

#ifndef DO_BINARY_EXPR
#define DO_BINARY_EXPR(func)                                            \
    auto lef = std::any_cast<Entry>(node->left()->accept(*this));       \
    auto rig = std::any_cast<Entry>(node->right()->accept(*this));      \
    if (lef.type != Entry::RecordEntryType::CONSTANT ||                 \
        rig.type != Entry::RecordEntryType::CONSTANT) {                 \
        NOT_SUPPORT_AND_THROW();                                        \
    }                                                                   \
    return Entry(cypher::func(lef.constant, rig.constant));
#endif

#ifndef DO_UNARY_EXPR
#define DO_UNARY_EXPR(func)                                             \
    auto expr = std::any_cast<Entry>(node->expr()->accept(*this));      \
    if (expr.type != Entry::RecordEntryType::CONSTANT) {                \
        NOT_SUPPORT_AND_THROW();                                        \
    }                                                                   \
    return Entry(cypher::func(expr.constant));
#endif

namespace cypher {

static cypher::FieldData And(const cypher::FieldData &x, const cypher::FieldData &y) {
    cypher::FieldData ret;
    if (x.IsBool() && y.IsBool()) {
        ret.type = FieldData::SCALAR;
        ret.scalar = ::lgraph::FieldData(x.scalar.AsBool() && y.scalar.AsBool());
        return ret;
    }
    NOT_SUPPORT_AND_THROW();
}

static cypher::FieldData Or(const cypher::FieldData &x, const cypher::FieldData &y) {
    cypher::FieldData ret;
    if (x.IsBool() && y.IsBool()) {
        ret.type = FieldData::SCALAR;
        ret.scalar = ::lgraph::FieldData(x.scalar.AsBool() || y.scalar.AsBool());
        return ret;
    }
    NOT_SUPPORT_AND_THROW();
}

static cypher::FieldData Xor(const cypher::FieldData &x, const cypher::FieldData &y) {
    cypher::FieldData ret;
    if (x.IsBool() && y.IsBool()) {
        ret.type = FieldData::SCALAR;
        ret.scalar = ::lgraph::FieldData(!x.scalar.AsBool() != !y.scalar.AsBool());
        return ret;
    }
    NOT_SUPPORT_AND_THROW();
}

static cypher::FieldData Not(const cypher::FieldData &x) {
    cypher::FieldData ret;
    if (x.IsBool()) {
        ret.scalar = ::lgraph::FieldData(!x.scalar.AsBool());
        return ret;
    }
    NOT_SUPPORT_AND_THROW();
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

std::any cypher::AstExprEvaluator::visit(geax::frontend::Not* node) {
    DO_UNARY_EXPR(Not);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Neg* node) {
    DO_UNARY_EXPR(Neg);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Tilde* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VSome* node) {
    NOT_SUPPORT_AND_THROW();
}

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

std::any cypher::AstExprEvaluator::visit(geax::frontend::BAdd* node) {
    DO_BINARY_EXPR(Add);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BSub* node) {
    DO_BINARY_EXPR(Sub);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BDiv* node) {
    DO_BINARY_EXPR(Div);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BMul* node) {
    DO_BINARY_EXPR(Mul);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BMod* node) {
    DO_BINARY_EXPR(Mod);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BAnd* node) {
    DO_BINARY_EXPR(And);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BOr* node) {
    DO_BINARY_EXPR(Or);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BXor* node) {
    DO_BINARY_EXPR(Xor);
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitAnd* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitOr* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitXor* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitLeftShift* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BBitRightShift* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BConcat* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BIndex* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BLike* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BIn* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::If* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Function* node) {
    static std::unordered_map<std::string, BuiltinFunction::FUNC> ae_registered_funcs =
        ArithOpNode::RegisterFuncs();
    auto it = ae_registered_funcs.find(node->name());
    if (it != ae_registered_funcs.end()) {
        std::vector<ArithExprNode> args;
        args.emplace_back(node, *sym_tab_);
        for (auto i : node->args()) {
            args.emplace_back(i, *sym_tab_);
        }
        return Entry(it->second(ctx_, *record_, args));
    }
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Case* node) {
    if (node->input().has_value()) {
        NOT_SUPPORT_AND_THROW();
    }
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

std::any cypher::AstExprEvaluator::visit(geax::frontend::Cast* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MatchCase* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::AggFunc* node) {
    std::unordered_map<std::string, std::function<std::shared_ptr<AggCtx>()>> registered_agg_funcs =
        ArithOpNode::RegisterAggFuncs();
    auto agg_it = registered_agg_funcs.find(ToString(node->funcName()));
    if (agg_it != registered_agg_funcs.end()) {
        // Evalute Mode
        if (visit_mode_ == VisitMode::EVALUATE) {
            if (agg_pos_ >= agg_ctxs_.size()) {
                NOT_SUPPORT_AND_THROW();
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
            args.emplace_back(Entry(cypher::FieldData(lgraph::FieldData(node->isDistinct()))));
            args.emplace_back(std::any_cast<Entry>(node->expr()->accept(*this)));
            agg_ctxs_[agg_pos_]->Step(args);
            return Entry();
        }
    }
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BAggFunc* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MultiCount* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Windowing* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkList* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkMap* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkRecord* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkSet* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MkTuple* node) {
    NOT_SUPPORT_AND_THROW();
}

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

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDate* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDatetime* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VDuration* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VTime* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VNull* node) {
    return Entry(cypher::FieldData(lgraph::FieldData()));
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::VNone* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Ref* node) {
    auto it = sym_tab_->symbols.find(node->name());
    return record_->values[it->second.id];
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::GraphPattern* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::PathPattern* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::PathChain* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Node* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Edge* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::ElementFiller* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::WhereClause* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::OrderByField* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::PathModePrefix* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::PathSearchPrefix* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SingleLabel* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::LabelOr* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::LabelAnd* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::LabelNot* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::PropStruct* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::YieldField* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::TableFunctionClause* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::ReadConsistency* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::AllowAnonymousTable* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::OpConcurrent* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SetAllProperties* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SetLabel* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsNull* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsDirected* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsNormalized* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsSourceOf* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsDestinationOf* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::IsLabeled* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Same* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::AllDifferent* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Exists* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::ProcedureBody* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SchemaFromPath* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BindingValue* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BindingGraph* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::BindingTable* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::StatementWithYield* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::QueryStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::JoinQueryExpression* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::JoinRightPart* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::CompositeQueryStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::AmbientLinearQueryStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SelectStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::FocusedQueryStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::FocusedResultStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::MatchStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::FilterStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::PrimitiveResultStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::CatalogModifyStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::LinearDataModifyingStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::InsertStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::ReplaceStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::SetStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::DeleteStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::RemoveStatement* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::OtherWise* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Union* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Except* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::Intersect* node) {
    NOT_SUPPORT_AND_THROW();
}

std::any cypher::AstExprEvaluator::visit(geax::frontend::UpdateProperties* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::visit(geax::frontend::SetSingleProperty* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::visit(geax::frontend::Param* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::visit(geax::frontend::ShowProcessListStatement* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::visit(geax::frontend::KillStatement* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::visit(geax::frontend::ManagerStatement* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::visit(geax::frontend::DummyNode* node) {
    NOT_SUPPORT_AND_THROW();
}
std::any cypher::AstExprEvaluator::reportError() { return error_msg_; }

}  // namespace cypher
