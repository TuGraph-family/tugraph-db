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

// Generated from Lcypher.g4 by ANTLR 4.9.2

#pragma once

#include <cassert>
#include <tuple>
#include "antlr4-runtime/antlr4-runtime.h"
#include "parser/expression.h"
#include "parser/generated/LcypherVisitor.h"
#include "fma-common/utils.h"
#include "parser/clause.h"
#include "cypher/cypher_exception.h"
#include "procedure/procedure.h"
#include "core/defs.h"
#include "db/galaxy.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/filter/filter.h"

#if __APPLE__
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#endif  // #if __APPLE__

namespace parser {

/**
 * This class provides an empty implementation of LcypherVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class CypherBaseVisitor : public LcypherVisitor {
    cypher::RTContext *ctx_;
    CmdType _cmd_type;
    std::vector<SglQuery> _query;
    size_t _curr_query = 0;
    size_t _curr_part = 0;
    std::string _curr_procedure_name;
    /* alias carry between query parts */
    std::vector<std::pair<std::string, cypher::SymbolNode::Type>> _carry;
    std::string _listcompr_placeholder;
    std::unordered_map<std::string, std::set<std::string>> _node_property;
    std::unordered_map<std::string, std::set<std::string>> _rel_property;
    enum _ClauseType : uint32_t {
        NA = 0x0,
        MATCH = 0x1,
        RETURN = 0x2,
        WITH = 0x4,
        UNWIND = 0x8,
        WHERE = 0x10,
        ORDERBY = 0x20,
        CREATE = 0x40,
        DELETE = 0x80,
        SET = 0x100,
        REMOVE = 0x200,
        MERGE = 0x400,
        INQUERYCALL = 0x800,
    } _curr_clause_type = NA;
    /* Symbol is one of the following:
     * defined entity (node & relationship, eg. MATCH (n)-[r]->(m))
     * alias (eg. RETURN n.name AS name)
     * parameter (eg. MATCH (n {name:$name}))
     */
    size_t _symbol_idx = 0;
    /* Anonymous entity are not in symbol table:
     * MATCH (n) RETURN exists((n)-->()-->())  */
    size_t _anonymous_idx = 0;

    std::string GenAnonymousAlias(bool is_node) {
        std::string alias(ANONYMOUS);
        if (is_node) {
            alias.append("N").append(std::to_string(_anonymous_idx));
        } else {
            alias.append("R").append(std::to_string(_anonymous_idx));
        }
        _anonymous_idx++;
        return alias;
    }

    bool AddSymbol(const std::string &symbol_alias, cypher::SymbolNode::Type type,
                   cypher::SymbolNode::Scope scope) {
        if (_InClauseRETURN() ||
            (_InClauseWHERE() && !symbol_alias.empty() && symbol_alias[0] != '$')) {
            // TODO(anyone): more situations
            // where not((n)-[]->(newNodeVar))
            // where n.name = $name
            // RETURN 100 as x, [x IN range(0,10) | x] AS result
            return false;
        }
        /* add carry whether the symbol showed up or not. eg:
         * match (m) with m as m */
        if (_InClauseWITH()) _carry.emplace_back(symbol_alias, type);
        auto &alias_id_map = _query[_curr_query].parts[_curr_part].symbol_table.symbols;
        if (alias_id_map.find(symbol_alias) == alias_id_map.end()) {
            alias_id_map.emplace(symbol_alias, cypher::SymbolNode(_symbol_idx, type, scope));
            _symbol_idx++;
            return true;
        }
        return false;
    }
    void AstRewrite(cypher::RTContext *ctx);

 public:
    CypherBaseVisitor() = default;
    // TODO(anyone): consider yield items after parser visit
    CypherBaseVisitor(cypher::RTContext *ctx, antlr4::tree::ParseTree *tree) : ctx_(ctx) {
        // The default implementation of visit()
        tree->accept(this);
        // build annotations
        for (auto &q : _query) {
            for (auto &p : q.parts) p.Enrich();
        }
        AstRewrite(ctx);
    }

    /**
     * Override this method to avoid clone childResults. When the childResult is
     * !is_nothrow_copy_constructible (such as std::string), the default implementation
     * treats it as constant reference and wont'd clone its content.
     */
    std::any visitChildren(antlr4::tree::ParseTree *node) override {
        std::any result = defaultResult();
        size_t n = node->children.size();
        for (size_t i = 0; i < n; i++) {
            if (!shouldVisitNextChild(node, result)) {
                break;
            }

            std::any childResult = node->children[i]->accept(this);
            result = std::move(childResult);
        }

        return result;
    }

    const std::vector<SglQuery> &GetQuery() const { return _query; }

    const std::unordered_map<std::string, std::set<std::string>>&
    GetNodeProperty() const {return _node_property;}

    const std::unordered_map<std::string, std::set<std::string>>&
    GetRelProperty() const {return _rel_property;}

    CmdType CommandType() const { return _cmd_type; }

    std::any visitOC_Cypher(LcypherParser::OC_CypherContext *ctx) override {
        _query.clear();
        return visitChildren(ctx);
    }

    std::any visitOC_Statement(LcypherParser::OC_StatementContext *ctx) override {
        _cmd_type = ctx->EXPLAIN()   ? CmdType::EXPLAIN
                    : ctx->PROFILE() ? CmdType::PROFILE
                                     : CmdType::QUERY;
        return visitChildren(ctx);
    }

