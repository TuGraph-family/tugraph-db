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
// Created by ljp on 20-2-16.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"
#include "parser/clause.h"

namespace cypher {

class OpMerge : public OpBase {
    const SymbolTable &sym_tab_;
    std::vector<parser::Clause::TYPE_MERGE> merge_data_;
    PatternGraph *pattern_graph_ = nullptr;
    bool standalone_;
    bool summary_;

    static void ExtractProperties(const parser::TUP_PROPERTIES &properties, VEC_STR &fields,
                                  std::vector<lgraph::FieldData> &values) {
        using namespace parser;
        Expression map_literal = std::get<0>(properties);
        CYPHER_THROW_ASSERT(map_literal.type == Expression::NA ||
                            map_literal.type == Expression::MAP);
        if (map_literal.type != Expression::MAP) return;
        for (auto &prop : map_literal.Map()) {
            fields.emplace_back(prop.first);
            Expression p;
            if (prop.second.type == Expression::LIST) {
                p = prop.second.List().at(0);
            } else if (prop.second.type == Expression::MAP) {
                CYPHER_TODO();
            } else {
                p = prop.second;
            }
            switch (p.type) {
            case Expression::INT:
                values.emplace_back(p.Int());
                break;
            case Expression::DOUBLE:
                values.emplace_back(p.Double());
                break;
            case Expression::STRING:
                values.emplace_back(p.String());
                break;
            case Expression::NULL_:
            default:
                CYPHER_TODO();
            }
        }
    }

    static void ExtractProperties(const std::string &var, const parser::VEC_SET &set_items,
                                  VEC_STR &fields, std::vector<lgraph::FieldData> &values) {
        using namespace parser;
        for (auto &set_item : set_items) {
            auto &prop_expr = std::get<1>(set_item);
            if (prop_expr.Property().first.ToString() != var) {
                throw lgraph::CypherException(fma_common::StringFormatter::Format(
                    "Variable `{}` not found", prop_expr.Property().first.ToString()));
            }
            auto &prop_value = std::get<3>(set_item);
            fields.emplace_back(prop_expr.Property().second);  // property_expr filed name
            switch (prop_value.type) {
            case Expression::INT:
                values.emplace_back(prop_value.Int());
                break;
            case Expression::DOUBLE:
                values.emplace_back(prop_value.Double());
                break;
            case Expression::STRING:
                values.emplace_back(prop_value.String());
                break;
            case Expression::NULL_:
            default:
                CYPHER_TODO();
            }
        }
    }

    struct MatchIterator {
        std::string label;
        std::vector<std::string> field_names;
        std::vector<lgraph_api::FieldData> field_values;
        MatchIterator(lgraph::Transaction *txn, const std::string &l,
                      const std::vector<std::string> &fns,
                      const std::vector<lgraph_api::FieldData> &fvs, Node *n)
            : label(l), field_names(fns), field_values(fvs), _txn(txn), node_p(n) {
            if (field_names.size() == field_values.size()) {
                /* there must be at least 1 valid index */
                for (int i = 0; i < (int)field_names.size(); i++) {
                    if (!_txn->IsIndexed(label, field_names[i])) continue;
                    _iit = new lgraph::VertexIndexIterator(_txn->GetVertexIndexIterator(
                        label, field_names[i], field_values[i], field_values[i]));
                    while (_iit->IsValid()) {
                        if (CheckFields()) {
                            /* ON MATCH */
                            valid_ = true;
                            break;
                        }
                        _iit->Next();
                    }
                    break;
                }
            }
        }

        DISABLE_COPY(MatchIterator);

        ~MatchIterator() {
            delete _iit;
            _iit = nullptr;
        }

        bool Next() {
            if (!_iit) return false;
            valid_ = false;
            while (_iit->IsValid()) {
                _iit->Next();
                if (_iit->IsValid() && CheckFields()) {
                    valid_ = true;
                    break;
                }
            }
            return valid_;
        }

        bool IsMatch() const { return node_p->derivation_ == Node::Derivation::MATCHED; }

        bool IsValid() const { return _iit && valid_; }

        bool IsIndexValid() const { return _iit != nullptr; }

        int64_t GetVid() const {
            return IsMatch() ? node_p->PullVid() : IsValid() ? _iit->GetVid() : -1;
        }

