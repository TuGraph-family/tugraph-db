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
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express osr implied.
*/

/*
* written by botu.wzy
*/
#include <string>
#include "tools/lgraph_log.h"
#include "bolt/record.h"
#include "bolt/graph.h"
#include "lgraph/lgraph_date_time.h"
#pragma once

namespace bolt {
namespace detail {
nlohmann::json ToJsonObj(const bolt::Node& node);
nlohmann::json ToJsonObj(const bolt::Relationship& rel);
nlohmann::json ToJsonObj(const bolt::Path& path);
nlohmann::json ToJsonObj(const std::any& item) {
    if (!item.has_value()) {
        return nullptr;
    }
    if (item.type() == typeid(int64_t)) {
        return std::any_cast<int64_t>(item);
    } else if (item.type() == typeid(bool)) {
        return std::any_cast<bool>(item);
    } else if (item.type() == typeid(float)) {
        return std::any_cast<float>(item);
    } else if (item.type() == typeid(double)) {
        return std::any_cast<double>(item);
    } else if (item.type() == typeid(std::string)) {
        return std::any_cast<const std::string&>(item);
    } else if (item.type() == typeid(const char*)) {
        return std::any_cast<const char*>(item);
    } else if (item.type() == typeid(bolt::Node)) {
        const auto& node = std::any_cast<const bolt::Node&>(item);
        return ToJsonObj(node);
    } else if (item.type() == typeid(bolt::Relationship)) {
        const auto& rel = std::any_cast<const bolt::Relationship&>(item);
        return ToJsonObj(rel);
    } else if (item.type() == typeid(bolt::Path)) {
        const auto& path = std::any_cast<const bolt::Path&>(item);
        return ToJsonObj(path);
    } else if (item.type() == typeid(bolt::Date)) {
        const auto& date = std::any_cast<const bolt::Date&>(item);
        lgraph_api::Date d(date.days);
        return ToJsonObj(d.ToString());
    } else if (item.type() == typeid(bolt::LocalDateTime)) {
        const auto& localDateTime = std::any_cast<const bolt::LocalDateTime&>(item);
        lgraph_api::DateTime dateTime(
            localDateTime.seconds * 1000000 + localDateTime.nanoseconds/1000);
        return ToJsonObj(dateTime.ToString());
    } else if (item.type() == typeid(std::vector<std::any>)) {
        const auto& vector = std::any_cast<const std::vector<std::any>&>(item);
        nlohmann::json ret = nlohmann::json::array();
        for (auto& item : vector) {
            ret.push_back(ToJsonObj(item));
        }
        return ret;
    } else if (item.type() == typeid(std::unordered_map<std::string, std::any>)) {
        const auto& map = std::any_cast<const std::unordered_map<std::string, std::any>&>(item);
        nlohmann::json ret = nlohmann::json::object();
        for (auto& pair : map) {
            ret[pair.first] = ToJsonObj(pair.second);
        }
        return ret;
    } else {
        auto err = std::string("Unsupported type: ") + item.type().name();
        LOG_ERROR() << err;
        throw std::runtime_error(err);
    }
}

nlohmann::json ToJsonObj(const bolt::Node& node) {
    std::string ret("(");
    for (auto& label : node.labels) {
        ret.append(":").append(label);
    }
    ret.append(" {");
    int count = 0;
    for (auto& pair : node.props) {
        if (count > 0) {
            ret.append(",");
        }
        ret.append(pair.first);
        ret.append(":");
        ret.append(ToJsonObj(pair.second).dump());
        count++;
    }
    ret.append("})");
    return ret;
}

nlohmann::json ToJsonObj(const bolt::Relationship& rel) {
    std::string ret;
    ret.append("[:").append(rel.type).append(" {");
    int count = 0;
    for (auto& pair : rel.props) {
        if (count > 0) {
            ret.append(",");
        }
        ret.append(pair.first);
        ret.append(":");
        ret.append(ToJsonObj(pair.second).dump());
        count++;
    }
    ret.append("}]");
    return ret;
}

nlohmann::json ToJsonObj(const bolt::Path& path) {
    std::unordered_map<int64_t, size_t> nodes_index;
    for (size_t i = 0; i < path.nodes.size(); i++) {
        nodes_index[path.nodes[i].id] = i;
    }
    auto ret = ToJsonObj(path.nodes[0]).get<std::string>();
    auto nodeId = path.nodes[0].id;
    bool forward = true;
    for (auto& rel : path.relationships) {
        forward = (rel.startId == nodeId);
        ret.append(forward ? "-" : "<-");
        ret.append(ToJsonObj(rel).get<std::string>());
        ret.append(forward ? "->" : "-");
        ret.append(ToJsonObj(path.nodes[nodes_index.at(rel.endId)]).get<std::string>());
        nodeId = rel.endId;
    }
    return ret;
}
}  // namespace detail

std::string Print(const std::any& boltType) {
    auto j = detail::ToJsonObj(boltType);
    if (j.is_string()) {
        return j.get<std::string>();
    } else {
        return j.dump();
    }
}

nlohmann::json ToJson(const std::any& boltType) {
    return detail::ToJsonObj(boltType);
}

}  // namespace bolt
