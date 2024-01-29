/**
 * Copyright 2024 AntGroup CO., Ltd.
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
// Created by wt on 19-11-28.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class RelationshipCount : public OpBase {
    lgraph::Transaction *txn_;
    cypher::Node *start_;
    cypher::Node *neighbor_;
    cypher::Relationship *relp_;
    unsigned int pattern_type_ = 0;

    inline bool GetLabelIds(lgraph::LabelId &start_lid, lgraph::LabelId &nbr_lid,
                            std::vector<lgraph::LabelId> &relp_lids) {
        try {
            if (!start_->Label().empty()) start_lid = txn_->GetLabelId(true, start_->Label());
            if (!neighbor_->Label().empty()) nbr_lid = txn_->GetLabelId(true, neighbor_->Label());
        } catch (std::exception &) {
            return false;
        }
        for (auto &t : relp_->Types()) {
            lgraph::LabelId rl;
            try {
                rl = txn_->GetLabelId(false, t);
            } catch (std::exception &) {
                continue;
            }
            relp_lids.emplace_back(rl);
        }
        return true;
    }

    inline size_t CountAllOutEdges() const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            n += vit.GetNumOutEdges();
        }
        return n;
    }

    inline size_t CountOutEdgesWithStartLabel(lgraph::LabelId start_lid) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != start_lid) continue;
            n += vit.GetNumOutEdges();
        }
        return n;
    }

    inline size_t CountOutEdgesWithNeighborLabel(lgraph::LabelId nbr_lid) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != nbr_lid) continue;
            n += vit.GetNumInEdges();
        }
        return n;
    }

    inline size_t CountOutEdgesWithStartAndNeighborLabel(lgraph::LabelId start_lid,
                                                         lgraph::LabelId nbr_lid) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != start_lid) continue;
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (txn_->GetVertexLabelId(txn_->GetVertexIterator(eit.GetDst())) != nbr_lid) {
                    continue;
                }
                n++;
            }
        }
        return n;
    }

    inline size_t CountOutEdgesWithRelationshipLabels(
        const std::vector<lgraph::LabelId> &relp_lids) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            for (auto rl : relp_lids) {
                for (auto eit =
                         vit.GetOutEdgeIterator(lgraph::EdgeUid(vit.GetId(), 0, rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    n++;
                }
            }
        }
        return n;
    }

    inline size_t CountOutEdgesWithStartAndRelationshipLabels(
        lgraph::LabelId start_lid, const std::vector<lgraph::LabelId> &relp_lids) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != start_lid) continue;
            for (auto rl : relp_lids) {
                for (auto eit =
                         vit.GetOutEdgeIterator(lgraph::EdgeUid(vit.GetId(), 0, rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    n++;
                }
            }
        }
        return n;
    }

    inline size_t CountOutEdgesWithNeighborAndRelationshipLabels(
        lgraph::LabelId nbr_lid, const std::vector<lgraph::LabelId> &relp_lids) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != nbr_lid) continue;
            for (auto rl : relp_lids) {
                for (auto eit =
                         vit.GetInEdgeIterator(lgraph::EdgeUid(0, vit.GetId(), rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    n++;
                }
            }
        }
        return n;
    }

    inline size_t CountOutEdgesWithStartAndNeighborAndRelationshipLabels(
        lgraph::LabelId start_lid, lgraph::LabelId nbr_lid,
        const std::vector<lgraph::LabelId> &relp_lids) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != start_lid) continue;
            for (auto rl : relp_lids) {
                for (auto eit =
                         vit.GetOutEdgeIterator(lgraph::EdgeUid(vit.GetId(), 0, rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    if (txn_->GetVertexLabelId(txn_->GetVertexIterator(eit.GetDst())) != nbr_lid) {
                        continue;
                    }
                    n++;
                }
            }
        }
        return n;
    }

    inline size_t CountEdgesWithOneEndLabel(lgraph::LabelId oe_lid) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != oe_lid) continue;
            n += vit.GetNumOutEdges();
            n += vit.GetNumInEdges();
        }
        return n;
    }

    inline size_t CountEdgesWithStartAndNeighborLabel(lgraph::LabelId start_lid,
                                                      lgraph::LabelId nbr_lid) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != start_lid) continue;
            for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (txn_->GetVertexLabelId(txn_->GetVertexIterator(eit.GetDst())) != nbr_lid) {
                    continue;
                }
                n++;
            }
            for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) {
                if (txn_->GetVertexLabelId(txn_->GetVertexIterator(eit.GetSrc())) != nbr_lid) {
                    continue;
                }
                n++;
            }
        }
        return n;
    }

    inline size_t CountEdgesWithOneEndAndRelationshipLabels(
        lgraph::LabelId oe_lid, const std::vector<lgraph::LabelId> &relp_lids) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != oe_lid) continue;
            for (auto rl : relp_lids) {
                for (auto eit =
                         vit.GetOutEdgeIterator(lgraph::EdgeUid(vit.GetId(), 0, rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    n++;
                }
                for (auto eit =
                         vit.GetInEdgeIterator(lgraph::EdgeUid(0, vit.GetId(), rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    n++;
                }
            }
        }
        return n;
    }

    inline size_t CountEdgesWithStartAndNeighborAndRelationshipLabels(
        lgraph::LabelId start_lid, lgraph::LabelId nbr_lid,
        const std::vector<lgraph::LabelId> &relp_lids) const {
        size_t n = 0;
        for (auto vit = txn_->GetVertexIterator(); vit.IsValid(); vit.Next()) {
            if (txn_->GetVertexLabelId(vit) != start_lid) continue;
            for (auto rl : relp_lids) {
                for (auto eit =
                         vit.GetOutEdgeIterator(lgraph::EdgeUid(vit.GetId(), 0, rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    if (txn_->GetVertexLabelId(txn_->GetVertexIterator(eit.GetDst())) != nbr_lid) {
                        continue;
                    }
                    n++;
                }
                for (auto eit =
                         vit.GetInEdgeIterator(lgraph::EdgeUid(0, vit.GetId(), rl, 0, 0), true);
                     eit.IsValid(); eit.Next()) {
                    if (eit.GetLabelId() != rl) break;
                    if (txn_->GetVertexLabelId(txn_->GetVertexIterator(eit.GetSrc())) != nbr_lid) {
                        continue;
                    }
                    n++;
                }
            }
        }
        return n;
    }

 public:
    RelationshipCount(Node *start, Node *neighbor, Relationship *relp)
        : OpBase(OpType::RELATIONSHIP_COUNT, "Relationship Count"),
          start_(start),
          neighbor_(neighbor),
          relp_(relp) {
        /* decide pattern type */
        pattern_type_ |= start_->Label().empty() ? 0 : 0x1u << 0u;
        pattern_type_ |= neighbor_->Label().empty() ? 0 : 0x1u << 1u;
        pattern_type_ |= relp_->Types().empty() ? 0 : 0x1u << 2u;
        pattern_type_ |= !relp_->Undirected() ? 0 : 0x1u << 3u;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(children.empty());
        if (start_->Prop().type != Property::NUL || neighbor_->Prop().type != Property::NUL ||
            relp_->VarLen()) {
            CYPHER_TODO();
        }
        record = std::make_shared<Record>(1);
        record->values[0].type = Entry::CONSTANT;
        // TODO(anyone) remove txn totally
        txn_ = ctx->txn_->GetTxn().get();
        state = StreamUnInitialized;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        int64_t num_edges = 0;
        lgraph::LabelId start_lid = 0, nbr_lid = 0;
        std::vector<lgraph::LabelId> relp_lids;
        if (!GetLabelIds(start_lid, nbr_lid, relp_lids)) goto return_result;
        switch (pattern_type_) {
        case 0x0:  // ()-[r]->()
            num_edges = CountAllOutEdges();
            break;
        case 0x1:  // (:SL)-[r]->()
            num_edges = CountOutEdgesWithStartLabel(start_lid);
            break;
        case 0x2:  // ()-[r]->(:NL)
            num_edges = CountOutEdgesWithNeighborLabel(nbr_lid);
            break;
        case 0x3:  // (:SL)-[r]->(:NL)
            num_edges = CountOutEdgesWithStartAndNeighborLabel(start_lid, nbr_lid);
            break;
        case 0x4:  // ()-[r:RL]->()
            num_edges = CountOutEdgesWithRelationshipLabels(relp_lids);
            break;
        case 0x5:  // (:SL)-[r:RL]->()
            num_edges = CountOutEdgesWithStartAndRelationshipLabels(start_lid, relp_lids);
            break;
        case 0x6:  // ()-[r:RL]->(:NL)
            num_edges = CountOutEdgesWithNeighborAndRelationshipLabels(nbr_lid, relp_lids);
            break;
        case 0x7:  // (:SL)-[r:RL]->(:NL)
            num_edges = CountOutEdgesWithStartAndNeighborAndRelationshipLabels(start_lid, nbr_lid,
                                                                               relp_lids);
            break;
        case 0x8:  // ()-[r]-()
            num_edges = CountAllOutEdges() * 2;
            break;
        case 0x9:  // (:SL)-[r]-()
            num_edges = CountEdgesWithOneEndLabel(start_lid);
            break;
        case 0xa:  // ()-[r]-(:NL)
            num_edges = CountEdgesWithOneEndLabel(nbr_lid);
            break;
        case 0xb:  // (:SL)-[r]-(:NL)
            num_edges = CountEdgesWithStartAndNeighborLabel(start_lid, nbr_lid);
            break;
        case 0xc:  // ()-[r:RL]-()
            num_edges = CountOutEdgesWithRelationshipLabels(relp_lids) * 2;
            break;
        case 0xd:  // (:SL)-[r:RL]-()
            num_edges = CountEdgesWithOneEndAndRelationshipLabels(start_lid, relp_lids);
            break;
        case 0xe:  // ()-[r:RL]-(:NL)
            num_edges = CountEdgesWithOneEndAndRelationshipLabels(nbr_lid, relp_lids);
            break;
        case 0xf:  // (:SL)-[r:RL]-(:NL)
            num_edges =
                CountEdgesWithStartAndNeighborAndRelationshipLabels(start_lid, nbr_lid, relp_lids);
            break;
        default:
            break;
        }
    return_result:
        record->values[0].constant = lgraph::FieldData(num_edges);
        state = StreamDepleted;
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [")
            .append(relp_->Alias())
            .append("(")
            .append(std::to_string(pattern_type_))
            .append(")]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};

}  // namespace cypher