     private:
        lgraph::Transaction *_txn = nullptr;
        lgraph::VertexIndexIterator *_iit = nullptr;
        bool valid_ = false;
        Node *node_p;

        bool CheckFields() const {
            if (!_iit || !_iit->IsValid()) return false;
            auto vit = _txn->GetVertexIterator(_iit->GetVid());
            size_t j = 0;
            for (auto &fn : field_names) {
                if (_txn->GetVertexField(vit, fn) != field_values[j]) break;
                j++;
            }
            return j == field_names.size();
        }
    };

    lgraph::VertexId MergeVertex(RTContext *ctx, MatchIterator &&vertex,
                                 const std::vector<std::string> &field_names_on_create,
                                 const std::vector<lgraph_api::FieldData> &field_values_on_create,
                                 const std::vector<std::string> &field_names_on_match,
                                 const std::vector<lgraph_api::FieldData> &field_values_on_match,
                                 std::string &err) {
        lgraph::VertexId res = 0;
        /* check txn */
        if (!ctx->txn_->IsValid() || ctx->txn_->IsReadOnly()) {
            err = "invalid txn";
            return -1;
        }
        if (vertex.IsValid()) {
            /* ON MATCH */
            std::vector<lgraph::VertexId> match_vids;
            do {
                match_vids.emplace_back(vertex.GetVid());
                vertex.Next();
            } while (vertex.IsValid());
            auto vit = ctx->txn_->GetTxn()->GetVertexIterator();
            for (auto vid : match_vids) {
                vit.Goto(vid);
                ctx->txn_->GetTxn()->SetVertexProperty(vit, field_names_on_match,
                                                       field_values_on_match);
            }
            res = match_vids.back();
        } else if (vertex.IsIndexValid()) {
            /* ON CREATE */
            std::vector<std::string> fns;
            std::vector<lgraph_api::FieldData> fvs;
            fns.insert(fns.end(), vertex.field_names.begin(), vertex.field_names.end());
            fns.insert(fns.end(), field_names_on_create.begin(), field_names_on_create.end());
            fvs.insert(fvs.end(), vertex.field_values.begin(), vertex.field_values.end());
            fvs.insert(fvs.end(), field_values_on_create.begin(), field_values_on_create.end());
            int64_t vid = ctx->txn_->AddVertex(vertex.label, fns, fvs);
            res = vid;
        } else {
            err = "invalid index ";
            return -1;
        }
        return res;
    }

    lgraph_api::EdgeUid MergeEdge(RTContext *ctx, MatchIterator &&src, MatchIterator &&dst,
                                  const std::string &label,
                                  const std::vector<std::string> &field_names,
                                  const std::vector<lgraph_api::FieldData> &field_values,
                                  const std::vector<std::string> &field_names_on_create,
                                  const std::vector<lgraph_api::FieldData> &field_values_on_create,
                                  const std::vector<std::string> &field_names_on_match,
                                  const std::vector<lgraph_api::FieldData> &field_values_on_match,
                                  std::string &err) {
        /* check txn */
        lgraph_api::EdgeUid eid_res;
        if (!ctx->txn_->IsValid() || ctx->txn_->IsReadOnly()) {
            err = "invalid txn";
            return eid_res;
        }
        if ((!src.IsValid() && !src.IsMatch()) || (!dst.IsValid() && !dst.IsMatch())) {
            err = "invalid vertex";
            return eid_res;
        }
        if (field_names.size() != field_values.size()) {
            err = "number of fields and data values do not match";
            return eid_res;
        }
        std::vector<int64_t> src_ids, dst_ids;
        if (src.IsMatch()) {
            src_ids.emplace_back(src.GetVid());
        } else {
            while (src.IsValid()) {
                src_ids.emplace_back(src.GetVid());
                src.Next();
            }
        }
        if (dst.IsMatch()) {
            dst_ids.emplace_back(dst.GetVid());
        } else {
            while (dst.IsValid()) {
                dst_ids.emplace_back(dst.GetVid());
                dst.Next();
            }
        }
        for (auto src_id : src_ids) {
            for (auto dst_id : dst_ids) {
                auto eit = ctx->txn_->GetTxn()->GetVertexIterator(src_id).GetOutEdgeIterator();
                std::vector<lgraph::EdgeUid> match_eids;
                while (eit.IsValid()) {
                    if (eit.GetDst() == dst_id) {
                        size_t j = 0;
                        for (auto &fn : field_names) {
                            if (ctx->txn_->GetTxn()->GetEdgeField(eit, fn) != field_values[j])
                                break;
                            j++;
                        }
                        if (ctx->txn_->GetTxn()->GetEdgeLabel(eit) == label &&
                            j == field_names.size()) {
                            match_eids.emplace_back(src_id, dst_id, eit.GetLabelId(),
                                                    eit.GetTemporalId(), eit.GetEdgeId());
                        }
                    }
                    eit.Next();
                }
                if (!match_eids.empty()) {
                    /* ON MATCH */
                    for (auto uid : match_eids) {
                        eit.Goto(uid, true);
                        ctx->txn_->GetTxn()->SetEdgeProperty(eit, field_names_on_match,
                                                             field_values_on_match);
                    }
                    eid_res = match_eids.back();
                } else {
                    /* ON CREATE */
                    std::vector<std::string> fns;
                    std::vector<lgraph_api::FieldData> fvs;
                    fns.insert(fns.end(), field_names.begin(), field_names.end());
                    fns.insert(fns.end(), field_names_on_create.begin(),
                               field_names_on_create.end());
                    fvs.insert(fvs.end(), field_values.begin(), field_values.end());
                    fvs.insert(fvs.end(), field_values_on_create.begin(),
                               field_values_on_create.end());
                    eid_res = ctx->txn_->AddEdge(src_id, dst_id, label, fns, fvs);
                }
            }
        }
        return eid_res;
    }

