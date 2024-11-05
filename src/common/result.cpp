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

/*
 * written by botu.wzy
 */

#include "result.h"
#include <spdlog/fmt/ranges.h>
#include "bolt/graph.h"
#include "bolt/path.h"
namespace common {

std::string Node::ToString() const{
    return fmt::format("{{id:{}, labels:{}, properties:{}}}",
                       id, labels, properties);
}

bolt::Node Node::ToBolt() const {
    bolt::Node ret;
    ret.id = id;
    ret.labels = {labels.begin(), labels.end()};
    for (auto& pair : properties) {
        ret.props.emplace(pair.first, pair.second.ToBolt());
    }
    return ret;
}

std::string Relationship::ToString() const{
    return fmt::format("{{id:{}, src:{}, dst:{}, type:\"{}\", properties:{}}}",
                       id, src, dst, type, properties);
}

bolt::Relationship Relationship::ToBolt() const {
    bolt::Relationship ret;
    ret.id = id;
    ret.startId = src;
    ret.endId = dst;
    ret.type = type;
    for (auto& pair : properties) {
        ret.props.emplace(pair.first, pair.second.ToBolt());
    }
    return ret;
}

bolt::RelNode Relationship::ToBoltUnbound() const {
    bolt::RelNode rel;
    rel.id = id;
    rel.name = type;
    for (auto &pair : properties) {
        rel.props.emplace(pair.first, pair.second.ToBolt());
    }
    return rel;
}

std::string Result::ToString() const{
    switch (type) {
        case ResultType::Node: {
            return std::any_cast<const Node&>(data).ToString();
        }
        case ResultType::Relationship: {
            return std::any_cast<const Relationship&>(data).ToString();
        }
        case ResultType::Value: {
            return std::any_cast<const Value&>(data).ToString();
        }
        default: {
            throw std::runtime_error("unexpected type for Result::ToString");
        }
    }
}

std::any Result::ToBolt() const {
    switch (type) {
        case ResultType::Node: {
            return std::any_cast<const Node&>(data).ToBolt();
        }
        case ResultType::Relationship: {
            return std::any_cast<const Relationship&>(data).ToBolt();
        }
        case ResultType::Value: {
            return std::any_cast<const Value&>(data).ToBolt();
        }
        case ResultType::Path: {
            return std::any_cast<const Path&>(data).ToBolt();
        }
        default: {
            throw std::runtime_error("unexpected type for Result::ToBolt");
        }
    }
}

std::string Path::ToString() const {
    std::vector<std::string> str;
    for (auto& ele : data) {
        if (ele.is_node) {
            str.push_back(std::any_cast<const Node&>(ele.data).ToString());
        } else {
            str.push_back(std::any_cast<const Relationship&>(ele.data).ToString());
        }
    }
    return fmt::format("[{}]", str);
}

bolt::InternalPath Path::ToBolt() const {
    bolt::InternalPath path;
    for (size_t i = 0; i < data.size(); i++) {
        auto& p = data[i];
        if (p.is_node) {
            path.nodes.push_back(std::any_cast<const common::Node&>(p.data).ToBolt());
        } else {
            path.rels.push_back(std::any_cast<const common::Relationship&>(p.data).ToBoltUnbound());
        }
        if (i >= 1) {
            if (i%2 == 1) {
                if (std::any_cast<const common::Relationship&>(p.data).src == path.nodes.back().id) {
                    path.indices.push_back((int)path.rels.size());
                } else {
                    path.indices.push_back(0-(int)path.rels.size());
                }
            } else {
                path.indices.push_back((int)path.nodes.size()-1);
            }
        }
    }
    return path;
}

}
