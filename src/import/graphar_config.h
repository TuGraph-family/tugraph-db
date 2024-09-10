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

#pragma once

#include <graphar/graph_info.h>
#include <graphar/types.h>

#include "tools/json.hpp"

namespace lgraph {
namespace import_v3 {

/**
 * @brief  Parse the gar DataType to FieldType in config.
 *
 * @param[in]   data_type  The GraphAr DataType of the vetex or edge property.
 * @param[out]   type_name  The FieldType string which used to make json object.
 */
inline void ParseType(const std::shared_ptr<graphar::DataType>& data_type, std::string& type_name) {
    switch (data_type->id()) {
    case graphar::Type::BOOL:
        type_name = "BOOL";
        break;
    case graphar::Type::INT32:
        type_name = "INT32";
        break;
    case graphar::Type::INT64:
        type_name = "INT64";
        break;
    case graphar::Type::FLOAT:
        type_name = "FLOAT";
        break;
    case graphar::Type::DOUBLE:
        type_name = "DOUBLE";
        break;
    case graphar::Type::STRING:
        type_name = "STRING";
        break;
    case graphar::Type::DATE:
        type_name = "DATE";
        break;
    default:
        THROW_CODE(InputError, "Unsupported data type error!");
        break;
    }
}

/**
 * Traverse all properties of the vertex, get the primary key, the properties and the property
 * names. Keep the original order in yml config.
 *
 * @param[in]  ver_info  The gar vertex information.
 * @param[out] primary  The primary key of the vertex.
 * @param[out] props  All the properties of the vertex. One of it maybe
 * {"name":"id","type":"INT64"}.
 * @param[out] prop_names  All the property names of the vertex. One of it maybe "id".
 */
inline void WalkVertex(const graphar::VertexInfo& ver_info, std::string& primary, nlohmann::json& props,
                       std::vector<std::string>& prop_names) {
    auto& ver_groups = ver_info.GetPropertyGroups();
    for (auto& ver_props : ver_groups) {
        for (const auto& prop : ver_props->GetProperties()) {
            if (prop.is_primary) primary = prop.name;
            prop_names.emplace_back(std::move(prop.name));
            std::string type_name;
            ParseType(prop.type, type_name);
            auto prop_config =
                nlohmann::json({{"name", prop.name}, {"type", type_name}});
            if (prop.is_nullable) {
                prop_config["optional"] = true;
            }
            props.emplace_back(prop_config);
        }
    }
}

/**
 * Traverse all properties of the edge, get the properties and the property names.
 * Keep the original order in yml config. Similar to WalkVertex, but don't get primary.
 *
 * @param[in]   edge_info  The gar edge information.
 * @param[out]  props  All the properties of the vertex. One of it maybe
 * {"name":"id","type":"INT64"}.
 * @param[out]  prop_names  All the property names of the vertex. One of it maybe "id".
 */
inline void WalkEdge(const graphar::EdgeInfo& edge_info, nlohmann::json& props,
                     std::vector<std::string>& prop_names) {
    auto& edge_groups = edge_info.GetPropertyGroups();
    for (const auto& edge_props : edge_groups) {
        for (const auto& prop : edge_props->GetProperties()) {
            prop_names.emplace_back(std::move(prop.name));
            std::string type_name;
            ParseType(prop.type, type_name);
            auto prop_config =
                nlohmann::json({{"name", prop.name}, {"type", type_name}});
            if (prop.is_nullable) {
                prop_config["optional"] = true;
            }
            props.emplace_back(prop_config);
        }
    }
}

/**
 * @brief   Read the gar yml file to construct the import config in json form.
 *
 * @param[out]  gar_conf  The json object of the import config used in import_v3.
 * @param[in]   path  The location of gar yml file.
 */
inline void ParserGraphArConf(nlohmann::json& gar_conf, const std::string& path) {
    auto graph_info = graphar::GraphInfo::Load(path).value();
    gar_conf["schema"] = {};
    gar_conf["files"] = {};
    auto vertex_infos = graph_info->GetVertexInfos();
    for (const auto& vertex_info : vertex_infos) {
        nlohmann::json schema_node;
        schema_node["label"] = vertex_info->GetLabel();
        schema_node["type"] = "VERTEX";
        std::string primary;
        nlohmann::json properties;
        std::vector<std::string> prop_names;
        WalkVertex(*vertex_info, primary, properties, prop_names);
        schema_node["primary"] = primary;
        schema_node["properties"] = properties;
        gar_conf["schema"].push_back(schema_node);

        nlohmann::json file_node;
        file_node["path"] = path;
        file_node["format"] = "GraphAr";
        file_node["label"] = vertex_info->GetLabel();
        file_node["columns"] = prop_names;
        gar_conf["files"].push_back(file_node);
    }

    auto edge_infos = graph_info->GetEdgeInfos();
    // The map of edge_label and its properties
    std::unordered_map<std::string, nlohmann::json> edge_labels;
    for (const auto& edge_info : edge_infos) {
        std::string label = edge_info->GetEdgeLabel();
        nlohmann::json properties;
        std::vector<std::string> prop_names = {"SRC_ID", "DST_ID"};
        WalkEdge(*edge_info, properties, prop_names);
        if (!edge_labels.count(label)) {
            edge_labels[label] = properties;
            nlohmann::json schema_node;
            schema_node["label"] = label;
            schema_node["type"] = "EDGE";
            if (properties.size()) {
                schema_node["properties"] = properties;
            }
            gar_conf["schema"].push_back(schema_node);
        } else {
            if (properties != edge_labels[label]) {
                THROW_CODE(InputError, "The edge [" + label + "] has different properties!");
            }
        }

        nlohmann::json file_node;
        file_node["path"] = path;
        file_node["format"] = "GraphAr";
        file_node["label"] = edge_info->GetEdgeLabel();
        file_node["SRC_ID"] = edge_info->GetSrcLabel();
        file_node["DST_ID"] = edge_info->GetDstLabel();
        file_node["columns"] = prop_names;
        gar_conf["files"].push_back(file_node);
    }
}

}  // namespace import_v3
}  // namespace lgraph
