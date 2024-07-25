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

#include "parser/clause.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_result.h"
#include "lgraph_api/result_element.h"
#include "geax-front-end/ast/utils/AstUtil.h"

namespace lgraph {

template <class EIT>
class LabeledEdgeIterator : public EIT {
    uint16_t lid_;
    size_t tid_;
    bool valid_;

 public:
    LabeledEdgeIterator(EIT&& eit, uint16_t lid, int64_t tid)
        : EIT(std::move(eit)), lid_(lid), tid_(tid) {
        valid_ = EIT::IsValid() && EIT::GetLabelId() == lid_ && EIT::GetTemporalId() == tid_;
    }

    bool IsValid() { return valid_; }

    bool Next() {
        if (!valid_) return false;
        valid_ = (EIT::Next() && EIT::GetLabelId() == lid_ && EIT::GetTemporalId() == tid_);
        return valid_;
    }

    void Reset(graph::VertexIterator& vit, uint16_t lid, int64_t tid) {
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
class ArithExprNode;
/**
 * @brief Plugin Adapter plays a role which converts back and forth between a cypher record and json
 * string. it would try best effort to check type of input record and result records valid with
 * provided signature in plan-build time. However, due to lack of powerful type system, it cannot
 * do for now(e.g. for n.id, we only know it is property, even worse a constant, but cannot
 * know it is a int64).
 *
 * @note Plugin Adapter CAN ONLY be used in new created plugins with `GetSignature` function.
 * Plugins without `GetSignature` functions are called `old plugins`. Call `old plugins` by
 * Utils::CallPlugin instead.
 */
class PluginAdapter {
    class NodeBuffer {
        std::vector<cypher::Node> buffer_;

     public:
        NodeBuffer() { buffer_.reserve(128); }
        ~NodeBuffer() = default;
        void Reserve(int64_t size) { buffer_.reserve(size); }
        int64_t Capacity() { return buffer_.capacity(); }
        DISABLE_COPY(NodeBuffer);
        DISABLE_MOVE(NodeBuffer);
        Node& AllocNode(NodeID id, const std::string& alias) {
            // N.B. alloc node when exceeding buffer capacity makes reallocation.
            // It will invalid the old nodes allocated before.
            CYPHER_THROW_ASSERT(buffer_.size() < buffer_.capacity());
            buffer_.emplace_back(id, "", alias, cypher::Node::Derivation::YIELD);
            return buffer_.back();
        }
    } node_buffer_;

 public:
    PluginAdapter(lgraph_api::SigSpec* sig_spec, lgraph::plugin::Type type, std::string name)
        : sig_spec_(sig_spec), type_(type), name_(std::move(name)) {}
    PluginAdapter(const PluginAdapter&) = delete;
    PluginAdapter& operator=(const PluginAdapter&) = delete;
    PluginAdapter(PluginAdapter&&) = delete;
    PluginAdapter& operator=(PluginAdapter&&) = delete;
    bool Process(const RTContext* ctx, const std::vector<ArithExprNode>& params,
                 const Record& input, std::vector<Record>* results);

 private:
    lgraph_api::SigSpec* sig_spec_ = nullptr;
    lgraph::plugin::Type type_ = lgraph::plugin::Type::CPP;
    std::string name_;
};

class Utils {
 public:
    static bool CallPlugin(const RTContext& ctx, lgraph::plugin::Type type, const std::string& name,
                           const std::vector<parser::Expression>& params, std::string& output) {
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
        return ctx.ac_db_->CallPlugin(ctx.txn_.get(), type, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name,
                                      input, 0, false, output);
    }

    static bool CallPlugin(const RTContext& ctx, lgraph::plugin::Type type, const std::string& name,
                           const std::vector<geax::frontend::Expr*>& params, std::string& output) {
        std::string input;
        CYPHER_THROW_ASSERT(params.size() <= 1);
        if (!params.empty()) {
            input = geax::frontend::ToString(params[0]);
        }
        return ctx.ac_db_->CallPlugin(ctx.txn_.get(), type, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name,
                                      input, 0, false, output);
    }
};

}  // namespace cypher
