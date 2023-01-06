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
// Created by wt on 2020/3/20.
//
#pragma once

#include "core/lightning_graph.h"
#include "db/galaxy.h"
#include "parser/clause.h"

namespace lgraph {

template <class EIT>
class LabeledEdgeIterator : public EIT {
    uint16_t lid_;
    size_t tid_;
    bool valid_;

 public:
    LabeledEdgeIterator(EIT &&eit, uint16_t lid, int64_t tid)
        : EIT(std::move(eit)), lid_(lid), tid_(tid) {
        valid_ = EIT::IsValid() && EIT::GetLabelId() == lid_ && EIT::GetTemporalId() == tid_;
    }

    bool IsValid() { return valid_; }

    bool Next() {
        if (!valid_) return false;
        valid_ = (EIT::Next() && EIT::GetLabelId() == lid_ && EIT::GetTemporalId() == tid_);
        return valid_;
    }

    void Reset(graph::VertexIterator &vit, uint16_t lid, int64_t tid) {
        Reset(vit.GetId(), lid, tid);
    }

    void Reset(int64_t vid, uint16_t lid, int64_t tid) {
        lid_ = lid;
        tid_ = tid;
        if (std::is_same<EIT, graph::OutEdgeIterator>::value) {
            EIT::Goto(EdgeUid(vid, 0, lid, tid, 0), true);
        } else {
            EIT::Goto(EdgeUid(0, vid, lid, tid, 0), true);
        }
        valid_ = (EIT::IsValid() && EIT::GetLabelId() == lid_ && EIT::GetTemporalId() == tid_);
    }
};

}  // namespace lgraph

namespace cypher {

class Utils {
 public:
    static bool CallPlugin(const RTContext &ctx, lgraph::plugin::Type type, const std::string &name,
                           const std::vector<parser::Expression> &params, std::string &output) {
        std::string input;
        CYPHER_THROW_ASSERT(params.size() <= 1);
        if (!params.empty()) {
            switch (params[0].type) {
            case parser::Expression::MAP:
                input.append("{").append(params[0].ToString(false)).append("}");
                break;
            case parser::Expression::STRING:
                input = params[0].String();
                break;
            default:
                throw lgraph::EvaluationException("Invalid argument");
            }
        }
        auto ac_db = ctx.galaxy_->OpenGraph(ctx.user_, ctx.graph_);
        return ac_db.CallPlugin(type, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name, input, 0, false,
                                output);
    }
};

}  // namespace cypher
