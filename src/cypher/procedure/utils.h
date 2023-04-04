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
#include "lgraph/lgraph_types.h"

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
    PluginAdapter(lgraph_api::SigSpec* sig_spec,
                  lgraph::plugin::Type type,
                  std::string name)
        : sig_spec_(sig_spec),
          type_(type),
          name_(std::move(name)) {}
    PluginAdapter(const PluginAdapter&) = delete;
    PluginAdapter& operator=(const PluginAdapter&) = delete;
    PluginAdapter(PluginAdapter&&) = delete;
    PluginAdapter& operator=(PluginAdapter&&) = delete;
    bool Process(const RTContext *ctx, const std::vector<ArithExprNode> &params,
                 const Record& input, std::vector<Record>* results) {
        CYPHER_THROW_ASSERT(params.size() == sig_spec_->input_list.size());
        CYPHER_THROW_ASSERT(sig_spec_ != nullptr);
        std::vector<std::string> arguments;
        // input params type check
        for (size_t i = 0; i < params.size(); i++) {
            auto argument = params[i].Evaluate(const_cast<RTContext *>(ctx), input);
            auto parameter = sig_spec_->input_list[i];
            switch (argument.type) {
                case Entry::CONSTANT:
                    if (!lgraph_api::LGraphTypeIsField(parameter.type)) {
                        throw lgraph::EvaluationException("Invalid argument");
                    }
                    if (parameter.type == lgraph_api::LGraphType::BOOLEAN) {
                        if(!argument.constant.IsBool()) {
                            throw lgraph::EvaluationException("Invalid argument");
                        } else {
                            arguments.emplace_back(argument.ToString("null"));
                            continue;
                        }
                    }
                    if (parameter.type == lgraph_api::LGraphType::INTEGER) {
                        if(!argument.constant.IsInteger()) {
                            throw lgraph::EvaluationException("Invalid argument");
                        } else {
                            arguments.emplace_back(argument.ToString("null"));
                            continue;
                        }
                    }
                    if (parameter.type == lgraph_api::LGraphType::FLOAT) {
                        if(!argument.constant.IsReal()) {
                            throw lgraph::EvaluationException("Invalid argument");
                        } else {
                            arguments.emplace_back(argument.ToString("null"));
                            continue;
                        }
                    }
                    if (parameter.type == lgraph_api::LGraphType::DOUBLE) {
                        if(!argument.constant.IsReal()) {
                            throw lgraph::EvaluationException("Invalid argument");
                        } else {
                            arguments.emplace_back(argument.ToString("null"));
                            continue;
                        }
                    }
                    if (parameter.type == lgraph_api::LGraphType::STRING) {
                        if(!argument.constant.IsString()) {
                            throw lgraph::EvaluationException("Invalid argument");
                        } else {
                            arguments.emplace_back("\"" + argument.ToString("null") + "\"");
                            continue;
                        }
                    }
                    if (parameter.type == lgraph_api::LGraphType::LIST) {
                        if (!argument.constant.IsArray()) {
                            throw lgraph::EvaluationException("Invalid argument");
                        } else {
                            arguments.emplace_back(argument.ToString("null"));
                            continue;
                        }
                    }
                    if (parameter.type == lgraph_api::LGraphType::MAP) {
                        throw lgraph::EvaluationException("Invalid argument, not support map");
                    }
                    break;
                case Entry::NODE:
                    if (parameter.type != lgraph_api::LGraphType::NODE) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back("\"" + argument.ToString("null") + "\"");
                        continue;
                    }
                case Entry::RELATIONSHIP:
                    if (parameter.type != lgraph_api::LGraphType::RELATIONSHIP) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back("\"" + argument.ToString("null") + "\"");
                        continue;
                    }
                default:
                    throw lgraph::EvaluationException("Invalid argument");
                }
            arguments.emplace_back(argument.ToString("null"));
        }

        std::string request("{");
        size_t input_list_size = sig_spec_->input_list.size();
        for (size_t i = 0; i < input_list_size - 1; i++) {
            request.append(
                FMA_FMT("\"{}\":{},", sig_spec_->input_list[i].name, arguments[i])
            );
        }
        request.append(
            FMA_FMT("\"{}\":{}", sig_spec_->input_list[input_list_size-1].name, arguments[input_list_size-1])
        );
        request.append("}");

        std::string response;
        ctx->ac_db_->CallPlugin(type_, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name_, request, 0, false,
                               response);

        try {
            auto j = nlohmann::json::parse(response);
            for (const auto& item : j.items()) {
                    Record r;
                    for (const auto &result : sig_spec_->result_list) {
                        switch (result.type) {
                        case lgraph_api::LGraphType::INTEGER:
                            {
                                auto result_val = item.value()[result.name].get<int64_t>();
                                r.AddConstant(lgraph::FieldData(result_val));
                            }
                            break;
                        case lgraph_api::LGraphType::FLOAT:
                            {
                                auto result_val = item.value()[result.name].get<float>();
                                r.AddConstant(lgraph::FieldData(result_val));
                            }
                            break;
                        case lgraph_api::LGraphType::DOUBLE:
                            {
                                auto result_val = item.value()[result.name].get<double>();
                                r.AddConstant(lgraph::FieldData(result_val));
                            }
                            break;
                        case lgraph_api::LGraphType::BOOLEAN:
                            {
                                auto result_val = item.value()[result.name].get<bool>();
                                r.AddConstant(lgraph::FieldData(result_val));
                            }
                            break;
                        case lgraph_api::LGraphType::STRING:
                            {
                                auto result_val = item.value()[result.name].get<std::string>();
                                r.AddConstant(lgraph::FieldData(result_val));
                            }
                            break;
                        case lgraph_api::LGraphType::LIST:
                            {
                                std::cout << "item = " << item.value().dump() << std::endl;
                                auto list = item.value()[result.name];
                                CYPHER_THROW_ASSERT(list.is_array());
                                std::vector<lgraph::FieldData> entry;
                                for (auto it = list.begin(); it != list.end(); it++) {
                                    CYPHER_THROW_ASSERT(it->is_primitive());
                                    if (it->is_number_float()) {
                                        entry.emplace_back(lgraph::FieldData(it->get<float>()));
                                    } else if (it->is_number_integer()) {
                                        entry.emplace_back(lgraph::FieldData(it->get<int64_t>()));
                                    } else if (it->is_boolean()) {
                                        entry.emplace_back(lgraph::FieldData(it->get<bool>()));
                                    } else if (it->is_string()) {
                                        entry.emplace_back(
                                            lgraph::FieldData(it->get<std::string>()));
                                    } else {
                                        CYPHER_TODO();
                                    }
                                }
                                r.AddConstant(cypher::FieldData(std::move(entry)));
                            }
                            break;
                        case lgraph_api::LGraphType::NODE:
                            {
                                auto node_str = item.value()[result.name].get<std::string>();
                                auto parse_vertex_node = [](const std::string &s) -> int64_t {
                                    auto id_begin = s.find('[');
                                    return strtoll(s.c_str() + id_begin + 1, nullptr, 10);
                                };
                                lgraph::VertexId vid = parse_vertex_node(node_str);
                                Node& node = node_buffer_.AllocNode(vid, result.name);
                                r.AddNode(&node);
                            }
                            break;
                        default:
                            CYPHER_TODO();
                        }
                    }
                    results->emplace_back(std::move(r));
            }
        } catch (std::exception &e) {
            response = std::string("error parsing json: ") + e.what();
            return false;
        }
        return true;
    }
 private:
    std::string name_;
    lgraph::plugin::Type type_ = lgraph::plugin::Type::CPP;
    lgraph_api::SigSpec* sig_spec_ = nullptr;
};




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
        return ctx.ac_db_->CallPlugin(type, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name, input, 0, false,
                                output);
    }
};

}  // namespace cypher
