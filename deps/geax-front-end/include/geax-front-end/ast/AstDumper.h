/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#ifndef GEAXFRONTEND_UTILS_ASTDUMPER_H_
#define GEAXFRONTEND_UTILS_ASTDUMPER_H_

#include <iostream>
#include <string>
#include <tuple>

#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstNodeVisitor.h"

namespace geax {
namespace frontend {

#ifndef VISIT_AND_CHECK
#define VISIT_AND_CHECK(ast)                                 \
    do {                                                     \
        auto res = std::any_cast<GEAXErrorCode>(visit(ast)); \
        if (res != GEAXErrorCode::GEAX_SUCCEED) {            \
            return res;                                      \
        }                                                    \
    } while (0)
#endif

#ifndef VISIT_AND_CHECK_WITH_MSG
#define VISIT_AND_CHECK_WITH_MSG(ast)                                                     \
    do {                                                                                  \
        auto res = std::any_cast<GEAXErrorCode>(visit(ast));                              \
        if (res != GEAXErrorCode::GEAX_SUCCEED) {                                         \
            error_msg_ = error_msg_.empty()                                               \
                             ? "visit(" + std::string(#ast) + ") failed at" +             \
                                   std::string(__FILE__) + ":" + std::to_string(__LINE__) \
                             : error_msg_;                                                \
            return res;                                                                   \
        }                                                                                 \
    } while (0)
#endif

#ifndef VISIT_PARAM_AND_CHECK_WITH_MSG
#define VISIT_PARAM_AND_CHECK_WITH_MSG(ast) \
    cur_var_ = std::string(#ast);           \
    VISIT_AND_CHECK_WITH_MSG(ast)
#endif

class IndentGuard {
 public:
    explicit IndentGuard(int64_t* indent, const int64_t& indent_size)
        : indent_(indent), indent_size_(indent_size) {
        *indent_ += indent_size_;
    }
    ~IndentGuard() { *indent_ -= indent_size_; }

 private:
    int64_t* indent_;
    int64_t indent_size_;
};

class VariableGuard {
 public:
    explicit VariableGuard(std::string& str, int64_t* indent, const std::string& var,
                           const std::string& var_type_name, bool is_list = false,
                           bool is_newline = true)
        : str_(str), indent_(indent), is_list_(is_list), is_newline_(is_newline) {
        str_.append(std::string(*indent, ' '))
            .append(var.empty() ? var : var + " = ")
            .append(var_type_name)
            .append(is_list_ ? "[" : "(")
            .append(is_newline_ ? "\n" : "");
    }

    ~VariableGuard() {
        str_.append(std::string(*indent_, ' '))
            .append(is_list_ ? "]" : ")")
            .append(",")
            .append(is_newline_ ? "\n" : "");
    }

 private:
    std::string& str_;
    int64_t* indent_;
    bool is_list_;
    bool is_newline_;
};

#ifndef INDET_GUARD
#define INDET_GUARD() IndentGuard guard(&cur_indent_, indent_size_);
#endif

#ifndef VARIABLE_GUARD
#define VARIABLE_GUARD_WITH_TYPE_NAME(type_name)                \
    VariableGuard vg(str_, &cur_indent_, cur_var_, #type_name); \
    cur_var_.clear()
#define VARIABLE_GUARD() VARIABLE_GUARD_WITH_TYPE_NAME()
#endif

class AstDumper : public AstNodeVisitor {
 public:
    AstDumper() = default;

    ~AstDumper() = default;

    GEAXErrorCode handle(AstNode* node) {
        str_ = "";
        indent_size_ = 2;
        cur_indent_ = -indent_size_;
        return std::any_cast<GEAXErrorCode>(node->accept(*this));
    }

    std::string dump() { return str_; }

    std::string error_msg() { return error_msg_; }

    template <typename T>
    std::any visit(const std::vector<T>& v) {
        if (v.empty()) return GEAXErrorCode::GEAX_SUCCEED;
        INDET_GUARD();
        VariableGuard vg(str_, &cur_indent_, cur_var_, "", true);
        cur_var_.clear();
        for (auto t : v) {
            VISIT_AND_CHECK_WITH_MSG(t);
        }
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    template <typename T>
    std::any visit(const std::optional<T>& t) {
        if (!t.has_value()) {
            return GEAXErrorCode::GEAX_SUCCEED;
        }
        INDET_GUARD();
        VARIABLE_GUARD();
        if (t.has_value()) {
            VISIT_AND_CHECK_WITH_MSG(t.value());
        } else {
            INDET_GUARD();
            str_.append(std::string(cur_indent_, ' ')).append("null\n");
        }
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    template <typename T1, typename T2>
    std::any visit(const std::tuple<T1, T2>& t) {
        INDET_GUARD();
        VARIABLE_GUARD();
        cur_var_ = "tuple<0>";
        VISIT_AND_CHECK_WITH_MSG(std::get<0>(t));
        cur_var_ = "tuple<1>";
        VISIT_AND_CHECK_WITH_MSG(std::get<1>(t));
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    template <typename T1, typename T2>
    std::any visit(const std::variant<T1, T2>& t) {
        INDET_GUARD();
        VARIABLE_GUARD();
        if (std::holds_alternative<T1>(t)) {
            VISIT_AND_CHECK_WITH_MSG(std::get<T1>(t));
        } else if (std::holds_alternative<T2>(t)) {
            VISIT_AND_CHECK_WITH_MSG(std::get<T2>(t));
        }
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    template <typename T>
    std::any reportError(T) {
        error_msg_ = error_msg_.empty() ? "visit(" + std::string(typeid(T).name()) + ") failed at" +
                                              std::string(__FILE__) + ":" + std::to_string(__LINE__)
                                        : error_msg_;
        return reportError();
    }

    std::any visit(const std::string& node) {
        INDET_GUARD();
        str_.append(std::string(cur_indent_, ' '))
            .append(cur_var_.empty() ? cur_var_ : cur_var_ + " = ")
            .append(node)
            .append(",\n");
        cur_var_.clear();
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(const char* node) {
        INDET_GUARD();
        str_.append(std::string(cur_indent_, ' '))
            .append(cur_var_.empty() ? cur_var_ : cur_var_ + " = ")
            .append(node)
            .append(",\n");
        cur_var_.clear();
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(bool node) { return visit(std::to_string(node)); }

    std::any visit(int64_t node) { return visit(std::to_string(node)); }

    std::any visit(double node) { return visit(std::to_string(node)); }

    std::any visit(TransactionMode node) { return visit(ToString(node)); }

    std::any visit(StatementMode node) { return visit(ToString(node)); }

    std::any visit(MatchMode node) { return visit(ToString(node)); }

    std::any visit(ExprConjunctionType node) { return visit(ToString(node)); }

    std::any visit(EdgeDirection node) { return visit(ToString(node)); }

    std::any visit(GeneralSetFunction node) { return visit(ToString(node)); }

    std::any visit(BinarySetFunction node) { return visit(ToString(node)); }

    std::any visit(ModeType node) { return visit(ToString(node)); }

    std::any visit(SearchType node) { return visit(ToString(node)); }

    std::any visit(QueryJoinType node) { return visit(ToString(node)); }

    std::any visit(Statement* node) { return node->accept(*this); }

    std::any visit(SessionParamType node) { return visit(ToString(node)); }

    std::any visit(LinearQueryStatement* node) { return node->accept(*this); }

    std::any visit(SimpleQueryStatement* node) { return node->accept(*this); }

    std::any visit(PrimitiveDataModifyStatement* node) { return node->accept(*this); }

    std::any visit(Expr* node) { return node->accept(*this); }

    std::any visit(LabelTree* node) { return node->accept(*this); }

    std::any visit(ElementPredicate* node) { return node->accept(*this); }

    std::any visit(PathPrefix* node) { return node->accept(*this); }

    std::any visit(EdgeLike* node) { return node->accept(*this); }

    std::any visit(SingleCatalogStatement* node) { return node->accept(*this); }

    std::any visit(SetItem* node) { return node->accept(*this); }

    std::any visit(BindingDefinition* node) { return node->accept(*this); }

    std::any visit(BindingTableExpr* node) { return node->accept(*this); }

    std::any visit(Session* node) { return node->accept(*this); }

    std::any visit(Transaction* node) { return node->accept(*this); }

    std::any visit(SessionSetCommand* node) { return node->accept(*this); }

    std::any visit(SessionResetCommand* node) { return node->accept(*this); }

    std::any visit(EndTransaction* node) { return node->accept(*this); }
    std::any visit(ProcedureCall* node) { return node->accept(*this); }

    //---------------------------------------------------------------------------------
    // patterns
    std::any visit(GraphPattern* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(GraphPattern);
        auto& path_patterns = node->pathPatterns();
        auto& mode = node->matchMode();
        auto& keep = node->keep();
        auto& where = node->where();
        auto& yield = node->yield();
        VISIT_PARAM_AND_CHECK_WITH_MSG(path_patterns);
        VISIT_PARAM_AND_CHECK_WITH_MSG(mode);
        VISIT_PARAM_AND_CHECK_WITH_MSG(keep);
        VISIT_PARAM_AND_CHECK_WITH_MSG(where);
        VISIT_PARAM_AND_CHECK_WITH_MSG(yield);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(PathPattern* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(PathPattern);
        auto& alias = node->alias();
        auto& prefix = node->prefix();
        auto& chains = node->chains();
        auto& op_type = node->opType();
        VISIT_PARAM_AND_CHECK_WITH_MSG(alias);
        VISIT_PARAM_AND_CHECK_WITH_MSG(prefix);
        VISIT_PARAM_AND_CHECK_WITH_MSG(chains);
        VISIT_PARAM_AND_CHECK_WITH_MSG(op_type);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(PathChain* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(PathChain);
        auto head = node->head();
        auto& tails = node->tails();
        VISIT_PARAM_AND_CHECK_WITH_MSG(head);
        VISIT_PARAM_AND_CHECK_WITH_MSG(tails);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Node* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Node);
        auto filler = node->filler();
        VISIT_PARAM_AND_CHECK_WITH_MSG(filler);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Edge* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME();
        auto direction = node->direction();
        auto filler = node->filler();
        auto& hop_range = node->hopRange();
        VISIT_PARAM_AND_CHECK_WITH_MSG(direction);
        VISIT_PARAM_AND_CHECK_WITH_MSG(filler);
        VISIT_PARAM_AND_CHECK_WITH_MSG(hop_range);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ElementFiller* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ElementFiller);
        auto& v = node->v();
        auto& label = node->label();
        auto& element_predicate = node->predicates();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(label);
        VISIT_PARAM_AND_CHECK_WITH_MSG(element_predicate);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    //---------------------------------------------------------------------------------
    // clauses
    std::any visit(WhereClause* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME();
        auto predicate = node->predicate();
        VISIT_PARAM_AND_CHECK_WITH_MSG(predicate);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(OrderByField* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME();
        auto field = node->field();
        auto order = node->order();
        auto& null_order = node->nullOrder();
        VISIT_PARAM_AND_CHECK_WITH_MSG(field);
        VISIT_PARAM_AND_CHECK_WITH_MSG(order);
        VISIT_PARAM_AND_CHECK_WITH_MSG(null_order);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(PathModePrefix* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(PathModePrefix);
        auto path_mode = node->pathMode();
        VISIT_PARAM_AND_CHECK_WITH_MSG(path_mode);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(PathSearchPrefix* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(PathSearchPrefix);
        auto& num = node->num();
        auto search_type = node->searchType();
        auto& search_mode = node->searchMode();
        VISIT_PARAM_AND_CHECK_WITH_MSG(num);
        VISIT_PARAM_AND_CHECK_WITH_MSG(search_type);
        VISIT_PARAM_AND_CHECK_WITH_MSG(search_mode);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SingleLabel* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SingleLabel);
        auto& label = node->label();
        VISIT_PARAM_AND_CHECK_WITH_MSG(label);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(LabelOr* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(LabelOr);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(LabelAnd* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(LabelAnd);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(LabelNot* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(LabelNot);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(PropStruct* node) override {
        if (node) {
            INDET_GUARD();
            VARIABLE_GUARD_WITH_TYPE_NAME();
            auto& properties = node->properties();
            VISIT_PARAM_AND_CHECK_WITH_MSG(properties);
        }
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(YieldField* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME();
        auto& items = node->items();
        VISIT_AND_CHECK_WITH_MSG(items);
        auto predicate = node->predicate();
        if (predicate) {
            VISIT_PARAM_AND_CHECK_WITH_MSG(predicate);
        }
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(TableFunctionClause* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME();
        auto function = node->function();
        VISIT_PARAM_AND_CHECK_WITH_MSG(function);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ReadConsistency* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ReadConsistency);
        auto& val = node->val();
        VISIT_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(AllowAnonymousTable*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(AllowAnonymousTable);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(OpConcurrent*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(OpConcurrent);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(EdgeOnJoin* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(EdgeOnJoin);
        auto& edge = node->edge();
        auto& joinKey = node->joinKey();
        VISIT_AND_CHECK_WITH_MSG(edge);
        VISIT_AND_CHECK_WITH_MSG(joinKey);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SetAllProperties* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SetAllProperties);
        auto& v = node->v();
        auto structs = node->structs();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(structs);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(UpdateProperties* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(UpdateProperties);
        auto& v = node->v();
        auto structs = node->structs();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(structs);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SetSingleProperty* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SetSingleProperty);
        auto& v = node->v();
        auto& property = node->property();
        auto value = node->value();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(property);
        VISIT_PARAM_AND_CHECK_WITH_MSG(value);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(RemoveSingleProperty* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(RemoveSingleProperty);
        auto& v = node->v();
        auto& property = node->property();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(property);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SetLabel* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(GetField);
        auto& v = node->v();
        auto label = node->label();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(label);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(OtherWise*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(OtherWise);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Union* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Union);
        auto distinct = node->distinct();
        VISIT_PARAM_AND_CHECK_WITH_MSG(distinct);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Except* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Except);
        auto distinct = node->distinct();
        VISIT_PARAM_AND_CHECK_WITH_MSG(distinct);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Intersect* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Intersect);
        auto distinct = node->distinct();
        VISIT_PARAM_AND_CHECK_WITH_MSG(distinct);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SchemaFromPath* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SchemaFromPath);
        auto path = node->path();
        VISIT_PARAM_AND_CHECK_WITH_MSG(path);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SetSchemaClause* node) override { return reportError(node); }
    std::any visit(SetGraphClause* node) override { return reportError(node); }
    std::any visit(SetTimeZoneClause* node) override { return reportError(node); }
    std::any visit(SetParamClause* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SetParamClause);
        auto paramType = node->paramType();
        auto& name = node->name();
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(paramType);
        VISIT_PARAM_AND_CHECK_WITH_MSG(name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ResetAll* node) override { return reportError(node); }
    std::any visit(ResetSchema* node) override { return reportError(node); }
    std::any visit(ResetTimeZone* node) override { return reportError(node); }
    std::any visit(ResetGraph* node) override { return reportError(node); }
    std::any visit(ResetParam* node) override { return reportError(node); }

    //---------------------------------------------------------------------------------
    // exprs
    std::any visit(GetField* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(GetField);
        auto expr = node->expr();
        auto& field_name = node->fieldName();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        VISIT_PARAM_AND_CHECK_WITH_MSG(field_name);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(TupleGet* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(TupleGet);
        auto expr = node->expr();
        auto index = node->index();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        VISIT_PARAM_AND_CHECK_WITH_MSG(index);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Not* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Not);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Neg* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Neg);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Tilde* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Tilde);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VSome* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VSome);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(BEqual* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BEqual);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BNotEqual* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BNotEqual);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BGreaterThan* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BGreaterThan);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BNotSmallerThan* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BNotSmallerThan);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BSmallerThan* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BSmallerThan);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BNotGreaterThan* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BNotGreaterThan);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BSafeEqual* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BSafeEqual);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BAdd* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BAdd);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BSub* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BSub);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BDiv* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BDiv);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BMul* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BMul);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BMod* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BMod);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BSquare* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BSquare);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BAnd* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BAnd);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BOr* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BOr);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BXor* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BXor);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BBitAnd* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BBitAnd);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BBitOr* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BBitOr);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BBitXor* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BBitXor);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BBitLeftShift* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BBitLeftShift);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BBitRightShift* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BBitRightShift);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BConcat* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BConcat);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BIndex* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BIndex);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BLike* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BLike);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BIn* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BIn);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(If* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(If);
        auto condition = node->condition();
        auto true_body = node->trueBody();
        auto false_body = node->falseBody();
        VISIT_PARAM_AND_CHECK_WITH_MSG(condition);
        VISIT_PARAM_AND_CHECK_WITH_MSG(true_body);
        VISIT_PARAM_AND_CHECK_WITH_MSG(false_body);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Function* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Function);
        auto& name = node->name();
        auto& args = node->args();
        VISIT_PARAM_AND_CHECK_WITH_MSG(name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(args);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Case* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Case);
        auto& input = node->input();
        auto& case_bodies = node->caseBodies();
        auto& else_body = node->elseBody();
        VISIT_PARAM_AND_CHECK_WITH_MSG(input);
        VISIT_PARAM_AND_CHECK_WITH_MSG(case_bodies);
        VISIT_PARAM_AND_CHECK_WITH_MSG(else_body);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Cast* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Cast);
        auto expr = node->expr();
        auto& cast_type = node->castType();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        VISIT_PARAM_AND_CHECK_WITH_MSG(cast_type);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MatchCase* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MatchCase);
        auto input = node->input();
        auto& cases = node->cases();
        VISIT_PARAM_AND_CHECK_WITH_MSG(input);
        VISIT_PARAM_AND_CHECK_WITH_MSG(cases);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(AggFunc* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(AggFunc);
        auto func_name = node->funcName();
        auto is_distinct = node->isDistinct();
        auto expr = node->expr();
        auto& distinct_by = node->distinctBy();
        VISIT_PARAM_AND_CHECK_WITH_MSG(func_name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(is_distinct);
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        VISIT_PARAM_AND_CHECK_WITH_MSG(distinct_by);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BAggFunc* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BAggFunc);
        auto func_name = node->funcName();
        auto& lef = node->lExpr();
        auto rig = node->rExpr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(func_name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MultiCount* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MultiCount);
        auto& args = node->args();
        VISIT_PARAM_AND_CHECK_WITH_MSG(args);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Windowing* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME();
        auto& partition_by = node->partitionBy();
        auto& order_by = node->orderByClause();
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(partition_by);
        VISIT_PARAM_AND_CHECK_WITH_MSG(order_by);
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(MkList* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MkList);
        auto& elems = node->elems();
        VISIT_PARAM_AND_CHECK_WITH_MSG(elems);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MkMap* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MkMap);
        auto& elems = node->elems();
        VISIT_PARAM_AND_CHECK_WITH_MSG(elems);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MkRecord* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MkRecord);
        auto& elems = node->elems();
        VISIT_PARAM_AND_CHECK_WITH_MSG(elems);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MkSet* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MkSet);
        auto& elems = node->elems();
        VISIT_PARAM_AND_CHECK_WITH_MSG(elems);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MkTuple* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MkTuple);
        auto& elems = node->elems();
        VISIT_PARAM_AND_CHECK_WITH_MSG(elems);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VBool* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VBool);
        auto val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VInt* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VInt);
        auto val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VDouble* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VDouble);
        auto val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VString* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VString);
        auto& val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VDate* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VDate);
        auto& val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VDatetime* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VDatetime);
        auto& val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VDuration* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VDuration);
        auto& val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VTime* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VTime);
        auto& val = node->val();
        VISIT_PARAM_AND_CHECK_WITH_MSG(val);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VNull*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VNull);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(VNone*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(VNone);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Ref* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Ref);
        auto& name = node->name();
        VISIT_PARAM_AND_CHECK_WITH_MSG(name);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Param* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Param);
        auto& name = node->name();
        VISIT_PARAM_AND_CHECK_WITH_MSG(name);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    // predicates
    std::any visit(IsNull* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(IsNull);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(IsDirected* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(IsDirected);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(IsNormalized* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(IsNormalized);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(IsSourceOf* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(IsSourceOf);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(IsDestinationOf* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(IsDestinationOf);
        auto lef = node->left();
        auto rig = node->right();
        VISIT_PARAM_AND_CHECK_WITH_MSG(lef);
        VISIT_PARAM_AND_CHECK_WITH_MSG(rig);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(IsLabeled* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(IsLabeled);
        auto expr = node->expr();
        auto label_tree = node->labelTree();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        VISIT_PARAM_AND_CHECK_WITH_MSG(label_tree);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Same* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Same);
        auto& items = node->items();
        VISIT_PARAM_AND_CHECK_WITH_MSG(items);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(AllDifferent* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(AllDifferent);
        auto& items = node->items();
        VISIT_PARAM_AND_CHECK_WITH_MSG(items);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(Exists* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(Exists);
        auto& path_chains = node->pathChains();
        VISIT_PARAM_AND_CHECK_WITH_MSG(path_chains);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    //---------------------------------------------------------------------------------
    // stmt
    std::any visit(ExplainActivity* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ExplainActivity);
        auto is_profile = node->isProfile();
        auto& format = node->format();
        auto stmts = node->procedureBody();
        VISIT_PARAM_AND_CHECK_WITH_MSG(is_profile);
        VISIT_PARAM_AND_CHECK_WITH_MSG(format);
        VISIT_PARAM_AND_CHECK_WITH_MSG(stmts);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ProcedureBody* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ProcedureBody);
        auto& schema_ref = node->schemaRef();
        auto& binding_defs = node->bindingDefinitions();
        auto& stmts = node->statements();
        VISIT_PARAM_AND_CHECK_WITH_MSG(schema_ref);
        VISIT_PARAM_AND_CHECK_WITH_MSG(binding_defs);
        VISIT_PARAM_AND_CHECK_WITH_MSG(stmts);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BindingValue*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BindingValue);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BindingGraph*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BindingGraph);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BindingTable* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BindingTable);
        auto& var_name = node->varName();
        auto& types = node->types();
        auto query = node->query();
        VISIT_PARAM_AND_CHECK_WITH_MSG(var_name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(types);
        VISIT_PARAM_AND_CHECK_WITH_MSG(query);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BindingTableInnerQuery* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BindingTableInnerQuery);
        auto body = node->body();
        VISIT_PARAM_AND_CHECK_WITH_MSG(body);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(BindingTableInnerExpr* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(BindingTableInnerExpr);
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(StatementWithYield* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(StatementWithYield);
        auto& yield = node->yield();
        auto stmt = node->statement();
        VISIT_PARAM_AND_CHECK_WITH_MSG(yield);
        VISIT_PARAM_AND_CHECK_WITH_MSG(stmt);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(QueryStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(QueryStatement);
        auto join_query = node->joinQuery();
        VISIT_PARAM_AND_CHECK_WITH_MSG(join_query);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(JoinQueryExpression* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(JoinQueryExpression);
        auto head = node->head();
        auto& body = node->body();
        VISIT_PARAM_AND_CHECK_WITH_MSG(head);
        VISIT_PARAM_AND_CHECK_WITH_MSG(body);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(JoinRightPart* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(JoinQueryExpression);
        auto join_type = node->joinType();
        auto right_body = node->rightBody();
        auto on = node->onExpr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(join_type);
        VISIT_PARAM_AND_CHECK_WITH_MSG(right_body);
        VISIT_PARAM_AND_CHECK_WITH_MSG(on);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(CompositeQueryStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(CompositeQueryStatement);
        auto head = node->head();
        auto& body = node->body();
        VISIT_PARAM_AND_CHECK_WITH_MSG(head);
        VISIT_PARAM_AND_CHECK_WITH_MSG(body);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(AmbientLinearQueryStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(AmbientLinearQueryStatement);
        auto result_stmt = node->resultStatement();
        auto& query_stmts = node->queryStatements();
        VISIT_PARAM_AND_CHECK_WITH_MSG(result_stmt);
        VISIT_PARAM_AND_CHECK_WITH_MSG(query_stmts);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SelectStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SelectStatement);
        auto result_stmt = node->resultStatement();
        auto& from_clauses = node->fromClauses();
        auto& where = node->where();
        auto& having = node->having();
        VISIT_PARAM_AND_CHECK_WITH_MSG(result_stmt);
        VISIT_PARAM_AND_CHECK_WITH_MSG(from_clauses);
        VISIT_PARAM_AND_CHECK_WITH_MSG(where);
        VISIT_PARAM_AND_CHECK_WITH_MSG(having);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(FocusedQueryStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(FocusedQueryStatement);
        auto& query_list = node->queryList();
        auto result_stmt = node->resultStatement();
        VISIT_PARAM_AND_CHECK_WITH_MSG(query_list);
        VISIT_PARAM_AND_CHECK_WITH_MSG(result_stmt);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(FocusedResultStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(FocusedResultStatement);
        auto& graph_ref = node->graphRef();
        auto result_stmt = node->resultStatement();
        VISIT_PARAM_AND_CHECK_WITH_MSG(graph_ref);
        VISIT_PARAM_AND_CHECK_WITH_MSG(result_stmt);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MatchStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MatchStatement);
        auto graph_pattern = node->graphPattern();
        auto& statement_mode = node->statementMode();
        VISIT_PARAM_AND_CHECK_WITH_MSG(graph_pattern);
        VISIT_PARAM_AND_CHECK_WITH_MSG(statement_mode);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(FilterStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(FilterStatement);
        auto predicate = node->predicate();
        VISIT_PARAM_AND_CHECK_WITH_MSG(predicate);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ForStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ForStatement);
        auto& items = node->itemsAlias();
        auto& ordinalityOrOffset = node->ordinalityOrOffset();
        auto expr = node->expr();
        VISIT_PARAM_AND_CHECK_WITH_MSG(items);
        VISIT_PARAM_AND_CHECK_WITH_MSG(ordinalityOrOffset);
        VISIT_PARAM_AND_CHECK_WITH_MSG(expr);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(PrimitiveResultStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(PrimitiveResultStatement);
        auto distinct = node->distinct();
        auto& items = node->items();
        auto& group_keys = node->groupKeys();
        auto& order_keys = node->orderBys();
        auto& limit = node->limit();
        auto& offset = node->offset();
        auto& hints = node->hints();
        VISIT_PARAM_AND_CHECK_WITH_MSG(distinct);
        VISIT_PARAM_AND_CHECK_WITH_MSG(items);
        VISIT_PARAM_AND_CHECK_WITH_MSG(group_keys);
        VISIT_PARAM_AND_CHECK_WITH_MSG(order_keys);
        VISIT_PARAM_AND_CHECK_WITH_MSG(limit);
        VISIT_PARAM_AND_CHECK_WITH_MSG(offset);
        VISIT_PARAM_AND_CHECK_WITH_MSG(hints);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(CatalogModifyStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(CatalogModifyStatement);
        auto& stmts = node->statements();
        VISIT_PARAM_AND_CHECK_WITH_MSG(stmts);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(LinearDataModifyingStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(LinearDataModifyingStatement);
        auto& use_graph = node->useGraph();
        auto& query_stmts = node->queryStatements();
        auto& modify_stmts = node->modifyStatements();
        auto& result_stmt = node->resultStatement();
        VISIT_PARAM_AND_CHECK_WITH_MSG(use_graph);
        VISIT_PARAM_AND_CHECK_WITH_MSG(query_stmts);
        VISIT_PARAM_AND_CHECK_WITH_MSG(modify_stmts);
        VISIT_PARAM_AND_CHECK_WITH_MSG(result_stmt);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(InsertStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(InsertStatement);
        auto& paths = node->paths();
        VISIT_PARAM_AND_CHECK_WITH_MSG(paths);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ReplaceStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ReplaceStatement);
        auto& paths = node->paths();
        VISIT_PARAM_AND_CHECK_WITH_MSG(paths);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SetStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SetStatement);
        auto& items = node->items();
        VISIT_PARAM_AND_CHECK_WITH_MSG(items);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(DeleteStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(DeleteStatement);
        auto& items = node->items();
        VISIT_PARAM_AND_CHECK_WITH_MSG(items);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(RemoveStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(RemoveStatement);
        auto& items = node->items();
        for (auto &item : items) {
            VISIT_PARAM_AND_CHECK_WITH_MSG(item);
        }
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(MergeStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(MergeStatement);
        auto pathPattern = node->pathPattern();
        auto& onCreate = node->onCreate();
        auto& onMatch = node->onMatch();
        VISIT_PARAM_AND_CHECK_WITH_MSG(pathPattern);
        VISIT_PARAM_AND_CHECK_WITH_MSG(onCreate);
        VISIT_PARAM_AND_CHECK_WITH_MSG(onMatch);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ShowProcessListStatement*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(ShowProcessListStatement);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(KillStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(KillStatement);
        auto id = node->id();
        auto is_query = node->isQuery();
        VISIT_PARAM_AND_CHECK_WITH_MSG(id);
        VISIT_PARAM_AND_CHECK_WITH_MSG(is_query);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(ManagerStatement* node) override { return reportError(node); }
    std::any visit(SessionActivity* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SessionActivity);
        auto& sessions = node->sessions();
        VISIT_PARAM_AND_CHECK_WITH_MSG(sessions);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(TransactionActivity* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SessionActivity);
        auto transaction = node->transaction();
        VISIT_PARAM_AND_CHECK_WITH_MSG(transaction);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(FullTransaction* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(FullTransaction);
        auto start = node->startTransaction();
        auto normal = node->normalTransaction();
        VISIT_PARAM_AND_CHECK_WITH_MSG(start);
        VISIT_PARAM_AND_CHECK_WITH_MSG(normal);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(NormalTransaction* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(NormalTransaction);
        auto query = node->query();
        auto& end = node->endTransaction();
        VISIT_PARAM_AND_CHECK_WITH_MSG(query);
        VISIT_PARAM_AND_CHECK_WITH_MSG(end);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(StartTransaction* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(StartTransaction);
        auto& modes = node->modes();
        VISIT_PARAM_AND_CHECK_WITH_MSG(modes);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(CommitTransaction*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(CommitTransaction);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(RollBackTransaction*) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(RollBackTransaction);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SessionSet* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SessionSet);
        auto command = node->command();
        VISIT_PARAM_AND_CHECK_WITH_MSG(command);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(SessionReset* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(SessionReset);
        auto command = node->command();
        VISIT_PARAM_AND_CHECK_WITH_MSG(command);
        return GEAXErrorCode::GEAX_SUCCEED;
    }

    std::any visit(StandaloneCallStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(CallQueryStatement);
        auto procedure = node->procedureStatement();
        VISIT_PARAM_AND_CHECK_WITH_MSG(procedure);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(CallQueryStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(CallQueryStatement);
        auto procedure = node->procedureStatement();
        VISIT_PARAM_AND_CHECK_WITH_MSG(procedure);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(CallProcedureStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(CallProcedureStatement);
        auto isOption = node->isOption();
        auto procedure = node->procedureCall();
        VISIT_PARAM_AND_CHECK_WITH_MSG(isOption);
        VISIT_PARAM_AND_CHECK_WITH_MSG(procedure);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(InlineProcedureCall* node) override { return reportError(node); }
    std::any visit(NamedProcedureCall* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(NamedProcedureCall);
        auto& name = node->name();
        auto& args = node->args();
        auto& yield = node->yield();
        VISIT_PARAM_AND_CHECK_WITH_MSG(name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(args);
        VISIT_PARAM_AND_CHECK_WITH_MSG(yield);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(UnwindStatement* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(UnwindStatement);
        auto& v = node->variable();
        auto list = node->list();
        VISIT_PARAM_AND_CHECK_WITH_MSG(v);
        VISIT_PARAM_AND_CHECK_WITH_MSG(list);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(InQueryProcedureCall* node) override {
        INDET_GUARD();
        VARIABLE_GUARD_WITH_TYPE_NAME(InQueryProcedureCall);
        auto& name = node->name();
        auto& args = node->args();
        auto& yield = node->yield();
        VISIT_PARAM_AND_CHECK_WITH_MSG(name);
        VISIT_PARAM_AND_CHECK_WITH_MSG(args);
        VISIT_PARAM_AND_CHECK_WITH_MSG(yield);
        return GEAXErrorCode::GEAX_SUCCEED;
    }
    std::any visit(DummyNode* node) override { return reportError(node); }
    std::any visit(ListComprehension* node) override {
        VISIT_PARAM_AND_CHECK_WITH_MSG(node->getVariable());
        VISIT_PARAM_AND_CHECK_WITH_MSG(node->getInExpression());
        VISIT_PARAM_AND_CHECK_WITH_MSG(node->getOpExpression());
        return GEAXErrorCode::GEAX_SUCCEED;
    }

 protected:
    std::any reportError() override { return GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT; }
    std::string str_;
    std::string error_msg_;
    int64_t indent_size_;
    int64_t cur_indent_;
    std::string cur_var_;
};  // class AstNodeVisitor

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_UTILS_ASTDUMPER_H_
