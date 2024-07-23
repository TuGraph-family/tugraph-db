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

#include "cypher/procedure/utils.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {

bool PluginAdapter::Process(const RTContext* ctx, const std::vector<ArithExprNode>& params,
             const Record& input, std::vector<Record>* results) {
    CYPHER_THROW_ASSERT(params.size() == sig_spec_->input_list.size());
    CYPHER_THROW_ASSERT(sig_spec_ != nullptr);
    std::vector<std::string> arguments;
    // input params type check
    for (size_t i = 0; i < params.size(); i++) {
        auto argument = params[i].Evaluate(const_cast<RTContext*>(ctx), input);
        auto parameter = sig_spec_->input_list[i];
        switch (argument.type) {
        case Entry::CONSTANT:
            if (lgraph_api::LGraphTypeIsField(parameter.type)) {
                if (parameter.type == lgraph_api::LGraphType::BOOLEAN) {
                    if (!argument.constant.IsBool()) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back(argument.ToString("null"));
                        continue;
                    }
                }
                if (parameter.type == lgraph_api::LGraphType::INTEGER) {
                    if (!argument.constant.IsInteger()) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back(argument.ToString("null"));
                        continue;
                    }
                }
                if (parameter.type == lgraph_api::LGraphType::FLOAT) {
                    if (!argument.constant.IsReal()) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back(argument.ToString("null"));
                        continue;
                    }
                }
                if (parameter.type == lgraph_api::LGraphType::DOUBLE) {
                    if (!argument.constant.IsReal()) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back(argument.ToString("null"));
                        continue;
                    }
                }
                if (parameter.type == lgraph_api::LGraphType::STRING) {
                    if (!argument.constant.IsString()) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back("\"" + argument.ToString("null") + "\"");
                        continue;
                    }
                }
            } else {
                if (parameter.type == lgraph_api::LGraphType::LIST) {
                    if (!argument.constant.IsArray()) {
                        throw lgraph::EvaluationException("Invalid argument");
                    } else {
                        arguments.emplace_back(argument.ToString("null"));
                        continue;
                    }
                } else {
                    throw lgraph::EvaluationException("Invalid argument");
                }
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
    int input_list_size = sig_spec_->input_list.size();
    for (int i = 0; i < input_list_size - 1; i++) {
        request.append(FMA_FMT("\"{}\":{},", sig_spec_->input_list[i].name, arguments[i]));
    }
    if (input_list_size > 0) {
        request.append(FMA_FMT("\"{}\":{}", sig_spec_->input_list[input_list_size - 1].name,
                               arguments[input_list_size - 1]));
    }
    request.append("}");

    lgraph_api::Result api_result;
    bool ret = ctx->ac_db_->CallV2Plugin(ctx->txn_.get(),
                                         type_, "A_DUMMY_TOKEN_FOR_CPP_PLUGIN", name_,
                                         request, 0, false, api_result);
    if (!ret) {
        throw lgraph::CypherException("Plugin return false, errMsg: " + api_result.Dump());
    }

    try {
        results->reserve(results->size() + api_result.Size());
        int64_t node_num_in_result = 0;
        for (const auto& result : sig_spec_->result_list) {
            if (result.type == lgraph_api::LGraphType::NODE) {
                node_num_in_result += api_result.Size();
            }
        }
        node_buffer_.Reserve(node_num_in_result);

        for (int64_t i = 0; i < api_result.Size(); i++) {
            const auto& rview = api_result.RecordView(i);
            Record r;
            for (const auto& result : sig_spec_->result_list) {
                const auto it = rview.find(result.name);
                CYPHER_THROW_ASSERT(it != rview.end());
                switch (result.type) {
                case lgraph_api::LGraphType::INTEGER:
                case lgraph_api::LGraphType::FLOAT:
                case lgraph_api::LGraphType::DOUBLE:
                case lgraph_api::LGraphType::BOOLEAN:
                case lgraph_api::LGraphType::STRING:
                    r.AddConstant(*(it->second->v.fieldData));
                    break;
                case lgraph_api::LGraphType::LIST:
                    {
                        auto list = it->second->v.list;
                        std::vector<lgraph::FieldData> entry;
                        for (const auto& json_obj : *list) {
                            CYPHER_THROW_ASSERT(json_obj.is_primitive());
                            if (json_obj.is_number_float()) {
                                entry.emplace_back(json_obj.get<float>());
                            } else if (json_obj.is_number_integer()) {
                                entry.emplace_back(json_obj.get<int64_t>());
                            } else if (json_obj.is_boolean()) {
                                entry.emplace_back(json_obj.get<bool>());
                            } else if (json_obj.is_string()) {
                                entry.emplace_back(json_obj.get<std::string>());
                            } else {
                                CYPHER_TODO();
                            }
                        }
                        r.AddConstant(cypher::FieldData(std::move(entry)));
                    }
                    break;
                case lgraph_api::LGraphType::NODE:
                    {
                        auto node_ptr = it->second->v.node;
                        lgraph::VertexId vid = node_ptr->id;
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
    } catch (std::exception& e) {
        LOG_WARN() << "error parsing json: " << e.what();
        return false;
    }
    return true;
}

}  // namespace cypher
