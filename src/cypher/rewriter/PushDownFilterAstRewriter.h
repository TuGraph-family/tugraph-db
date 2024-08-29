/**
* Copyright 2022 AntGroup CO., Ltd.
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

#pragma once

namespace cypher {

using namespace cypher;
class PushDownFilterAstRewriter : public AstNodeVisitorImpl {
 public:
    explicit PushDownFilterAstRewriter(geax::common::ObjectArenaAllocator& allocator,
                                       RTContext *ctx)
        : allocator_(allocator), ctx(ctx) {}
    std::any visit(geax::frontend::GraphPattern* node) override {
        if (ctx->graph_.empty()) {
            return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
        }
        auto graph = ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_);
        auto txn = graph.CreateReadTxn();
        const auto &si = txn.GetSchemaInfo();
        PushDownFilterAstV2(node, si);
        return geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    }

 private:
    void RewriteFilterAstV2(geax::frontend::Expr* &where,
                            geax::frontend::GraphPattern* node,
                            const lgraph::SchemaInfo& si) {
        auto node_add_filler = [&node, &si, this](
                                   geax::frontend::Node* n,
                                   const std::string& node_name,
                                   const std::string& node_field,
                                   geax::frontend::Expr* right) {
            if (n != nullptr &&
                n->filler() != nullptr &&
                n->filler()->v().has_value() &&
                n->filler()->v().value() == node_name &&
                n->filler()->label().has_value()) {
                geax::frontend::SingleLabel *label_node;
                checkedCast(n->filler()->label().value(), label_node);
                std::string label = label_node->label();
                auto s = si.v_schema_manager.GetSchema(label);
                if (!s) {
                    THROW_CODE(CypherException, "No such vertex label: {}", label);
                }
                if (s->GetPrimaryField() == node_field) {
                    auto ps = allocator_.allocate<geax::frontend::PropStruct>();
                    ps->appendProperty(const_cast<std::string &&>(node_field), right);
                    n->filler()->appendPredicate(ps);
                    return true;
                }
            }
            return false;
        };
        if (where->type() == geax::frontend::AstNodeType::kBEqual) {
            geax::frontend::BEqual *equal;
            checkedCast(where, equal);
            auto left = equal->left();
            auto right = equal->right();
            bool rewrite = false;
            if (left->type() != geax::frontend::AstNodeType::kGetField ||
                (right->type() != geax::frontend::AstNodeType::kVString &&
                right->type() != geax::frontend::AstNodeType::kVBool &&
                right->type() != geax::frontend::AstNodeType::kVDate &&
                right->type() != geax::frontend::AstNodeType::kVDatetime &&
                right->type() != geax::frontend::AstNodeType::kVDouble &&
                right->type() != geax::frontend::AstNodeType::kVDuration &&
                right->type() != geax::frontend::AstNodeType::kVInt &&
                right->type() != geax::frontend::AstNodeType::kVNone &&
                right->type() != geax::frontend::AstNodeType::kVNull &&
                right->type() != geax::frontend::AstNodeType::kVSome &&
                right->type() != geax::frontend::AstNodeType::kVTime)) {
                return;
            }
            geax::frontend::GetField *get_field;
            checkedCast(left, get_field);
            geax::frontend::Ref *node_name_ref;
            checkedCast(get_field->expr(), node_name_ref);
            std::string node_name = node_name_ref->name(), node_field = get_field->fieldName();
            for (auto &path_pattern : node->pathPatterns()) {
                for (auto &path_chain : path_pattern->chains()) {
                    rewrite |= node_add_filler(path_chain->head(), node_name, node_field, right);
                    for (auto [edge, end_node] : path_chain->tails()) {
                        rewrite |= node_add_filler(end_node, node_name, node_field, right);
                    }
                }
            }
            if (rewrite) {
                where = nullptr;
            }
            return;
        }
        if (where->type() == geax::frontend::AstNodeType::kBAnd) {
            geax::frontend::BAnd *bAnd;
            checkedCast(where, bAnd);
            auto where_left = bAnd->left(), where_right = bAnd->right();
            RewriteFilterAstV2(where_left, node, si);
            RewriteFilterAstV2(where_right, node, si);
            if (where_left == nullptr && where_right != nullptr) {
                where = where_right;
            } else if (where_left != nullptr && where_right == nullptr) {
                where = where_left;
            } else if (where_left == nullptr && where_right == nullptr) {
                where = nullptr;
            }
        }
    }

    /**
     * before:
     *     MATCH (n:person),(m:movie) where n.id = 1 and m.id = 1 ...
     * after:
     *     MATCH (n:person {id:1}),(m:movie {id:1}) ...
     */

    /**
     * before:
     *     MATCH (n:person)-[r]-(m:movie)-[r1]-(m1:user) where n.id = 10 and m.id = 1 and m1.id > 10 ...
     * after:
     *     MATCH (n:person {id:10})-[r]-(m:movie {id:1})-[r1]-(m1:user) where m1.id > 10 ...
     */

    void PushDownFilterAstV2(geax::frontend::GraphPattern* node, const lgraph::SchemaInfo& si) {
        if (node->where().has_value()) {
            auto where = node->where().value();
            RewriteFilterAstV2(where, node, si);
            if (where == nullptr) {
                node->resetWhere();
            } else {
                node->setWhere(where);
            }
        }
    }

    geax::common::ObjectArenaAllocator& allocator_;
    RTContext *ctx;
};

}  // namespace cypher