    void MergeNode(RTContext *ctx, const parser::TUP_NODE_PATTERN &node_pattern,
                   const parser::VEC_SET &on_match_set_items,
                   const parser::VEC_SET &on_create_set_items) {
        VEC_STR field_names;
        std::vector<lgraph::FieldData> field_values;
        VEC_STR field_names_on_match;
        std::vector<lgraph::FieldData> field_values_on_match;
        VEC_STR field_names_on_create;
        std::vector<lgraph::FieldData> field_values_on_create;  // need to be initalized by ""
        std::string err_msg;
        auto node_variable = std::get<0>(node_pattern);  // string
        auto &node_labels = std::get<1>(node_pattern);   // VEC_STR
        auto &properties = std::get<2>(node_pattern);    // TUP_PROPERTIES
        auto &node_patt = pattern_graph_->GetNode(node_variable);
        if (node_labels.empty())
            throw lgraph::EvaluationException("vertex label missing in merge.");
        const std::string &node_label = *node_labels.begin();
        ExtractProperties(properties, field_names, field_values);  // Node parameters
        ExtractProperties(node_variable, on_match_set_items, field_names_on_match,
                          field_values_on_match);
        ExtractProperties(node_variable, on_create_set_items, field_names_on_create,
                          field_values_on_create);
        auto node_res = MergeVertex(ctx,
                                    MatchIterator(ctx->txn_->GetTxn().get(), node_label,
                                                  field_names, field_values, &node_patt),
                                    field_names_on_create, field_values_on_create,
                                    field_names_on_match, field_values_on_match, err_msg);
        // check node_res whether valid
        //
        // When given merge node pattern, if none of field names is indexed in given label
        // MergeVertex return a invalid node_res(-1)
        //
        // # Error
        // invalid cypher example:
        //      MERGE (n:null {id: 2909}) RETURN n
        // see issue: https://code.alipay.com/fma/tugraph-db/issues/340
        if (node_res == -1) {
            throw lgraph::CypherException("cannot match node with given label and properties");
        }

        // TODO(anyone) When multiple nodes are matched, all data should be processed
        ctx->result_info_->statistics.vertices_created++;
        if (!node_variable.empty()) {
            auto node = &pattern_graph_->GetNode(node_variable);
            if (node->Empty()) CYPHER_TODO();
            node->Visited() = true;
            node->ItRef()->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::VERTEX_ITER,
                                      node_res);
            if (!summary_) {
                auto it = sym_tab_.symbols.find(node_variable);
                CYPHER_THROW_ASSERT(it != sym_tab_.symbols.end());
                record->values[it->second.id].type = Entry::NODE;
                record->values[it->second.id].node = node;
            }
        }
    }