    std::any visitOC_Query(LcypherParser::OC_QueryContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_RegularQuery(LcypherParser::OC_RegularQueryContext *ctx) override {
        // reserve for single_queries
        _query.resize(ctx->oC_Union().size() + 1);
        _curr_query = 0;
        _curr_part = 0;
        _symbol_idx = 0;
        _anonymous_idx = 0;
        visit(ctx->oC_SingleQuery());
        for (auto u : ctx->oC_Union()) {
            // initialize for the next single_query
            _curr_query++;
            _curr_part = 0;
            _symbol_idx = 0;
            _anonymous_idx = 0;
            visit(u);
        }
        return 0;
    }

    std::any visitOC_Union(LcypherParser::OC_UnionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_SingleQuery(LcypherParser::OC_SingleQueryContext *ctx) override {
        // reserve for query parts
        if (ctx->oC_SinglePartQuery()) {
            _query[_curr_query].parts.resize(1);
        } else {
            int part_num = ctx->oC_MultiPartQuery()->oC_With().size() + 1;
            _query[_curr_query].parts.resize(part_num);
        }
        return visitChildren(ctx);
    }

    std::any visitOC_SinglePartQuery(
        LcypherParser::OC_SinglePartQueryContext *ctx) override {
        if (ctx->oC_ReadingClause().size() > 2) CYPHER_TODO();
        return visitChildren(ctx);
    }

    std::any visitOC_MultiPartQuery(LcypherParser::OC_MultiPartQueryContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_UpdatingClause(LcypherParser::OC_UpdatingClauseContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_ReadingClause(LcypherParser::OC_ReadingClauseContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_Match(LcypherParser::OC_MatchContext *ctx) override {
        _EnterClauseMATCH();
        bool optional = ctx->OPTIONAL_() != nullptr;
        std::vector<TUP_PATTERN_PART> pattern =
            std::any_cast<std::vector<TUP_PATTERN_PART>>(visit(ctx->oC_Pattern()));
        VEC_STR hints;
        for (auto &hint : ctx->oC_Hint()) {
            std::string h = std::any_cast<std::string>(visit(hint));
            hints.emplace_back(h);
        }
        Expression where;
        if (ctx->oC_Where()) where = std::any_cast<Expression>(visit(ctx->oC_Where()));
        Clause clause;
        clause.type = Clause::MATCH;
        clause.data = std::make_shared<Clause::TYPE_MATCH>(std::move(pattern), std::move(hints),
                                                           std::move(where), optional);
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseMATCH();
        return 0;
    }

    std::any visitOC_Unwind(LcypherParser::OC_UnwindContext *ctx) override {
        _EnterClauseUNWIND();
        Expression e = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        std::string variable = std::any_cast<std::string>(visit(ctx->oC_Variable()));
        CYPHER_THROW_ASSERT(!variable.empty());
        /* Set scope flag according to the list expression. e.g.:
         * WITH ['Shanghai','Beijing'] AS cids
         * UNWIND cids AS cid MATCH (n {id:cid}) RETURN n
         */
        auto var_scope = cypher::SymbolNode::Scope::LOCAL;
        auto &alias_id_map = _query[_curr_query].parts[_curr_part].symbol_table.symbols;
        std::unordered_set<std::string> alias_id_set;
        for (auto &alias_pair : alias_id_map) {
            alias_id_set.emplace(alias_pair.first);
        }
        if (e.ContainAlias(alias_id_set)) {
            var_scope = cypher::SymbolNode::DERIVED_ARGUMENT;
        }
        AddSymbol(variable, cypher::SymbolNode::CONSTANT, var_scope);
        Clause clause;
        clause.type = Clause::UNWIND;
        clause.data = std::make_shared<Clause::TYPE_UNWIND>(e, variable);
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseUNWIND();
        return 0;
    }

    std::any visitOC_Merge(LcypherParser::OC_MergeContext *ctx) override {
        _EnterClauseMERGE();
        TUP_PATTERN_PART pattern_part;
        VEC_SET on_match_items;
        VEC_SET on_create_items;
        pattern_part = std::any_cast<TUP_PATTERN_PART>(visit(ctx->oC_PatternPart()));
        for (auto act : ctx->oC_MergeAction()) {
            TUP_SET_ITEM item;
            for (auto &set_item : (act->oC_Set())->oC_SetItem()) {
                item = std::any_cast<TUP_SET_ITEM>(visit(set_item));
                if (act->MATCH()) {
                    on_match_items.emplace_back(item);
                }
                if (act->CREATE()) {
                    on_create_items.emplace_back(item);
                }
            }
        }
        Clause::TYPE_MERGE merge_pattern_part_action =
            std::make_tuple(pattern_part, on_match_items, on_create_items);
        Clause clause;
        clause.type = Clause::MERGE;
        clause.data = std::make_shared<Clause::TYPE_MERGE>(std::move(merge_pattern_part_action));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseMERGE();
        return 0;
    }

    std::any visitOC_MergeAction(LcypherParser::OC_MergeActionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_Create(LcypherParser::OC_CreateContext *ctx) override {
        _EnterClauseCREATE();
        std::vector<TUP_PATTERN_PART> pattern =
            std::any_cast<std::vector<TUP_PATTERN_PART>>(visit(ctx->oC_Pattern()));
        Clause clause;
        clause.type = Clause::CREATE;
        clause.data = std::make_shared<Clause::TYPE_CREATE>(std::move(pattern));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseCREATE();
        return 0;
    }

    std::any visitOC_Set(LcypherParser::OC_SetContext *ctx) override {
        _EnterClauseSET();
        VEC_SET pattern;
        for (auto &item : ctx->oC_SetItem()) {
            TUP_SET_ITEM set_item = std::any_cast<TUP_SET_ITEM>(visit(item));
            pattern.emplace_back(set_item);
        }
        Clause clause;
        clause.type = Clause::SET;
        clause.data = std::make_shared<VEC_SET>(std::move(pattern));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseSET();
        return 0;
    }

    std::any visitOC_SetItem(LcypherParser::OC_SetItemContext *ctx) override {
        std::string variable;
        Expression property_expr;
        std::string sign;
        Expression expr;
        VEC_STR node_labels;
        if (ctx->oC_PropertyExpression()) {
            property_expr = std::any_cast<Expression>(visit(ctx->oC_PropertyExpression()));
            sign = "=";
            expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        } else {
            variable = ctx->oC_Variable()->getText();
            if (!_IsVariableDefined(variable)) {
                THROW_CODE(InputError, "Variable `{}` not defined", variable);
            }
            for (auto &c : ctx->children) {
                if (c->getText() == "+=" || c->getText() == "=") {
                    sign = c->getText();
                    break;
                }
            }
            if (ctx->oC_Expression()) {
                expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
            } else {
                VEC_STR tmp = std::any_cast<VEC_STR>(visit(ctx->oC_NodeLabels()));
                node_labels = std::move(tmp);
            }
        }
        TUP_SET_ITEM set_item = std::make_tuple(variable, property_expr, sign, expr, node_labels);
        return set_item;
    }

    std::any visitOC_Delete(LcypherParser::OC_DeleteContext *ctx) override {
        _EnterClauseDELETE();
        std::vector<Expression> expr;
        for (auto &ex : ctx->oC_Expression()) {
            expr.emplace_back(std::any_cast<Expression>(visit(ex)));
            if (expr.back().type != Expression::VARIABLE) {
                THROW_CODE(InputError, "Type mismatch: expected Node, Path or Relationship");
            }
            const auto &variable = expr.back().ToString();
            const auto &symbols = _query[_curr_query].parts[_curr_part].symbol_table.symbols;
            // variable not defined should be handled as a exception in visitOC_Atom()
            // so assert that variable must be found in symbols
            auto it = symbols.find(variable);
            CYPHER_THROW_ASSERT(it != symbols.end());
            auto variable_type = it->second.type;
            if (variable_type != cypher::SymbolNode::NODE &&
                variable_type != cypher::SymbolNode::RELATIONSHIP &&
                variable_type != cypher::SymbolNode::NAMED_PATH) {
                THROW_CODE(InputError, "Type mismatch: expected Node, Path or Relationship");
            }
        }
        Clause clause;
        clause.type = Clause::DELETE_;
        clause.data = std::make_shared<VEC_DEL>(std::move(expr));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseDELETE();
        return 0;
    }

    std::any visitOC_Remove(LcypherParser::OC_RemoveContext *ctx) override {
        _EnterClauseREMOVE();
        std::vector<Expression> items;
        for (auto &ri : ctx->oC_RemoveItem()) {
            items.emplace_back(std::any_cast<Expression>(visit(ri)));
        }
        Clause clause;
        clause.type = Clause::REMOVE;
        clause.data = std::make_shared<Clause::TYPE_REMOVE>(std::move(items));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseREMOVE();
        return 0;
    }

    std::any visitOC_RemoveItem(LcypherParser::OC_RemoveItemContext *ctx) override {
        if (ctx->oC_PropertyExpression()) {
            auto pe_ctx = ctx->oC_PropertyExpression();
            Expression atom = std::any_cast<Expression>(visit(pe_ctx->oC_Atom()));
            if (atom.type != Expression::VARIABLE || pe_ctx->oC_PropertyLookup().empty())
                CYPHER_TODO();
            std::string property = std::any_cast<std::string>(visit(pe_ctx->oC_PropertyLookup(0)));
            Expression expr;
            expr.type = Expression::PROPERTY;
            expr.data = std::make_shared<Expression::EXPR_TYPE_PROPERTY>(std::move(atom),
                                                                         std::move(property));
            return expr;
        } else {
            CYPHER_TODO();
        }
    }

    std::any visitOC_InQueryCall(LcypherParser::OC_InQueryCallContext *ctx) override {
        _EnterClauseINQUERYCALL();
        std::tuple<std::string, std::vector<Expression>> invocation =
            std::any_cast<std::tuple<std::string, std::vector<Expression>>>(
                visit(ctx->oC_ExplicitProcedureInvocation()));
        _curr_procedure_name = std::get<0>(invocation);
        std::vector<std::pair<std::string, lgraph_api::LGraphType>> yield_items;
        Expression where;

        if (ctx->oC_YieldItems()) {
            auto yield_items_and_where = std::any_cast<std::tuple<
                std::vector<std::pair<std::string, lgraph_api::LGraphType>>, Expression>>(
                visit(ctx->oC_YieldItems()));
            yield_items = std::get<0>(yield_items_and_where);
            where = std::get<1>(yield_items_and_where);
        }
        std::string procedure_name = std::get<0>(invocation);
        TUP_CALL call = std::make_tuple(std::get<0>(invocation), std::get<1>(invocation),
                                        std::move(yield_items), where);
        Clause clause;
        clause.type = Clause::INQUERYCALL;
        clause.data = std::make_shared<Clause::TYPE_CALL>(std::move(call));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _curr_procedure_name = "";
        _LeaveClauseINQUERYCALL();
        return 0;
    }

    std::any visitOC_StandaloneCall(LcypherParser::OC_StandaloneCallContext *ctx) override {
        // reserve for the 1st query
        _query.resize(1);
        // reserve for the 1st part
        _query[0].parts.resize(1);
        _curr_query = 0;
        _curr_part = 0;
        _symbol_idx = 0;
        _anonymous_idx = 0;

        std::tuple<std::string, std::vector<Expression>> invocation;
        if (ctx->oC_ImplicitProcedureInvocation()) {
            invocation = std::any_cast<std::tuple<std::string, std::vector<Expression>>>(
                visit(ctx->oC_ImplicitProcedureInvocation()));
        } else {
            invocation = std::any_cast<std::tuple<std::string, std::vector<Expression>>>(
                visit(ctx->oC_ExplicitProcedureInvocation()));
        }
        _curr_procedure_name = std::get<0>(invocation);
        std::vector<std::pair<std::string, lgraph_api::LGraphType>> yield_items;
        Expression where;

        if (ctx->oC_YieldItems()) {
            auto yield_items_and_where = std::any_cast<std::tuple<
                std::vector<std::pair<std::string, lgraph_api::LGraphType>>, Expression>>(
                visit(ctx->oC_YieldItems()));
            yield_items = std::get<0>(yield_items_and_where);
            where = std::get<1>(yield_items_and_where);
        }
        TUP_CALL call = std::make_tuple(std::get<0>(invocation), std::get<1>(invocation),
                                        std::move(yield_items), where);
        Clause clause;
        clause.type = Clause::STANDALONECALL;
        clause.data = std::make_shared<Clause::TYPE_CALL>(std::move(call));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _curr_procedure_name = "";
        return 0;
    }

    std::any visitOC_YieldItems(LcypherParser::OC_YieldItemsContext *ctx) override {
        std::vector<std::string> yield_items;
        for (auto item : ctx->oC_YieldItem()) {
            if (item->AS()) CYPHER_TODO();
            yield_items.emplace_back(item->oC_Variable()->getText());
        }

        std::vector<std::pair<std::string, lgraph_api::LGraphType>> _yield_items;
        CYPHER_THROW_ASSERT(!_curr_procedure_name.empty());
        auto pp = cypher::global_ptable.GetProcedure(_curr_procedure_name);
        if (pp) {
            auto concat_str = [](const cypher::Procedure *pp) {
                if (pp->signature.result_list.empty()) return std::string("[]");
                std::string args = "[";
                for (auto &arg : pp->signature.result_list) {
                    args += arg.name;
                    args += ", ";
                }
                return args.substr(0, args.size() - 2) + "]";
            };
            for (auto &yield_item : yield_items) {
                if (!pp->ContainsYieldItem(yield_item)) {
                    THROW_CODE(InputError,
                        "yield item [{}] is not exsit, should be one of {}", yield_item,
                                concat_str(pp));
                }
                auto type = lgraph_api::LGraphType::NUL;
                for (auto &r : pp->signature.result_list) {
                    if (r.name == yield_item) {
                        type = r.type;
                        _yield_items.emplace_back(yield_item, r.type);
                    }
                }
                switch (type) {
                case lgraph_api::LGraphType::NODE:
                    AddSymbol(yield_item, cypher::SymbolNode::NODE, cypher::SymbolNode::LOCAL);
                    break;
                default:
                    AddSymbol(yield_item, cypher::SymbolNode::CONSTANT, cypher::SymbolNode::LOCAL);
                    break;
                }
            }
        } else {
            // plugin
            auto names = fma_common::Split(_curr_procedure_name, ".");
            if (names.size() > 2 && names[0] == "plugin") {
                std::string input, output;
                auto type = names[1] == "cpp" ? lgraph::PluginManager::PluginType::CPP
                                              : lgraph::PluginManager::PluginType::PYTHON;
                if (type == lgraph::PluginManager::PluginType::PYTHON) {
                    throw lgraph::EvaluationException(
                        "Calling python plugin in CYPHER is disabled in this release.");
                }
                auto db = ctx_->galaxy_->OpenGraph(ctx_->user_, ctx_->graph_);
                auto pm = db.GetLightningGraph()->GetPluginManager();
                lgraph_api::SigSpec *sig_spec = nullptr;
                if (!pm->GetPluginSignature(type, ctx_->user_, names[2], &sig_spec) ||
                    sig_spec == nullptr) {
                    THROW_CODE(InputError, "cannot find procedure name {}", names[2]);
                }
                for (auto &yield_item : yield_items) {
                    const auto iter = std::find_if(
                        sig_spec->result_list.cbegin(), sig_spec->result_list.cend(),
                        [&yield_item](const auto &param) { return yield_item == param.name; });
                    if (iter == sig_spec->result_list.cend()) {
                        THROW_CODE(InputError,
                            "yield item [{}] is not exist", yield_item);
                    }
                    auto type = lgraph_api::LGraphType::NUL;
                    for (auto &r : sig_spec->result_list) {
                        if (r.name == yield_item) {
                            type = r.type;
                            _yield_items.emplace_back(yield_item, r.type);
                        }
                    }
                    switch (type) {
                    case lgraph_api::LGraphType::NODE:
                        AddSymbol(yield_item, cypher::SymbolNode::NODE, cypher::SymbolNode::LOCAL);
                        break;
                    default:
                        AddSymbol(yield_item, cypher::SymbolNode::CONSTANT,
                                  cypher::SymbolNode::LOCAL);
                        break;
                    }
                }
            } else {
                CYPHER_TODO();
            }
        }
        Expression where;
        if (ctx->oC_Where()) {
            where = std::any_cast<Expression>(visit(ctx->oC_Where()));
        }
        return std::make_tuple(_yield_items, where);
    }

    std::any visitOC_YieldItem(LcypherParser::OC_YieldItemContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_With(LcypherParser::OC_WithContext *ctx) override {
        // set add_carry flag
        _EnterClauseWITH();
        TUP_RETURN_BODY return_body = std::any_cast<TUP_RETURN_BODY>(visit(ctx->oC_ReturnBody()));
        bool distinct = ctx->DISTINCT() != nullptr;
        Expression where;
        if (ctx->oC_Where()) where = std::any_cast<Expression>(visit(ctx->oC_Where()));
        Clause clause;
        clause.type = Clause::WITH;
        clause.data = std::make_shared<Clause::TYPE_WITH>(
            std::make_tuple(distinct, std::move(return_body), std::move(where)));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        /* now we finish one query part, start the next part */
        _curr_part++;
        // take carry
        _symbol_idx = _carry.size();
        auto &aim = _query[_curr_query].parts[_curr_part].symbol_table;
        for (int i = 0; i < (int)_carry.size(); i++) {
            aim.symbols.emplace(_carry[i].first, cypher::SymbolNode(i, _carry[i].second,
                                                                    cypher::SymbolNode::ARGUMENT));
        }
        _carry.clear();
        _LeaveClauseWITH();
        return 0;
    }

    std::any visitOC_Return(LcypherParser::OC_ReturnContext *ctx) override {
        _EnterClauseRETURN();
        TUP_RETURN_BODY return_body = std::any_cast<TUP_RETURN_BODY>(visit(ctx->oC_ReturnBody()));
        bool distinct = (ctx->DISTINCT() != nullptr);
        Clause clause;
        clause.type = Clause::RETURN;
        clause.data = std::make_shared<Clause::TYPE_RETURN>(
            std::make_tuple(distinct, std::move(return_body)));
        _query[_curr_query].parts[_curr_part].AddClause(clause);
        _LeaveClauseRETURN();
        return 0;
    }

    std::any visitOC_ReturnBody(LcypherParser::OC_ReturnBodyContext *ctx) override {
        std::vector<TUP_RETURN_ITEM> return_items =
            std::any_cast<std::vector<TUP_RETURN_ITEM>>(visit(ctx->oC_ReturnItems()));
        std::vector<std::pair<int, bool>> sort_items_idx;
        if (ctx->oC_Order()) {
            std::vector<std::pair<Expression, bool>> sort_items =
                std::any_cast<std::vector<std::pair<Expression, bool>>>(visit(ctx->oC_Order()));
            for (auto &sort_item : sort_items) {
                bool sort_idx_found = false;
                for (int i = 0; i < (int)return_items.size(); i++) {
                    auto &expr = std::get<0>(return_items[i]);
                    auto &alias = std::get<1>(return_items[i]);
                    CYPHER_THROW_ASSERT(sort_item.first.type == Expression::VARIABLE ||
                                        sort_item.first.type == Expression::PROPERTY);
                    if (sort_item.first == expr || sort_item.first.ToString() == alias) {
                        sort_items_idx.emplace_back(i, sort_item.second);
                        sort_idx_found = true;
                        break;
                    }
                }
                // sort_item alias is not in return_item
                if (!sort_idx_found) {
                    THROW_CODE(InputError,
                        "Variable `{}` not defined", sort_item.first.ToString());
                }
            }
            CYPHER_THROW_ASSERT(sort_items_idx.size() == sort_items.size());
        }
        Expression skip, limit;
        if (ctx->oC_Skip()) {
            auto tmp = std::any_cast<Expression>(visit(ctx->oC_Skip()));
            skip = tmp;
        }
        if (ctx->oC_Limit()) {
            auto tmp = std::any_cast<Expression>(visit(ctx->oC_Limit()));
            limit = tmp;
        }
        return std::make_tuple(return_items, sort_items_idx, skip, limit);
    }

    std::any visitOC_ReturnItems(LcypherParser::OC_ReturnItemsContext *ctx) override {
        // `WITH *` is prevented, since it will make ambiguity and some bugs
        //
        // ambiguity example: WITH * RETURN *
        //
        // bug example: WITH * MERGE (n:Person) RETURN n
        // this example will build a plan like: PROJECT(root: RETURN) --> OpMerge --> PROJECT(leaf:
        // WITH) `OpMerge` plan node will take record from PROJECT(leaf: WITH), but without no
        // values since `WITH *` specifies no any return items `OpMerge` with parent node
        // PROJECT(root: RETURN) will index its record->values to modifies value type and data, see
        // more: OpMerge::Initialize
        CYPHER_THROW_ASSERT(!ctx->children.empty());
        if (_InClauseWITH() && (ctx->children[0]->getText() == "*")) {
            throw lgraph::CypherException("WITH * syntax is not implemented now");
        }

        std::vector<TUP_RETURN_ITEM> return_items;
        for (auto &item : ctx->oC_ReturnItem()) {
            TUP_RETURN_ITEM return_item = std::any_cast<TUP_RETURN_ITEM>(visit(item));
            return_items.emplace_back(return_item);
        }
        return return_items;
    }

    std::any visitOC_ReturnItem(LcypherParser::OC_ReturnItemContext *ctx) override {
        Expression expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        std::string as_variable, variable;
        if (expr.type == Expression::VARIABLE) variable = expr.String();
        if (ctx->oC_Variable()) as_variable = ctx->oC_Variable()->getText();
        if (_InClauseWITH()) {
            if (as_variable.empty() && variable.empty()) {
                throw lgraph::ParserException("Non-variable expression in WITH must be aliased");
            }
            auto it = _query[_curr_query].parts[_curr_part].symbol_table.symbols.find(variable);
            auto type = it == _query[_curr_query].parts[_curr_part].symbol_table.symbols.end()
                            ? cypher::SymbolNode::CONSTANT
                            : it->second.type;
            AddSymbol(as_variable.empty() ? variable : as_variable, type,
                      cypher::SymbolNode::LOCAL);
        }
        return std::make_tuple(expr, as_variable);
    }

    std::any visitOC_Order(LcypherParser::OC_OrderContext *ctx) override {
        _EnterClauseORDERBY();
        std::vector<std::pair<Expression, bool>> sort_items;
        for (auto item : ctx->oC_SortItem()) {
            std::pair<Expression, bool> sort_item =
                std::any_cast<std::pair<Expression, bool>>(visit(item));
            sort_items.emplace_back(sort_item);
        }
        _LeaveClauseORDERBY();
        return sort_items;
    }

    std::any visitOC_Skip(LcypherParser::OC_SkipContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_Limit(LcypherParser::OC_LimitContext *ctx) override {
        return visit(ctx->oC_Expression());
    }

    std::any visitOC_SortItem(LcypherParser::OC_SortItemContext *ctx) override {
        Expression expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        bool ascending = ctx->ASCENDING() != nullptr || ctx->ASC() != nullptr ||
                         (ctx->DESCENDING() == nullptr && ctx->DESC() == nullptr);
        return std::make_pair(expr, ascending);
    }

    std::any visitOC_Hint(LcypherParser::OC_HintContext *ctx) override {
        const auto &var = ctx->oC_Variable()->getText();
        if (!_IsVariableDefined(var)) {
            THROW_CODE(InputError, "Variable `{}` not defined", var);
        }

        if (ctx->JOIN() && ctx->ON()) {
            return ctx->oC_Variable()->getText().append("@J");
        } else if (ctx->START() && ctx->ON()) {
            return ctx->oC_Variable()->getText().append("@S");
        } else {
            CYPHER_TODO();
        }
    }

    std::any visitOC_Where(LcypherParser::OC_WhereContext *ctx) override {
        _EnterClauseWHERE();
        Expression expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        if (expr.type == Expression::LABEL) expr.Label2Filter();
        CYPHER_THROW_ASSERT(expr.type == Expression::FILTER);
        _LeaveClauseWHERE();
        return expr;
    }

    std::any visitOC_Pattern(LcypherParser::OC_PatternContext *ctx) override {
        std::vector<TUP_PATTERN_PART> pattern;
        for (auto &ctx_part : ctx->oC_PatternPart()) {
            TUP_PATTERN_PART pattern_part = std::any_cast<TUP_PATTERN_PART>(visit(ctx_part));
            pattern.emplace_back(pattern_part);
        }
        return pattern;
    }

    std::any visitOC_PatternPart(LcypherParser::OC_PatternPartContext *ctx) override {
        std::string variable;
        if (ctx->oC_Variable() != nullptr) {
            // named path
            variable = ctx->oC_Variable()->getText();
            AddSymbol(variable, cypher::SymbolNode::NAMED_PATH, cypher::SymbolNode::LOCAL);
        }
        TUP_PATTERN_ELEMENT anonymous_pattern_part =
            std::any_cast<TUP_PATTERN_ELEMENT>(visit(ctx->oC_AnonymousPatternPart()));
        return std::make_tuple(variable, anonymous_pattern_part);
    }

    std::any visitOC_AnonymousPatternPart(
        LcypherParser::OC_AnonymousPatternPartContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_PatternElement(LcypherParser::OC_PatternElementContext *ctx) override {
        if (ctx->oC_NodePattern() == nullptr) {
            return visitChildren(ctx);
        }
        TUP_NODE_PATTERN node_pattern =
            std::any_cast<TUP_NODE_PATTERN>(visit(ctx->oC_NodePattern()));
        std::vector<TUP_PATTERN_ELEMENT_CHAIN> pattern_element_chains;
        auto pecs = ctx->oC_PatternElementChain();
        if (pecs.empty()) {
        } else {
            for (auto &p : pecs) {
                TUP_PATTERN_ELEMENT_CHAIN pattern_element_chain =
                    std::any_cast<TUP_PATTERN_ELEMENT_CHAIN>(visit(p));
                pattern_element_chains.push_back(pattern_element_chain);
            }
        }
        return std::make_tuple(node_pattern, pattern_element_chains);
    }

    std::any visitOC_NodePattern(LcypherParser::OC_NodePatternContext *ctx) override {
        std::string variable;
        VEC_STR node_labels;
        TUP_PROPERTIES properties;
        /* NOTE: 'variable = std::any_cast<>(visit(ctx->oC_Variable()));' goes wrong!
         * For it uses 'operator =' instead of copy constructor.  */
        if (ctx->oC_Variable() != nullptr) {
            std::string tmp = std::any_cast<std::string>(visit(ctx->oC_Variable()));
            variable = tmp;
        } else {
            // if alias is absent, generate an alias for the node
            variable = GenAnonymousAlias(true);
        }
        if (ctx->oC_NodeLabels() != nullptr) {
            VEC_STR tmp = std::any_cast<VEC_STR>(visit(ctx->oC_NodeLabels()));
            node_labels = std::move(tmp);
        }
        if (ctx->oC_Properties() != nullptr) {
            TUP_PROPERTIES tmp = std::any_cast<TUP_PROPERTIES>(visit(ctx->oC_Properties()));
            properties = std::move(tmp);
            for (const auto& pair : std::get<0>(properties).Map()) {
                for (auto& label : node_labels) {
                    if (!label.empty()) {
                        _node_property[label].emplace(pair.first);
                    }
                }
            }
        }
        AddSymbol(variable, cypher::SymbolNode::NODE, cypher::SymbolNode::LOCAL);
        return std::make_tuple(variable, node_labels, properties);
    }

    std::any visitOC_PatternElementChain(
        LcypherParser::OC_PatternElementChainContext *ctx) override {
        TUP_RELATIONSHIP_PATTERN relationship_pattern =
            std::any_cast<TUP_RELATIONSHIP_PATTERN>(visit(ctx->oC_RelationshipPattern()));
        TUP_NODE_PATTERN node_pattern =
            std::any_cast<TUP_NODE_PATTERN>(visit(ctx->oC_NodePattern()));
        return std::make_tuple(relationship_pattern, node_pattern);
    }

    std::any visitOC_RelationshipPattern(
        LcypherParser::OC_RelationshipPatternContext *ctx) override {
        LinkDirection direction;
        if (ctx->oC_LeftArrowHead() != nullptr && ctx->oC_RightArrowHead() == nullptr) {
            direction = LinkDirection::RIGHT_TO_LEFT;
        } else if (ctx->oC_RightArrowHead() != nullptr && ctx->oC_LeftArrowHead() == nullptr) {
            direction = LinkDirection::LEFT_TO_RIGHT;
        } else {
            direction = LinkDirection::DIR_NOT_SPECIFIED;
        }
        TUP_RELATIONSHIP_DETAIL relationship_detail;
        if (ctx->oC_RelationshipDetail() != nullptr) {
            relationship_detail =
                std::any_cast<TUP_RELATIONSHIP_DETAIL>(visit(ctx->oC_RelationshipDetail()));
        } else {
            // set anonymous relationship's alias, range_literal
            auto alias = GenAnonymousAlias(false);
            std::get<0>(relationship_detail) = alias;
            std::get<2>(relationship_detail) = {-1, -1};
            AddSymbol(alias, cypher::SymbolNode::RELATIONSHIP, cypher::SymbolNode::LOCAL);
        }
        return std::make_tuple(direction, relationship_detail);
    }

    std::any visitOC_RelationshipDetail(
        LcypherParser::OC_RelationshipDetailContext *ctx) override {
        std::string variable;
        VEC_STR relationship_types;
        std::array<int, 2> range_literal{-1, -1};
        TUP_PROPERTIES properties;
        if (ctx->oC_Variable() != nullptr) {
            std::string tmp = std::any_cast<std::string>(visit(ctx->oC_Variable()));
            variable = tmp;
        } else {
            // if alias is absent, generate an alias for the relationship
            variable = GenAnonymousAlias(false);
        }
        if (ctx->oC_RelationshipTypes() != nullptr) {
            VEC_STR tmp = std::any_cast<VEC_STR>(visit(ctx->oC_RelationshipTypes()));
            relationship_types = tmp;
        }
        if (ctx->oC_RangeLiteral() != nullptr) {
            std::array<int, 2> tmp =
                std::any_cast<std::array<int, 2>>(visit(ctx->oC_RangeLiteral()));
            range_literal = tmp;
        }
        if (ctx->oC_Properties() != nullptr) {
            TUP_PROPERTIES tmp = std::any_cast<TUP_PROPERTIES>(visit(ctx->oC_Properties()));
            properties = tmp;
            for (const auto& pair : std::get<0>(properties).Map()) {
                for (auto &rel_label : relationship_types) {
                    if (!rel_label.empty()) {
                        _rel_property[rel_label].emplace(pair.first);
                    }
                }
            }
        }
        AddSymbol(variable, cypher::SymbolNode::RELATIONSHIP, cypher::SymbolNode::LOCAL);
        return std::make_tuple(variable, relationship_types, range_literal, properties);
    }

    std::any visitOC_Properties(LcypherParser::OC_PropertiesContext *ctx) override {
        std::string parameter;
        if (ctx->oC_MapLiteral()) {
            Expression map_literal = std::any_cast<Expression>(visit(ctx->oC_MapLiteral()));
            return std::make_tuple(map_literal, parameter);
        } else if (ctx->oC_Parameter()) {
            std::string parameter_val = ctx->oC_Parameter()->getText();
            if (ctx_->bolt_parameters_) {
                auto iter = ctx_->bolt_parameters_->find(parameter_val);
                if (iter == ctx_->bolt_parameters_->end()) {
                    throw lgraph::CypherException(
                        FMA_FMT("Parameter {} missing value", parameter_val));
                }
                if (iter->second.type != Expression::DataType::MAP) {
                    throw lgraph::CypherException(
                        FMA_FMT("Parameter {} should be MAP type", parameter_val));
                }
                return std::make_tuple(iter->second, parameter);
            }
        }
        CYPHER_TODO();
    }

    std::any visitOC_RelationshipTypes(
        LcypherParser::OC_RelationshipTypesContext *ctx) override {
        VEC_STR relationship_types;
        for (auto &ctx_name : ctx->oC_RelTypeName()) {
            std::string name = std::any_cast<std::string>(visit(ctx_name));
            relationship_types.emplace_back(name);
            _rel_property.emplace(name, std::set<std::string>{});
        }
        return relationship_types;
    }

    std::any visitOC_NodeLabels(LcypherParser::OC_NodeLabelsContext *ctx) override {
        if (ctx->oC_NodeLabel().size() > 1) {
            LOG_WARN()
                << "More than one labels are provided, the first is picked [" << ctx->getText()
                << "]";
        }
        VEC_STR labels;
        for (auto &ctx_label : ctx->oC_NodeLabel()) {
            std::string label = std::any_cast<std::string>(visit(ctx_label));
            labels.emplace_back(label);
            _node_property.emplace(label, std::set<std::string>{});
        }
        return labels;
    }

    std::any visitOC_NodeLabel(LcypherParser::OC_NodeLabelContext *ctx) override {
        std::string label = std::any_cast<std::string>(visit(ctx->oC_LabelName()));
        return label;
    }

    std::any visitOC_RangeLiteral(LcypherParser::OC_RangeLiteralContext *ctx) override {
        /* cases:
         * *2         [2, 2]
         * *2..5      [2, 5]
         * *2..       [2, MAX]
         * *..5       [1, 5]
         * *..        [1, MAX]
         */
        std::array<int, 2> range_literal{-1, -1};
        // ctx->children may contain SP, minus SP first
        std::vector<antlr4::tree::ParseTree *> valid_children;
        for (const auto &it : ctx->children) {
            const auto &text = it->getText();
            if (!std::all_of(text.cbegin(), text.cend(), ::isspace)) {
                valid_children.emplace_back(it);
            }
        }
        switch (valid_children.size()) {
        case 2:
            {
                if (!ctx->oC_IntegerLiteral().empty()) {
                    range_literal[0] = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
                    range_literal[1] = range_literal[0];
                } else {
                    CYPHER_THROW_ASSERT(valid_children.at(1)->getText() == "..");
                    range_literal[0] = 1;
                    range_literal[1] = VAR_LEN_EXPAND_MAX_HOP;
                }
                break;
            }
        case 3:
            {
                if (valid_children.at(2)->getText() == "..") {
                    range_literal[0] = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
                    range_literal[1] = VAR_LEN_EXPAND_MAX_HOP;
                } else {
                    CYPHER_THROW_ASSERT(valid_children.at(1)->getText() == "..");
                    range_literal[0] = 1;
                    range_literal[1] = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
                }
                break;
            }
        case 4:
            {
                CYPHER_THROW_ASSERT(ctx->oC_IntegerLiteral().size() == 2);
                range_literal[0] = std::stoi(ctx->oC_IntegerLiteral(0)->getText());
                range_literal[1] = std::stoi(ctx->oC_IntegerLiteral(1)->getText());
                break;
            }
        default:
            CYPHER_TODO();
        }
        return range_literal;
    }

    std::any visitOC_LabelName(LcypherParser::OC_LabelNameContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_RelTypeName(LcypherParser::OC_RelTypeNameContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_Expression(LcypherParser::OC_ExpressionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_OrExpression(LcypherParser::OC_OrExpressionContext *ctx) override {
        if (ctx->OR().empty()) return visitChildren(ctx);
        Expression expr0 = std::any_cast<Expression>(visit(ctx->oC_XorExpression(0)));
        if (expr0.type == Expression::LABEL) expr0.Label2Filter();
        if (expr0.type != Expression::FILTER) CYPHER_TODO();
        auto filter = expr0.Filter();
        for (size_t i = 0; i < ctx->OR().size(); i++) {
            Expression e = std::any_cast<Expression>(visit(ctx->oC_XorExpression(i + 1)));
            if (e.type == Expression::LABEL) e.Label2Filter();
            CYPHER_THROW_ASSERT(e.type == Expression::FILTER);
            filter =
                std::make_shared<lgraph::Filter>(lgraph::LogicalOp::LBR_OR, filter, e.Filter());
        }
        Expression expr;
        expr.type = Expression::FILTER;
        expr.data = filter;
        return expr;
    }

    std::any visitOC_XorExpression(LcypherParser::OC_XorExpressionContext *ctx) override {
        if (ctx->XOR().empty()) return visitChildren(ctx);
        Expression expr0 = std::any_cast<Expression>(visit(ctx->oC_AndExpression(0)));
        if (expr0.type == Expression::LABEL) expr0.Label2Filter();
        if (expr0.type != Expression::FILTER) CYPHER_TODO();
        auto filter = expr0.Filter();
        for (size_t i = 0; i < ctx->XOR().size(); i++) {
            Expression e = std::any_cast<Expression>(visit(ctx->oC_AndExpression(i + 1)));
            if (e.type == Expression::LABEL) e.Label2Filter();
            CYPHER_THROW_ASSERT(e.type == Expression::FILTER);
            filter =
                std::make_shared<lgraph::Filter>(lgraph::LogicalOp::LBR_XOR, filter, e.Filter());
        }
        Expression expr;
        expr.type = Expression::FILTER;
        expr.data = filter;
        return expr;
    }

    std::any visitOC_AndExpression(LcypherParser::OC_AndExpressionContext *ctx) override {
        if (!ctx->AND().empty()) {
            Expression expr0 = std::any_cast<Expression>(visit(ctx->oC_NotExpression(0)));
            if (expr0.type == Expression::LABEL) expr0.Label2Filter();
            if (expr0.type != Expression::FILTER) CYPHER_TODO();
            auto filter = expr0.Filter();
            for (int i = 0; i < (int)ctx->AND().size(); i++) {
                Expression e = std::any_cast<Expression>(visit(ctx->oC_NotExpression(i + 1)));
                if (e.type == Expression::LABEL) e.Label2Filter();
                CYPHER_THROW_ASSERT(e.type == Expression::FILTER);
                filter = std::make_shared<lgraph::Filter>(lgraph::LogicalOp::LBR_AND, filter,
                                                          e.Filter());
            }
            Expression expr;
            expr.type = Expression::FILTER;
            expr.data = filter;
            return expr;
        }
        return visitChildren(ctx);
    }

    std::any visitOC_NotExpression(LcypherParser::OC_NotExpressionContext *ctx) override {
        if ((ctx->NOT().size() & 0x1) == 0) {
            return visitChildren(ctx);
        } else {
            Expression e = std::any_cast<Expression>(visit(ctx->oC_ComparisonExpression()));
            if (e.type != Expression::FILTER) throw lgraph::ParserException("Type error");
            auto filter = std::make_shared<lgraph::Filter>(lgraph::LogicalOp::LBR_NOT, e.Filter());
            Expression expr;
            expr.type = Expression::FILTER;
            expr.data = filter;
            return expr;
        }
    }

    std::any visitOC_ComparisonExpression(
        LcypherParser::OC_ComparisonExpressionContext *ctx) override {
        if (ctx->oC_PartialComparisonExpression().size() > 1) CYPHER_TODO();

        if (ctx->oC_PartialComparisonExpression().size() == 1) {
            Expression expr_a = std::any_cast<Expression>(visit(ctx->oC_AddOrSubtractExpression()));
            std::shared_ptr<lgraph::Filter> f;
            /* e.g.
             * n.age > 20,
             * id(n) = 2,
             * m <> n
             * count(*) AS f WHERE f > 1  */
            if (expr_a.type == Expression::PROPERTY || expr_a.type == Expression::FUNCTION ||
                expr_a.type == Expression::VARIABLE || expr_a.type == Expression::MATH) {
                std::pair<lgraph::CompareOp, Expression> partial =
                    std::any_cast<std::pair<lgraph::CompareOp, Expression>>(
                        visit(ctx->oC_PartialComparisonExpression(0)));
                /* Assert: the left & right alias are contained in alias_id_map */
                cypher::ArithExprNode ae_left(expr_a,
                                              _query[_curr_query].parts[_curr_part].symbol_table);
                cypher::ArithExprNode ae_right(partial.second,
                                               _query[_curr_query].parts[_curr_part].symbol_table);
                f = std::make_shared<lgraph::RangeFilter>(partial.first, ae_left, ae_right);
            } else {
                CYPHER_PARSER_CHECK(false, ctx->getText());
            }
            Expression expr;
            expr.type = Expression::FILTER;
            expr.data = f;
            return expr;
        }
        return visitChildren(ctx);
    }

    std::any visitOC_AddOrSubtractExpression(
        LcypherParser::OC_AddOrSubtractExpressionContext *ctx) override {
        if (ctx->oC_MultiplyDivideModuloExpression().size() == 1) {
            return visit(ctx->oC_MultiplyDivideModuloExpression(0));
        }
        // covert to rpn
        std::vector<Expression> operators;
        for (auto c : ctx->children) {
            if (c->getText() == "+" || c->getText() == "-") {
                Expression op;
                op.type = Expression::STRING;
                op.data = std::make_shared<std::string>(c->getText());
                operators.emplace_back(op);
            }
        }
        auto rpn = std::make_shared<Expression::EXPR_TYPE_LIST>();
        int i = 0;
        for (auto e : ctx->oC_MultiplyDivideModuloExpression()) {
            Expression expr = std::any_cast<Expression>(visit(e));
            if (expr.type == Expression::MATH) {
                auto &l = expr.List();
                rpn->insert(rpn->end(), std::make_move_iterator(l.begin()),
                            std::make_move_iterator(l.end()));
            } else {
                rpn->emplace_back(expr);
            }
            if (i++ > 0) rpn->emplace_back(operators[i - 2]);
        }
        Expression ret;
        ret.type = Expression::MATH;
        ret.data = rpn;
        return ret;
    }

    std::any visitOC_MultiplyDivideModuloExpression(
        LcypherParser::OC_MultiplyDivideModuloExpressionContext *ctx) override {
        if (ctx->oC_PowerOfExpression().size() == 1) {
            return visit(ctx->oC_PowerOfExpression(0));
        }
        // covert to rpn
        std::vector<Expression> operators;
        for (auto c : ctx->children) {
            auto text = c->getText();
            if (text == "*" || text == "/" || text == "%") {
                Expression op;
                op.type = Expression::STRING;
                op.data = std::make_shared<std::string>(std::move(text));
                operators.emplace_back(op);
            }
        }
        auto rpn = std::make_shared<Expression::EXPR_TYPE_LIST>();
        int i = 0;
        for (auto e : ctx->oC_PowerOfExpression()) {
            Expression expr = std::any_cast<Expression>(visit(e));
            if (expr.type == Expression::MATH) {
                auto &l = expr.List();
                rpn->insert(rpn->end(), std::make_move_iterator(l.begin()),
                            std::make_move_iterator(l.end()));
            } else {
                rpn->emplace_back(expr);
            }
            if (i++ > 0) rpn->emplace_back(operators[i - 2]);
        }
        Expression ret;
        ret.type = Expression::MATH;
        ret.data = rpn;
        return ret;
    }

    std::any visitOC_PowerOfExpression(
        LcypherParser::OC_PowerOfExpressionContext *ctx) override {
        if (ctx->oC_UnaryAddOrSubtractExpression().size() == 1) {
            return visit(ctx->oC_UnaryAddOrSubtractExpression(0));
        }
        // covert to rpn
        int op_count = 0;
        for (auto c : ctx->children) {
            if (c->getText() == "^") op_count++;
        }
        Expression power_op;
        power_op.type = Expression::STRING;
        power_op.data = std::make_shared<std::string>("^");
        auto rpn = std::make_shared<Expression::EXPR_TYPE_LIST>();
        int i = 0;
        for (auto e : ctx->oC_UnaryAddOrSubtractExpression()) {
            Expression expr = std::any_cast<Expression>(visit(e));
            if (expr.type == Expression::MATH) {
                auto &l = expr.List();
                rpn->insert(rpn->end(), std::make_move_iterator(l.begin()),
                            std::make_move_iterator(l.end()));
            } else {
                rpn->emplace_back(expr);
            }
            if (i++ > 0) rpn->emplace_back(power_op);
        }
        Expression ret;
        ret.type = Expression::MATH;
        ret.data = rpn;
        return ret;
    }

    std::any visitOC_UnaryAddOrSubtractExpression(
        LcypherParser::OC_UnaryAddOrSubtractExpressionContext *ctx) override {
        int sign = 0;
        if (ctx->children.size() > 1) {
            for (auto &child : ctx->children) {
                // TODO(anyone) not elegant
                if (child->getText() == "-") sign ^= 0x1;
            }
        }
        Expression e = std::any_cast<Expression>(visit(ctx->oC_StringListNullOperatorExpression()));
        if (sign) {
            switch (e.type) {
            case Expression::INT:
                e.data = -e.Int();
                return e;
            case Expression::DOUBLE:
                e.data = -e.Double();
                return e;
            default:
                {
                    // covert unary expression `-{E}` to binary expression `0-{E}`
                    auto rpn = std::make_shared<Expression::EXPR_TYPE_LIST>();
                    Expression zero;
                    zero.type = Expression::INT;
                    zero.data = static_cast<int64_t>(0);
                    Expression minus;
                    minus.type = Expression::STRING;
                    minus.data = std::make_shared<std::string>("-");
                    rpn->emplace_back(zero);
                    rpn->emplace_back(e);
                    rpn->emplace_back(minus);
                    Expression ret;
                    ret.type = Expression::MATH;
                    ret.data = rpn;
                    return ret;
                }
            }
        }
        return e;
    }

    std::any visitOC_StringListNullOperatorExpression(
        LcypherParser::OC_StringListNullOperatorExpressionContext *ctx) override {
        Expression left = std::any_cast<Expression>(visit(ctx->oC_PropertyOrLabelsExpression()));
        Expression ret;
        if (!ctx->oC_StringOperatorExpression().empty()) {
            if (ctx->oC_StringOperatorExpression().size() > 1) CYPHER_TODO();
            std::pair<lgraph::StringFilter::Comparison, Expression> right =
                std::any_cast<std::pair<lgraph::StringFilter::Comparison, Expression>>(
                    visit(ctx->oC_StringOperatorExpression(0)));
            cypher::ArithExprNode ae_left(left, _query[_curr_query].parts[_curr_part].symbol_table);
            cypher::ArithExprNode ae_right(right.second,
                                           _query[_curr_query].parts[_curr_part].symbol_table);
            std::shared_ptr<lgraph::Filter> f =
                std::make_shared<lgraph::StringFilter>(right.first, ae_left, ae_right);
            ret.type = Expression::FILTER;
            ret.data = f;
            return ret;
        } else if (!ctx->oC_NullOperatorExpression().empty()) {
            if (ctx->oC_NullOperatorExpression().size() > 1) CYPHER_TODO();
            auto null_op_expr = ctx->oC_NullOperatorExpression(0);
            CYPHER_THROW_ASSERT(null_op_expr->IS() && null_op_expr->NULL_());
            cypher::ArithExprNode ae_left(left, _query[_curr_query].parts[_curr_part].symbol_table);
            bool test_is_null = !null_op_expr->NOT();
            std::shared_ptr<lgraph::Filter> f =
                std::make_shared<lgraph::TestNullFilter>(test_is_null, ae_left);
            ret.type = Expression::FILTER;
            ret.data = f;
            return ret;
        } else if (!ctx->oC_ListOperatorExpression().empty()) {
            if (ctx->oC_ListOperatorExpression().size() > 1) CYPHER_TODO();
            auto list_op_expr = ctx->oC_ListOperatorExpression(0);
            if (list_op_expr->IN()) {
                CYPHER_THROW_ASSERT(list_op_expr->oC_PropertyOrLabelsExpression());
                cypher::ArithExprNode ae_left(left,
                                              _query[_curr_query].parts[_curr_part].symbol_table);
                Expression right =
                    std::any_cast<Expression>(visit(list_op_expr->oC_PropertyOrLabelsExpression()));
                cypher::ArithExprNode ae_right(right,
                                               _query[_curr_query].parts[_curr_part].symbol_table);
                std::shared_ptr<lgraph::Filter> f =
                    std::make_shared<lgraph::TestInFilter>(ae_left, ae_right);
                ret.type = Expression::FILTER;
                ret.data = f;
                return ret;
            } else {
                CYPHER_THROW_ASSERT(!list_op_expr->oC_Expression().empty());
                auto list = std::make_shared<Expression::EXPR_TYPE_LIST>();
                Expression func_name, distinct;
                func_name.type = Expression::STRING;
                func_name.data = std::make_shared<std::string>("subscript");
                distinct.type = Expression::BOOL;
                distinct.data = false;
                list->emplace_back(std::move(func_name));
                list->emplace_back(std::move(distinct));
                list->emplace_back(left);
                Expression start_idx =
                    std::any_cast<Expression>(visit(list_op_expr->oC_Expression(0)));
                list->emplace_back(std::move(start_idx));
                if (list_op_expr->oC_Expression().size() == 2) {
                    Expression end_idx =
                        std::any_cast<Expression>(visit(list_op_expr->oC_Expression(1)));
                    list->emplace_back(std::move(end_idx));
                }
                ret.type = Expression::FUNCTION;
                ret.data = list;
                return ret;
            }
        }
        return left;
    }

    std::any visitOC_ListOperatorExpression(
        LcypherParser::OC_ListOperatorExpressionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_StringOperatorExpression(
        LcypherParser::OC_StringOperatorExpressionContext *ctx) override {
        auto comp_op = ctx->STARTS()     ? lgraph::StringFilter::STARTS_WITH
                       : ctx->ENDS()     ? lgraph::StringFilter::ENDS_WITH
                       : ctx->CONTAINS() ? lgraph::StringFilter::CONTAINS
                                         : lgraph::StringFilter::REGEXP;
        Expression rhs = std::any_cast<Expression>(visit(ctx->oC_PropertyOrLabelsExpression()));
        return std::make_pair(comp_op, rhs);
    }

    std::any visitOC_NullOperatorExpression(
        LcypherParser::OC_NullOperatorExpressionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_PropertyOrLabelsExpression(
        LcypherParser::OC_PropertyOrLabelsExpressionContext *ctx) override {
        if (ctx->children.size() == 1) return visitChildren(ctx);
        Expression expr;
        if (!ctx->oC_PropertyLookup().empty()) {
            // e.g. tom.name
            if (ctx->oC_PropertyLookup().size() > 1 || ctx->oC_NodeLabels() != nullptr)
                CYPHER_TODO();
            Expression atom = std::any_cast<Expression>(visit(ctx->oC_Atom()));
            std::string key_name = std::any_cast<std::string>(visit(ctx->oC_PropertyLookup(0)));
            CYPHER_PARSER_CHECK(atom.type == Expression::VARIABLE, ctx->getText());
            expr.type = Expression::PROPERTY;
            expr.data = std::make_shared<Expression::EXPR_TYPE_PROPERTY>(
                Expression::EXPR_TYPE_PROPERTY{atom, key_name});
        } else if (ctx->oC_NodeLabels() != nullptr) {
            // e.g. tom:Person
            Expression atom = std::any_cast<Expression>(visit(ctx->oC_Atom()));
            VEC_STR labels = std::any_cast<VEC_STR>(visit(ctx->oC_NodeLabels()));
            CYPHER_PARSER_CHECK(atom.type == Expression::VARIABLE && !labels.empty(),
                                ctx->getText());
            expr.type = Expression::LABEL;
            expr.data = std::make_shared<Expression::EXPR_TYPE_LABEL>(
                Expression::EXPR_TYPE_LABEL{atom, labels});
        }
        return expr;
    }

    std::any visitOC_Atom(LcypherParser::OC_AtomContext *ctx) override {
        if (ctx->oC_Literal()) {
            return visit(ctx->oC_Literal());
        } else if (ctx->oC_Variable()) {
            const auto &var = ctx->oC_Variable()->getText();
            // WARNING: `_listcompr_placeholder` is a WORKAROUND for list comprehension. When going
            // into List Comprehension ctx, using one additional symbol string
            // `_listcompr_placeholder` to match var e.g. WTIH [x in [1,2,3] | x] AS result RETURN
            // result, here `_listcompr_placeholder` = "x"
            //
            // I try to add symbol in `visitOC_ListComprephension` function, a cypher todo
            // exception is thrown during exection. Fix this problem makes a lot adjustment
            // or refactor work, may break the whole semantic analysis robust.
            //
            // Also, the `_InClauseORDERBY()` condition is a WORKAROUND for that, `AddSymbol` cannot
            // add symbol into symbol_table when in return clause, but orderby sub-clause in return
            // may use variable or alias in return item. e.g. match
            // (v1:Film)<-[:ACTED_IN|DIRECTED]-(v2:Person) return v1.title,count(distinct v2) as cnt
            // order by cnt here `cnt` variable is from return item.
            //
            // variable definition validation for this case is performed in `visitOC_ReturnBody`

            // if in order by clause, DO NOT check
            // if variable is not in symbol table
            // if context is in list comprehension expression, compare var with
            // `_listcompr_placeholder`
            if (!_InClauseORDERBY() && !_IsVariableDefined(var) &&
                (_listcompr_placeholder.empty() || var != _listcompr_placeholder)) {
                THROW_CODE(InputError, "Variable `{}` not defined", var);
            }

            Expression expr;
            expr.type = Expression::VARIABLE;
            expr.data = std::make_shared<std::string>(ctx->oC_Variable()->getText());
            return expr;
        } else if (ctx->oC_Parameter()) {
            std::string parameter = ctx->oC_Parameter()->getText();
            if (ctx_->bolt_parameters_) {
                auto iter = ctx_->bolt_parameters_->find(parameter);
                if (iter == ctx_->bolt_parameters_->end()) {
                    throw lgraph::CypherException(FMA_FMT("Parameter {} missing value", parameter));
                }
                return iter->second;
            }
            AddSymbol(parameter, cypher::SymbolNode::PARAMETER, cypher::SymbolNode::LOCAL);
            Expression expr;
            expr.type = Expression::PARAMETER;
            expr.data = std::make_shared<std::string>(std::move(parameter));  // $param
            return expr;
        } else if (ctx->oC_FunctionInvocation()) {
            return visit(ctx->oC_FunctionInvocation());
        } else if (ctx->oC_ParenthesizedExpression()) {
            return visitChildren(ctx);
        } else if (ctx->COUNT()) {
            /* count(*) */
            auto list = std::make_shared<Expression::EXPR_TYPE_LIST>();
            Expression function_name;
            function_name.type = Expression::STRING;
            function_name.data = std::make_shared<std::string>("count(*)");
            list->emplace_back(function_name);
            Expression ret;
            ret.type = Expression::FUNCTION;
            ret.data = list;
            return ret;
        } else if (ctx->oC_CaseExpression()) {
            return visit(ctx->oC_CaseExpression());
        } else if (ctx->oC_RelationshipsPattern()) {
            Expression expr;
            expr.type = Expression::RELATIONSHIPS_PATTERN;
            Expression::EXPR_RELATIONSHIPS_PATTERN expr_relationships_pattern =
                std::any_cast<Expression::EXPR_RELATIONSHIPS_PATTERN>(
                    visit(ctx->oC_RelationshipsPattern()));
            expr.data = std::make_shared<Expression::EXPR_RELATIONSHIPS_PATTERN>(
                expr_relationships_pattern);
            return expr;
        } else if (ctx->oC_ListComprehension()) {
            return visit(ctx->oC_ListComprehension());
        } else {
            CYPHER_TODO();
            return visitChildren(ctx);
        }
    }

    std::any visitOC_Literal(LcypherParser::OC_LiteralContext *ctx) override {
        Expression expr;
        if (ctx->oC_NumberLiteral() != nullptr) {
            return visitChildren(ctx);
        } else if (ctx->StringLiteral() != nullptr) {
            auto str = ctx->StringLiteral()->getText();
            std::string res;
            // remove escape character
            for (size_t i = 1; i < str.length() - 1; i++) {
                if (str[i] == '\\') {
                    i++;
                }
                res.push_back(str[i]);
            }
            expr.type = Expression::STRING;
            expr.data = std::make_shared<std::string>(std::move(res));
            return expr;
        } else if (ctx->oC_BooleanLiteral() != nullptr) {
            return visitChildren(ctx);
        } else if (ctx->NULL_() != nullptr) {
            expr.type = Expression::NULL_;
            return expr;
        } else {  // NumberLiteral, BooleanLiteral, MapLiteral, ListLiteral
            return visitChildren(ctx);
        }
    }

    std::any visitOC_BooleanLiteral(LcypherParser::OC_BooleanLiteralContext *ctx) override {
        Expression expr;
        expr.type = Expression::BOOL;
        expr.data = ctx->TRUE_() != nullptr;
        return expr;
    }

    std::any visitOC_ListLiteral(LcypherParser::OC_ListLiteralContext *ctx) override {
        Expression::EXPR_TYPE_LIST list_literal;
        Expression expr;
        for (auto &ctx_expr : ctx->oC_Expression()) {
            Expression e = std::any_cast<Expression>(visit(ctx_expr));
            list_literal.emplace_back(e);
        }
        expr.type = Expression::LIST;
        expr.data = std::make_shared<Expression::EXPR_TYPE_LIST>(std::move(list_literal));
        return expr;
    }

    std::any visitOC_PartialComparisonExpression(
        LcypherParser::OC_PartialComparisonExpressionContext *ctx) override {
        Expression e = std::any_cast<Expression>(visit(ctx->oC_AddOrSubtractExpression()));
        auto predicate = ctx->getStart()->getText();
        if (predicate == "=") {
            return std::make_pair(lgraph::CompareOp::LBR_EQ, e);
        } else if (predicate == "<>") {
            return std::make_pair(lgraph::CompareOp::LBR_NEQ, e);
        } else if (predicate == "<") {
            return std::make_pair(lgraph::CompareOp::LBR_LT, e);
        } else if (predicate == ">") {
            return std::make_pair(lgraph::CompareOp::LBR_GT, e);
        } else if (predicate == "<=") {
            return std::make_pair(lgraph::CompareOp::LBR_LE, e);
        } else if (predicate == ">=") {
            return std::make_pair(lgraph::CompareOp::LBR_GE, e);
        } else {
            CYPHER_TODO();
            return visitChildren(ctx);
        }
    }

    std::any visitOC_ParenthesizedExpression(
        LcypherParser::OC_ParenthesizedExpressionContext *ctx) override {
        /* note that visitChildren will return only the last child's result */
        Expression expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        if (expr.type != Expression::FILTER && expr.type != Expression::MATH) {
            /* e.g.
             * cond1 OR (cond2 AND cond3)
             * 1 - (2 + 3)  */
            CYPHER_TODO();
        }
        return expr;
    }

    std::any visitOC_RelationshipsPattern(
        LcypherParser::OC_RelationshipsPatternContext *ctx) override {
        TUP_NODE_PATTERN node_pattern =
            std::any_cast<TUP_NODE_PATTERN>(visit(ctx->oC_NodePattern()));
        std::vector<TUP_PATTERN_ELEMENT_CHAIN> pattern_element_chains;
        for (auto p : ctx->oC_PatternElementChain()) {
            TUP_PATTERN_ELEMENT_CHAIN pattern_element_chain =
                std::any_cast<TUP_PATTERN_ELEMENT_CHAIN>(visit(p));
            pattern_element_chains.push_back(pattern_element_chain);
        }
        return std::make_tuple(node_pattern, pattern_element_chains);
    }

    std::any visitOC_FilterExpression(
        LcypherParser::OC_FilterExpressionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_IdInColl(LcypherParser::OC_IdInCollContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_FunctionInvocation(
        LcypherParser::OC_FunctionInvocationContext *ctx) override {
        Expression ret;
        std::string name = std::any_cast<std::string>(visit(ctx->oC_FunctionName()));
        if (name == "EXISTS") {
            if (ctx->oC_Expression().size() != 1) CYPHER_TODO();
            Expression expr = std::any_cast<Expression>(visit(ctx->oC_Expression(0)));
            if (expr.type == Expression::PROPERTY) {
                cypher::ArithOperandNode property(
                    expr, _query[_curr_query].parts[_curr_part].symbol_table);
                std::shared_ptr<lgraph::Filter> f = std::make_shared<lgraph::TestExists>(property);
                ret.type = Expression::FILTER;
                ret.data = f;
            } else if (expr.type == Expression::RELATIONSHIPS_PATTERN) {
                auto &sym_tab = _query[_curr_query].parts[_curr_part].symbol_table;
                auto &relationships_pattern = expr.RelationshipsPattern();
                auto nested_pattern_graph = std::make_shared<cypher::PatternGraph>();
                auto &node_pattern = std::get<0>(relationships_pattern);
                auto &pattern_element_chains = std::get<1>(relationships_pattern);
                cypher::NodeID curr, prev;
                auto der = sym_tab.symbols.find(std::get<0>(node_pattern)) == sym_tab.symbols.end()
                               ? cypher::Node::CREATED
                               : cypher::Node::ARGUMENT;
                prev = nested_pattern_graph->BuildNode(node_pattern, der);
                for (auto &chain : pattern_element_chains) {
                    auto &relp_patn = std::get<0>(chain);
                    auto &node_patn = std::get<1>(chain);
                    auto node_der =
                        sym_tab.symbols.find(std::get<0>(node_patn)) == sym_tab.symbols.end()
                            ? cypher::Node::CREATED
                            : cypher::Node::ARGUMENT;
                    auto relp_der = sym_tab.symbols.find(std::get<0>(std::get<1>(relp_patn))) ==
                                            sym_tab.symbols.end()
                                        ? cypher::Relationship::CREATED
                                        : cypher::Relationship::ARGUMENT;
                    curr = nested_pattern_graph->BuildNode(node_patn, node_der);
                    nested_pattern_graph->BuildRelationship(relp_patn, prev, curr, relp_der);
                    prev = curr;
                }
                std::shared_ptr<lgraph::Filter> f = std::make_shared<lgraph::TestExists>(
                    _query[_curr_query].parts[_curr_part].symbol_table, nested_pattern_graph);
                ret.type = Expression::FILTER;
                ret.data = f;
            } else {
                CYPHER_TODO();
            }
            return ret;
        }
        auto list = std::make_shared<Expression::EXPR_TYPE_LIST>();
        Expression function_name, distinct;
        function_name.type = Expression::STRING;
        function_name.data = std::make_shared<std::string>(std::move(name));
        list->emplace_back(function_name);
        distinct.type = Expression::BOOL;
        distinct.data = (ctx->DISTINCT() != nullptr);
        list->emplace_back(distinct);
        for (auto &e : ctx->oC_Expression()) {
            Expression expr = std::any_cast<Expression>(visit(e));
            list->emplace_back(expr);
        }
        ret.type = Expression::FUNCTION;
        ret.data = list;
        return ret;
    }

    std::any visitOC_FunctionName(LcypherParser::OC_FunctionNameContext *ctx) override {
        if (ctx->EXISTS()) {
            return std::string("EXISTS");
        }
        std::string name;
        if (ctx->oC_Namespace()) {
            std::string tmp = std::any_cast<std::string>(visit(ctx->oC_Namespace()));
            name = std::move(tmp);
        }
        name.append(ctx->oC_SymbolicName()->getText());
        return name;
    }

    std::any visitOC_ExplicitProcedureInvocation(
        LcypherParser::OC_ExplicitProcedureInvocationContext *ctx) override {
        std::string procedure_name = std::any_cast<std::string>(visit(ctx->oC_ProcedureName()));
        std::vector<Expression> parameters;
        for (auto &e : ctx->oC_Expression())
            parameters.emplace_back(std::any_cast<Expression>(visit(e)));
        return std::make_tuple(procedure_name, parameters);
    }

    std::any visitOC_ImplicitProcedureInvocation(
        LcypherParser::OC_ImplicitProcedureInvocationContext *ctx) override {
        std::string procedure_name = std::any_cast<std::string>(visit(ctx->oC_ProcedureName()));
        std::vector<Expression> parameters;
        return std::make_tuple(procedure_name, parameters);
    }

    std::any visitOC_ProcedureResultField(
        LcypherParser::OC_ProcedureResultFieldContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_ProcedureName(LcypherParser::OC_ProcedureNameContext *ctx) override {
        std::string name;
        if (ctx->oC_Namespace()) {
            std::string tmp = std::any_cast<std::string>(visit(ctx->oC_Namespace()));
            name = std::move(tmp);
        }
        name.append(ctx->oC_SymbolicName()->getText());
        return name;
    }

    std::any visitOC_Namespace(LcypherParser::OC_NamespaceContext *ctx) override {
        std::string name_space;
        for (auto &s : ctx->oC_SymbolicName()) name_space.append(s->getText()).append(".");
        return name_space;
    }

    std::any visitOC_ListComprehension(
        LcypherParser::OC_ListComprehensionContext *ctx) override {
        auto var = ctx->oC_FilterExpression()->oC_IdInColl()->oC_Variable()->getText();
        Expression var_expr;
        var_expr.type = Expression::VARIABLE;
        var_expr.data = std::make_shared<Expression::EXPR_TYPE_STRING>(var);
        Expression in_expr = std::any_cast<Expression>(
            visit(ctx->oC_FilterExpression()->oC_IdInColl()->oC_Expression()));
        Expression where_expr;
        Expression vert_expr;
        _listcompr_placeholder = var;
        if (ctx->oC_FilterExpression()->oC_Where()) {
            where_expr = std::any_cast<Expression>(visit(ctx->oC_FilterExpression()->oC_Where()));
        }
        if (ctx->oC_Expression()) {
            vert_expr = std::any_cast<Expression>(visit(ctx->oC_Expression()));
        } else {
            CYPHER_TODO();
        }
        _listcompr_placeholder = "";
        auto list = std::make_shared<Expression::EXPR_LIST_COMPREHENSION>();
        list->emplace_back(var_expr);
        list->emplace_back(in_expr);
        list->emplace_back(where_expr);
        list->emplace_back(vert_expr);
        Expression ret;
        ret.type = Expression::LIST_COMPREHENSION;
        ret.data = list;
        return ret;
    }

    std::any visitOC_PatternComprehension(
        LcypherParser::OC_PatternComprehensionContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_PropertyLookup(LcypherParser::OC_PropertyLookupContext *ctx) override {
        std::string key_name = std::any_cast<std::string>(visit(ctx->oC_PropertyKeyName()));
        return key_name;
    }

    std::any visitOC_CaseExpression(LcypherParser::OC_CaseExpressionContext *ctx) override {
        Expression::EXPR_TYPE_CASE case_expr;
        case_expr.second = ctx->oC_Expression().empty()       ? 0
                           : ctx->oC_Expression().size() == 2 ? 3
                           : ctx->ELSE() != nullptr           ? 1
                                                              : 2;
        for (auto ca : ctx->oC_CaseAlternatives()) {
            std::pair<Expression, Expression> ca_expr =
                std::any_cast<std::pair<Expression, Expression>>(visit(ca));
            case_expr.first.emplace_back(ca_expr.first);
            case_expr.first.emplace_back(ca_expr.second);
        }
        for (auto e : ctx->oC_Expression()) {
            // head or/and tail
            Expression expr = std::any_cast<Expression>(visit(e));
            case_expr.first.emplace_back(expr);
        }
        Expression ret;
        ret.type = Expression::CASE;
        ret.data = std::make_shared<Expression::EXPR_TYPE_CASE>(std::move(case_expr));
        return ret;
    }

    std::any visitOC_CaseAlternatives(
        LcypherParser::OC_CaseAlternativesContext *ctx) override {
        Expression e0 = std::any_cast<Expression>(visit(ctx->oC_Expression(0)));
        Expression e1 = std::any_cast<Expression>(visit(ctx->oC_Expression(1)));
        return std::make_pair(e0, e1);
    }

    std::any visitOC_Variable(LcypherParser::OC_VariableContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_NumberLiteral(LcypherParser::OC_NumberLiteralContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_MapLiteral(LcypherParser::OC_MapLiteralContext *ctx) override {
        MAP_MAP_LITERAL map_literal;
        Expression expr;
        int i = 0;
        while (ctx->oC_PropertyKeyName(i) != nullptr) {
            std::string name = std::any_cast<std::string>(visit(ctx->oC_PropertyKeyName(i)));
            if (map_literal.find(name) != map_literal.end())
                throw lgraph::ParserException("Key duplicate in map: " + ctx->getText());
            Expression e = std::any_cast<Expression>(visit(ctx->oC_Expression(i)));
            map_literal.emplace(name, e);
            i++;
        }
        expr.type = Expression::MAP;
        expr.data = std::make_shared<Expression::EXPR_TYPE_MAP>(std::move(map_literal));
        return expr;
    }

    std::any visitOC_Parameter(LcypherParser::OC_ParameterContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_PropertyExpression(
        LcypherParser::OC_PropertyExpressionContext *ctx) override {
        Expression expr;
        Expression atom = std::any_cast<Expression>(visit(ctx->oC_Atom()));
        std::string key_name = std::any_cast<std::string>(visit(ctx->oC_PropertyLookup(0)));
        CYPHER_PARSER_CHECK(atom.type == Expression::VARIABLE, ctx->getText());
        expr.type = Expression::PROPERTY;
        expr.data =
            std::make_shared<Expression::EXPR_TYPE_PROPERTY>(std::move(atom), std::move(key_name));
        return expr;
    }

    std::any visitOC_PropertyKeyName(
        LcypherParser::OC_PropertyKeyNameContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_IntegerLiteral(LcypherParser::OC_IntegerLiteralContext *ctx) override {
        Expression expr;
        expr.type = Expression::INT;
        expr.data = std::stol(ctx->getText());
        return expr;
    }

    std::any visitOC_DoubleLiteral(LcypherParser::OC_DoubleLiteralContext *ctx) override {
        Expression expr;
        expr.type = Expression::DOUBLE;
        expr.data = std::stod(ctx->getText());
        return expr;
    }

    std::any visitOC_SchemaName(LcypherParser::OC_SchemaNameContext *ctx) override {
        /* issue #225:
         * schema_name may be symbolic_name or reserved_word, here, we also treat reserved_word as
         * string literal.
         */
        if (ctx->oC_ReservedWord()) {
            return ctx->getText();
        } else {
            // ctx->oC_SymbolicName()
            return visitOC_SymbolicName(ctx->oC_SymbolicName());
        }
        return visitChildren(ctx);
    }

    std::any visitOC_ReservedWord(LcypherParser::OC_ReservedWordContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_SymbolicName(LcypherParser::OC_SymbolicNameContext *ctx) override {
        std::string name = ctx->getText();
        return name;
    }

    std::any visitOC_LeftArrowHead(LcypherParser::OC_LeftArrowHeadContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_RightArrowHead(LcypherParser::OC_RightArrowHeadContext *ctx) override {
        return visitChildren(ctx);
    }

    std::any visitOC_Dash(LcypherParser::OC_DashContext *ctx) override {
        return visitChildren(ctx);
    }

    bool _IsVariableDefined(const std::string &var) {
        if (var.empty()) {
            return true;
        }

        // TODO(anyone) The Lcypher parser parse the following `WORD` in StandaloneCall parameter as
        // VARIABLE IT SHOULD BE PARSED AS KEYWORD. THIS IS A WORKAROUND WAY. SEE ISSUE: #164
        static const std::vector<std::string> excluded_set = {
            "INT8",   "INT16", "INT32",    "INT64", "FLOAT", "DOUBLE",
            "STRING", "DATE",  "DATETIME", "BLOB",  "BOOL",
            "POINT", "LINESTRING", "POLYGON", "SPATIAL"
        };
        if (std::find_if(excluded_set.begin(), excluded_set.end(), [&var](const std::string &kw) {
                std::string upper_var(var.size(), ' ');
                std::transform(var.begin(), var.end(), upper_var.begin(), ::toupper);
                return upper_var == kw;
            }) != excluded_set.end()) {
            return true;
        }

        const auto &symbols = _query[_curr_query].parts[_curr_part].symbol_table.symbols;
        return symbols.find(var) != symbols.end();
    }

#define CLAUSE_HELPER_FUNC(clause)                                                   \
    inline bool _InClause##clause() {                                                \
        return (_curr_clause_type & static_cast<_ClauseType>(clause)) != NA;         \
    }                                                                                \
    inline void _EnterClause##clause() {                                             \
        _curr_clause_type = static_cast<_ClauseType>(_curr_clause_type | clause);    \
    }                                                                                \
    inline void _LeaveClause##clause() {                                             \
        _curr_clause_type = static_cast<_ClauseType>(_curr_clause_type & (~clause)); \
    }

    CLAUSE_HELPER_FUNC(MATCH);
    CLAUSE_HELPER_FUNC(RETURN);
    CLAUSE_HELPER_FUNC(WITH);
    CLAUSE_HELPER_FUNC(UNWIND);
    CLAUSE_HELPER_FUNC(WHERE);
    CLAUSE_HELPER_FUNC(ORDERBY);
    CLAUSE_HELPER_FUNC(CREATE);
    CLAUSE_HELPER_FUNC(DELETE);
    CLAUSE_HELPER_FUNC(SET);
    CLAUSE_HELPER_FUNC(REMOVE);
    CLAUSE_HELPER_FUNC(MERGE);
    CLAUSE_HELPER_FUNC(INQUERYCALL);
};

}  // namespace parser
