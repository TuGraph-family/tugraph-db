/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "cypher/execution_plan/ops/ops.h"
#include "cypher/execution_plan/ops/op_immediate_argument.h"
#include "acl_visitor.h"

namespace cypher {

void ACLVisitor::Traverse(const OpBase &op) {
    size_t child_num = op.children.size();
    for (size_t i = 0; i < child_num; i++)
        Traverse(*op.children[child_num-1-i]);
    FMA_DBG() << "op type: " << op.type << ", "
              << "op name: " << op.name << ", " << "op children size: "
              << op.children.size();
    op.Accept(this);
}

void ACLVisitor::Visit(const OpBase &op) {
    if (ctx_->field_access_.empty()) return;
    Traverse(op);
    FMA_DBG() << "---------- VisitPlan ----------";
    for (const auto& alias : alias_label_) {
        FMA_DBG() << "[alias_label_] "
                  << "alias: " << alias.first
                  << ", is_vertex: " << int(alias.second.first)
                  << ", label:" << alias.second.second;
    }
    FMA_DBG() << "---------- visited_fields_ ----------";
    for (const auto& field : visited_access_fields_) {
        FMA_DBG() << "[visited_fields_] "
                  << "is_vertex: " << field.ld_spec.is_vertex
                  << ", label: " << field.ld_spec.label
                  << ", field: " << field.ld_spec.field
                  << ", field_access_level: " << int(field.access_level);
    }
    FMA_DBG() << "---------- VisitPlan ----------";
    lgraph::FieldAccessLevel min_level = lgraph::FieldAccessLevel::WRITE;
    for (auto &field_access : ctx_->field_access_) {
        if (min_level > field_access.second) min_level = field_access.second;
    }
    for (auto &visited_access_field_ : visited_access_fields_) {
        if (visited_access_field_.ld_spec.label.empty() &&
            visited_access_field_.ld_spec.field.empty() &&
            !ctx_->field_access_.empty() &&
            min_level < visited_access_field_.access_level)
            throw lgraph::CypherException(fma_common::StringFormatter::Format(
                "Access denied: No access permission."
                ));
        for (auto &field_access : ctx_->field_access_) {
            if ((visited_access_field_.ld_spec.label.empty()
                 && field_access.first.is_vertex == visited_access_field_.ld_spec.is_vertex
                 && field_access.first.field == visited_access_field_.ld_spec.field
                 && field_access.second < visited_access_field_.access_level) ||
                (!visited_access_field_.ld_spec.label.empty()
                 && !visited_access_field_.ld_spec.field.empty()
                 && field_access.first == visited_access_field_.ld_spec
                 && field_access.second < visited_access_field_.access_level) ||
                (!visited_access_field_.ld_spec.label.empty()
                 && visited_access_field_.ld_spec.field.empty()
                 && field_access.first.label == visited_access_field_.ld_spec.label
                 && field_access.first.is_vertex == visited_access_field_.ld_spec.is_vertex
                 && field_access.second < visited_access_field_.access_level))
                throw lgraph::CypherException(fma_common::StringFormatter::Format(
                    "Access denied: No {}:{} {} permission.",
                    field_access.first.label, field_access.first.field,
                    lgraph_api::to_string(visited_access_field_.access_level)));
        }
    }
}

void ACLVisitor::Visit(const Aggregate &op) {
    const auto& noneaggregated_expressions = op.NoneAggregatedExpressions();
    const auto& noneaggr_item_names = op.NoneAggrItemNames();
    const auto& aggregated_expressions = op.AggregatedExpressions();
    const auto& aggr_item_names = op.AggrItemNames();

    FMA_DBG() << "Aggregate: " << noneaggregated_expressions.size()
              << ", " << noneaggr_item_names.size()
              << ", " << aggregated_expressions.size()
              << ", " << aggr_item_names.size();

    for (size_t i = 0; i < noneaggregated_expressions.size(); i++) {
        auto& ae = noneaggregated_expressions[i];
        auto& alias = noneaggr_item_names[i];
        if (ae.type == cypher::ArithExprNode::AR_EXP_OPERAND &&
            ae.operand.type == cypher::ArithOperandNode::AR_OPERAND_VARIADIC) {
            auto it = alias_label_.find(ae.operand.variadic.alias);
            if (it == alias_label_.end())
                throw lgraph::CypherException(fma_common::StringFormatter::Format(
                    "alias_label_ can't not find alias: {}", ae.operand.variadic.alias));
            auto label = it->second;
            if (it->second.first != SymbolNode::Type::NODE &&
                it->second.first != SymbolNode::Type::RELATIONSHIP) continue;
            if (ae.operand.variadic.entity_prop.empty()) continue;
            visited_access_fields_.emplace(
                AccessField(label.first == SymbolNode::Type::NODE, label.second,
                            ae.operand.variadic.entity_prop, lgraph::FieldAccessLevel::READ));
        } else {
            // todo: handle expression, with n.name+m.name as f
        }
    }
    for (size_t i = 0; i < aggregated_expressions.size(); i++) {
        auto& ae = aggregated_expressions[i];
        auto& alias = aggr_item_names[i];
        // todo: handle expression, with max(n.name+m.name) as f
    }
    // AS
    for (size_t i = 0; i < noneaggregated_expressions.size(); i++) {
        auto &ae = noneaggregated_expressions[i];
        auto &alias = noneaggr_item_names[i];
        if (ae.ToString() == alias)
            continue;
        if (ae.type == cypher::ArithExprNode::AR_EXP_OPERAND &&
            ae.operand.type == cypher::ArithOperandNode::AR_OPERAND_VARIADIC) {
            if (ae.operand.variadic.alias == ae.ToString()) {
                alias_label_[alias] = alias_label_[ae.operand.variadic.alias];
                alias_label_.erase(ae.operand.variadic.alias);
            } else {
                // alias_label_.erase(ae.operand.variadic.alias);
                alias_label_[alias] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
            }
        } else {
            alias_label_[alias] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
        }
    }
    for (size_t i = 0; i < aggregated_expressions.size(); i++) {
        auto &ae = aggregated_expressions[i];
        auto &alias = aggr_item_names[i];
        alias_label_[alias] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
    }
}

void ACLVisitor::Visit(const AllNodeScan &op) {
    auto node = op.GetNode();
    alias_label_[node->Alias()] = std::make_pair(SymbolNode::Type::NODE, node->Label());
}

void ACLVisitor::Visit(const AllNodeScanDynamic &op) {
    auto node = op.GetNode();
    alias_label_[node->Alias()] = std::make_pair(SymbolNode::Type::NODE, node->Label());
}

void ACLVisitor::Visit(const Apply &op) {
    // nothing to do
}

void ACLVisitor::Visit(const Argument &op) {
    // nothing to do
}

void ACLVisitor::Visit(const CartesianProduct &op) {
    // nothing to do
}

void ACLVisitor::Visit(const OpCreate &op) {
    const auto& sym_tab_ = op.SymTab();
    auto pattern_graph = op.GetPatternGraph();
    for (auto& sym : sym_tab_.symbols) {
        auto alias = sym.first;
        auto label = pattern_graph->GetNode(alias).Label();
        SymbolNode::Type t = SymbolNode::Type::CONSTANT;
        if (sym.second.type == SymbolNode::Type::NODE)
            t = SymbolNode::Type::NODE;
        else if (sym.second.type == SymbolNode::Type::RELATIONSHIP)
            t = SymbolNode::Type::RELATIONSHIP;
        alias_label_[alias] = std::make_pair(t, label);
        visited_access_fields_.emplace(
            AccessField(t == SymbolNode::Type::NODE, label,
                        std::string(), lgraph::FieldAccessLevel::WRITE));
    }
}

void ACLVisitor::Visit(const OpDelete &op) {
    const auto& delete_data = op.GetDeleteData();
    for (auto& delete_item : delete_data) {
        auto alias = delete_item.ToString();
        auto it = alias_label_.find(alias);
        if (it == alias_label_.end())
            throw lgraph::CypherException(fma_common::StringFormatter::Format(
                "alias_label_ can't not find alias: {}", alias));
        auto label = it->second;
        visited_access_fields_.emplace(
            AccessField(label.first == SymbolNode::Type::NODE, label.second,
                        std::string(), lgraph::FieldAccessLevel::WRITE));
    }
}

void ACLVisitor::Visit(const Distinct &op) {
    // nothing to do
}

void ACLVisitor::Visit(const ExpandAll &op) {
    auto start = op.GetStartNode();
    auto neighbor = op.GetNeighborNode();
    auto repl = op.GetRelationship();
    alias_label_[start->Alias()] = std::make_pair(SymbolNode::Type::NODE, start->Label());
    alias_label_[neighbor->Alias()] = std::make_pair(SymbolNode::Type::NODE, neighbor->Label());
    if (repl->Types().empty()) {
        alias_label_[repl->Alias()] = std::make_pair(SymbolNode::Type::RELATIONSHIP, std::string());
    } else {
        for (auto &edge_label : repl->Types())
            // todo: alias_label_ error
            alias_label_[repl->Alias()] = std::make_pair(SymbolNode::Type::RELATIONSHIP, edge_label);
    }
}

void ACLVisitor::Visit(const OpFilter &op) {
    const auto& filter = op.Filter();
    std::vector<std::shared_ptr<lgraph::Filter>> filters;
    filters.push_back(filter);
    for (size_t j = 0; j < filters.size(); j++) {
        auto cur_filter = filters[j];
        if (cur_filter != nullptr && cur_filter->Left() != nullptr)
            filters.push_back(cur_filter->Left());
        if (cur_filter != nullptr && cur_filter->Right() != nullptr)
            filters.push_back(cur_filter->Right());
        if (cur_filter->Type() == lgraph::Filter::RANGE_FILTER) {
            auto visited_fields = cur_filter->VisitedFields();
            for (auto& visited_field : visited_fields) {
                auto it = alias_label_.find(visited_field.first);
                if (it == alias_label_.end())
                    throw lgraph::CypherException(fma_common::StringFormatter::Format(
                        "alias_label_ can't not find alias: {}", visited_field.first));
                auto label = it->second;
                if (visited_field.second.empty()) continue;
                visited_access_fields_.emplace(
                    AccessField(label.first == SymbolNode::Type::NODE, label.second,
                                visited_field.second, lgraph::FieldAccessLevel::READ));
            }
        }
    }
}

void ACLVisitor::Visit(const InQueryCall &op) {
    const auto& call_clause = op.CallClause();
    for (auto& yield_items : std::get<2>(call_clause)) {
        alias_label_[yield_items] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
    }
}

void ACLVisitor::Visit(const Limit &op) {
    // nothing to do
}

void ACLVisitor::Visit(const OpMerge &op) {
    const auto& sym_tab_ = op.SymTab();
    auto pattern_graph = op.GetPatternGraph();
    for (auto& sym : sym_tab_.symbols) {
        auto alias = sym.first;
        std::vector<std::string> labels;
        std::string label;
        SymbolNode::Type t = SymbolNode::Type::CONSTANT;
        if (sym.second.type == SymbolNode::Type::NODE) {
            t = SymbolNode::Type::NODE;
            label = pattern_graph->GetNode(alias).Label();
        } else if (sym.second.type == SymbolNode::Type::RELATIONSHIP) {
            t = SymbolNode::Type::RELATIONSHIP;
            // todo: alias_label_ error
            for (auto &edge_label : pattern_graph->GetRelationship(alias).Types())
                label = edge_label;
        }
        alias_label_[alias] = std::make_pair(t, label);
        visited_access_fields_.emplace(
            AccessField(t == SymbolNode::Type::NODE, label,
                        std::string(), lgraph::FieldAccessLevel::WRITE));
    }
}

void ACLVisitor::Visit(const NodeByIdSeek &op) {
    // nothing to do
}

void ACLVisitor::Visit(const NodeByLabelScan &op) {
    auto node = op.GetNode();
    auto alias = node->Alias();
    auto label = std::make_pair(SymbolNode::Type::NODE, node->Label());
    alias_label_[alias] = label;
}

void ACLVisitor::Visit(const NodeByLabelScanDynamic &op) {
    auto node = op.GetNode();
    auto alias = node->Alias();
    auto label = std::make_pair(SymbolNode::Type::NODE, node->Label());
    alias_label_[alias] = label;
}

void ACLVisitor::Visit(const NodeIndexSeek &op) {
    auto node = op.GetNode();
    auto alias = node->Alias();
    auto label = std::make_pair(SymbolNode::Type::NODE, node->Label());
    alias_label_[alias] = label;
    auto field = node->Prop().field;
    if (field.empty()) return;
    visited_access_fields_.emplace(
        AccessField(label.first == SymbolNode::Type::NODE, label.second,
                    field, lgraph::FieldAccessLevel::READ));
}

void ACLVisitor::Visit(const NodeIndexSeekDynamic &op) {
    auto node = op.GetNode();
    auto alias = node->Alias();
    auto label = std::make_pair(SymbolNode::Type::NODE, node->Label());
    alias_label_[alias] = label;
    auto field = node->Prop().field;
    if (field.empty()) return;
    visited_access_fields_.emplace(
        AccessField(label.first == SymbolNode::Type::NODE, label.second,
                    field, lgraph::FieldAccessLevel::READ));
}

void ACLVisitor::Visit(const Optional &op) {
    // nothing to do
}

void ACLVisitor::Visit(const ProduceResults &op) {
    // nothing to do
}

void ACLVisitor::Visit(const Project &op) {
    const auto& return_elements = op.ReturnElements();
    const auto& return_alias = op.ReturnAlias();
    for (size_t i = 0; i < return_elements.size(); i++) {
        auto& ae = return_elements[i];
        auto& alias = return_alias[i];
        if (ae.type == cypher::ArithExprNode::AR_EXP_OPERAND &&
            ae.operand.type == cypher::ArithOperandNode::AR_OPERAND_VARIADIC) {
            auto it = alias_label_.find(ae.operand.variadic.alias);
            if (it == alias_label_.end())
                throw lgraph::CypherException(fma_common::StringFormatter::Format(
                    "alias_label_ can't not find alias: {}", ae.operand.variadic.alias));
            auto label = it->second;
            if (it->second.first != SymbolNode::Type::NODE &&
                it->second.first != SymbolNode::Type::RELATIONSHIP) continue;
            if (ae.operand.variadic.entity_prop.empty()) continue;
            visited_access_fields_.emplace(
                AccessField(label.first == SymbolNode::Type::NODE, label.second,
                            ae.operand.variadic.entity_prop, lgraph::FieldAccessLevel::READ));
        } else {
            // todo: handle expression, with n.name+m.name as f
        }
    }
    // AS
    for (size_t i = 0; i < return_elements.size(); i++) {
        auto &ae = return_elements[i];
        auto &alias = return_alias[i];
        if (ae.ToString() == alias)
            continue;
        if (ae.type == cypher::ArithExprNode::AR_EXP_OPERAND &&
            ae.operand.type == cypher::ArithOperandNode::AR_OPERAND_VARIADIC) {
            if (ae.operand.variadic.alias == ae.ToString()) {
                alias_label_[alias] = alias_label_[ae.operand.variadic.alias];
                alias_label_.erase(ae.operand.variadic.alias);
            } else {
                alias_label_[alias] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
            }
        } else {
            alias_label_[alias] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
        }
    }
}

void ACLVisitor::Visit(const RelationshipCount &op) {
    // nothing to do
}

void ACLVisitor::Visit(const OpRemove &op) {
    FMA_DBG() << op.ToString();
    throw lgraph::CypherException("-----AclVisitor OpRemove Visitor Not Implemented-----");
}

void ACLVisitor::Visit(const OpSet &op) {
    std::string alias;
    std::string field;
    const auto& set_data = op.GetSetData();
    for (auto& d : set_data) {
        for (auto &set_item : d) {
            auto &lhs_var = std::get<0>(set_item);
            auto &lhs_prop_expr = std::get<1>(set_item);
            if (lhs_var.empty()) {
                auto &property = lhs_prop_expr.Property();
                auto &key_expr = property.first;
                if (key_expr.type != parser::Expression::VARIABLE) CYPHER_TODO();
                alias = key_expr.String();
                field = property.second;
            } else {
                alias = lhs_var;
            }
            auto it = alias_label_.find(alias);
            if (it == alias_label_.end())
                throw lgraph::CypherException(fma_common::StringFormatter::Format(
                    "alias_label_ can't not find alias: {}", alias));
            auto label = it->second;
            visited_access_fields_.emplace(AccessField(label.first == SymbolNode::Type::NODE,
                                                       label.second, field,
                                                       lgraph::FieldAccessLevel::WRITE));
        }
    }
}

void ACLVisitor::Visit(const Skip &op) {
    // nothing to do
}

void ACLVisitor::Visit(const Sort &op) {
    // nothing to do
}

void ACLVisitor::Visit(const StandaloneCall &op) {
    // nothing to do
}

void ACLVisitor::Visit(const TopN &op) {
    // nothing to do
}

void ACLVisitor::Visit(const Union &op) {
    // nothing to do
}

void ACLVisitor::Visit(const Unwind &op) {
    auto alias = op.ResolvedName();
    alias_label_[alias] = std::make_pair(SymbolNode::Type::CONSTANT, std::string());
}

void ACLVisitor::Visit(const VarLenExpand &op) {
    auto start = op.GetStartNode();
    auto neighbor = op.GetNeighborNode();
    auto repl = op.GetRelationship();
    alias_label_[start->Alias()] = std::make_pair(SymbolNode::Type::NODE, start->Label());
    alias_label_[neighbor->Alias()] = std::make_pair(SymbolNode::Type::NODE, neighbor->Label());
    alias_label_[repl->Alias()] = std::make_pair(SymbolNode::Type::NAMED_PATH, std::string());
}

void ACLVisitor::Visit(const VarLenExpandInto &op) {
    FMA_DBG() << op.ToString();
    throw lgraph::CypherException(
        "-----AclVisitor VarLenExpandInto Visitor Not Implemented-----");
}

void ACLVisitor::Visit(const ImmediateArgument &op) {
    FMA_DBG() << op.ToString();
    throw lgraph::CypherException(
        "-----AclVisitor ImmediateArgument Visitor Not Implemented-----");
}
}  // namespace cypher