    void MergeChains(RTContext *ctx, const parser::TUP_NODE_PATTERN &node_patt,
                     const parser::TUP_NODE_PATTERN &rls_node_pattern,
                     const parser::TUP_RELATIONSHIP_PATTERN &rls_patt,
                     const parser::VEC_SET &on_match_set_items,
                     const parser::VEC_SET &on_create_set_items) {
        using namespace parser;
        auto src_node_var = std::get<0>(node_patt);
        auto dst_node_var = std::get<0>(rls_node_pattern);
        auto direction = std::get<0>(rls_patt);
        auto &relationship_detail = std::get<1>(rls_patt);
        if (direction == LinkDirection::RIGHT_TO_LEFT) {
            std::swap(src_node_var, dst_node_var);
        } else if (direction != LinkDirection::LEFT_TO_RIGHT) {
            throw lgraph::CypherException("Only directed relationships are supported in merge");
        }
        auto &edge_variable = std::get<0>(relationship_detail);
        auto &relationship_types = std::get<1>(relationship_detail);  // edge labels
        auto &properties = std::get<3>(relationship_detail);
        if (relationship_types.empty())
            throw lgraph::CypherException("edge label missing in merge");
        auto &src_node = pattern_graph_->GetNode(src_node_var);
        auto &dst_node = pattern_graph_->GetNode(dst_node_var);  // get node.Lable() get node.Prop()
        auto src = src_node.ItRef();
        auto dst = dst_node.ItRef();
        auto &label = relationship_types[0];
        if (!src->IsValid() || !dst->IsValid()) {
            std::string err = src->IsValid() ? dst_node_var : src_node_var;
            throw lgraph::CypherException("Invalid vertex when create edge: " + err);
        }
        const std::string &src_label = src_node.Label();
        // TODO(anyone) Currently only one field can be obtained
        const std::vector<std::string> &src_field_name{src_node.Prop().field};
        const std::vector<lgraph::FieldData> &src_field_value{src_node.Prop().value};
        const std::string &dst_label = dst_node.Label();
        const std::vector<std::string> dst_field_name{dst_node.Prop().field};
        const std::vector<lgraph::FieldData> dst_field_value{dst_node.Prop().value};
        VEC_STR fields;
        std::vector<lgraph::FieldData> values;
        VEC_STR field_names_on_match;
        std::vector<lgraph::FieldData> field_values_on_match;
        VEC_STR field_names_on_create;
        std::vector<lgraph::FieldData> field_values_on_create;
        std::string err_msg;
        ExtractProperties(properties, fields, values);
        ExtractProperties(edge_variable, on_match_set_items, field_names_on_match,
                          field_values_on_match);
        ExtractProperties(edge_variable, on_create_set_items, field_names_on_create,
                          field_values_on_create);
        auto res = MergeEdge(ctx,
                             MatchIterator(ctx->txn_->GetTxn().get(), src_label, src_field_name,
                                           src_field_value, &src_node),
                             MatchIterator(ctx->txn_->GetTxn().get(), dst_label, dst_field_name,
                                           dst_field_value, &dst_node),
                             label, fields, values, field_names_on_create, field_values_on_create,
                             field_names_on_match, field_values_on_match, err_msg);
        ctx->result_info_->statistics.edges_created++;
        if (!edge_variable.empty()) {
            auto relp = &pattern_graph_->GetRelationship(edge_variable);
            if (relp->Empty()) CYPHER_TODO();
            relp->ItRef()->Initialize(ctx->txn_->GetTxn().get(), res);
            if (!summary_) {
                auto it = sym_tab_.symbols.find(edge_variable);
                CYPHER_THROW_ASSERT(it != sym_tab_.symbols.end());
                record->values[it->second.id].type = Entry::RELATIONSHIP;
                record->values[it->second.id].relationship = relp;
            }
        }
    }

    void MergeVE(RTContext *ctx) {
        for (auto &pattern : merge_data_) {
            auto &pattern_part = std::get<0>(pattern);
            auto &match_items = std::get<1>(pattern);
            auto &create_items = std::get<2>(pattern);
            auto &pp_variable = std::get<0>(pattern_part);
            if (!pp_variable.empty()) CYPHER_TODO();
            auto pattern_element = std::get<1>(pattern_part);
            auto node_pattern = std::get<0>(pattern_element);
            auto pattern_element_chains = std::get<1>(pattern_element);
            if (pattern_element_chains.empty()) {  // create vertex
                MergeNode(ctx, node_pattern, match_items, create_items);
            } else {  // create chains
                for (auto &chain : pattern_element_chains) {
                    auto &rls_pattern = std::get<0>(chain);
                    auto &rls_node_patt = std::get<1>(chain);
                    auto &rls_node_var = std::get<0>(rls_node_patt);
                    auto &node_var = std::get<0>(node_pattern);
                    if (!pattern_graph_->GetNode(node_var).Visited()) {
                        // for eg: match(node1),(node2) or
                        // merge(m:person{name:""})
                        // -[r:konws{}]->(n:person{name:""})
                        MergeNode(ctx, node_pattern, match_items, create_items);
                    }
                    if (!pattern_graph_->GetNode(rls_node_var).Visited()) {
                        // for eg: merge(m:person{name:""})
                        // -[r:konws{}]->(n:person{name:""})
                        MergeNode(ctx, rls_node_patt, match_items, create_items);
                    }
                    MergeChains(ctx, node_pattern, rls_node_patt, rls_pattern, match_items,
                                create_items);
                    node_pattern = rls_node_patt;
                }
            }  // end else
        }
    }

    void ResultSummary(RTContext *ctx) {
        if (summary_) {
            std::string summary;
            summary.append("merged ")
                .append(std::to_string(ctx->result_info_->statistics.vertices_created))
                .append(" vertices, merged ")
                .append(std::to_string(ctx->result_info_->statistics.edges_created))
                .append(" edges.");
            CYPHER_THROW_ASSERT(ctx->result_->Header().size() == 1);
            CYPHER_THROW_ASSERT(record);
            record->values.clear();
            record->AddConstant(lgraph::FieldData(summary));
        } else {
            /* There should be a "Project" operation on top of this
             * "Create", so leave result set to it. */
        }
    }

 public:
    OpMerge(const parser::QueryPart *stmt, PatternGraph *pattern_graph)
        : OpBase(OpType::MERGE, "Merge"),
          sym_tab_(pattern_graph->symbol_table),
          pattern_graph_(pattern_graph) {
        for (auto &node : pattern_graph_->GetNodes()) {
            if (node.derivation_ == Node::MERGED) modifies.emplace_back(node.Alias());
            for (auto rr : node.RhsRelps()) {
                auto &r = pattern_graph_->GetRelationship(rr);
                if (r.derivation_ == Relationship::MERGED) modifies.emplace_back(r.Alias());
            }
        }
        for (auto c : stmt->merge_clause) {
            merge_data_.emplace_back(*c);
        }
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() < 2);
        summary_ = !parent->parent;
        standalone_ = children.empty();
        for (auto child : children) {
            child->Initialize(ctx);
        }
        record = children.empty() ?
            std::make_shared<Record>(sym_tab_.symbols.size(), &sym_tab_, ctx->param_tab_)
                                  : children[0]->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        if (standalone_) {
            MergeVE(ctx);
            ResultSummary(ctx);
            state = StreamDepleted;
            return OP_OK;
        } else {
            if (children.size() > 1) CYPHER_TODO();
            auto child = children[0];
            if (summary_) {
                while (child->Consume(ctx) == OP_OK) MergeVE(ctx);
                ResultSummary(ctx);
                state = StreamDepleted;
                return OP_OK;
            } else {
                if (child->Consume(ctx) == OP_OK) {
                    MergeVE(ctx);
                    return OP_OK;
                } else {
                    return OP_DEPLETED;
                }
            }
        }
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &m : modifies) str.append(m).append(",");
        if (!modifies.empty()) str.pop_back();
        str.append("]");
        return str;
    }

    const std::vector<parser::Clause::TYPE_MERGE> &MergeData() const { return merge_data_; }

    PatternGraph *GetPatternGraph() const { return pattern_graph_; }

    const SymbolTable &SymTab() const { return sym_tab_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
